"""
Script: alt.spm-to-alt

Convert a SentencePiece model to the ALT model file format.

Usage:
    python -m alt.spm-to-alt [options]

Options:
    -h : Show the help message and exit.
    -v : Enable verbose output.
    -t : Output only the tokenizer (Includes General and Parameters but omits tensors)
    -d : Data type (float32, float16, qint8, qint4 (Q4 is packed into int8))
    <model_dir> : Path to the directory containing the sentencepiece model.

ALT Start:
    - section_marker, 8 bytes, format as "q", 'ALT' in hex (0x616C7400)
    - section_size, 8 bytes, format as "q", Total size of the Header section (2 fields as int32_t where 4 + 4 = 8)
    - magic_version, 4 bytes, format as "i", Model file format version
    - magic_alignment, 4 bytes, format as "i", Value used for aligning model sections
    - Add alignment padding (if any) to the model file

Alt General:
    - TODO

Alt Parameters:
    - TODO

ALT Tokenizer:
    - section_marker, 8 bytes, format as "q", 0xBADDCAFE
    - section_size, 8 bytes, format as "q", Total size of the Tokenizer section
    - vocab_size, 4 bytes, format as "i", Total number of tokens
    - bos_id, 4 bytes, format as "i", Beginning-of-sequence token ID
    - eos_id, 4 bytes, format as "i", End-of-sequence token ID
    - pad_id, 4 bytes, format as "i", Padding token ID
    - unk_id, 4 bytes, format as "i", Unknown token ID
    - For each token in the vocabulary:
        - token_len, 4 bytes, format as "i", Length of each token (per token), Prefix for each token's data
        - token_data, variable, format as "s", Token string data (UTF-8), UTF-8 encoded byte sequence
        - token_score, 4 bytes, format as "f", E.g., log probability
        - token_type, 4 bytes, format as "i", Token type, Enum: `NORMAL`, `UNKNOWN`, etc.
    - Add alignment padding (if any) to the model file

Alt Tensors:
    - TODO

ALT End:
    - end_marker, 8 bytes, format as "q", Uniquely identifies the end of the file with 0x0FFFFFFF
    - There is no padding or alignment after the End Marker; any attempt to read beyond this point is undefined.

NOTE: The byte order is inferred. It is not explicitly provided for flexibility.
"""

import logging
import struct
import sys
from argparse import ArgumentParser, Namespace
from collections import OrderedDict
from dataclasses import dataclass
from logging import Logger
from pathlib import Path
from typing import IO, Any, Optional

from sentencepiece import SentencePieceProcessor


@dataclass
class MetaParams:
    """Encapsulates parameters for the ALT model conversion process."""

    alt: Optional[IO] = None  # File handle for the ALT file
    model_dir: str = ""  # Directory containing the model components
    verbose: bool = False  # Enable verbose output
    data_type: int = 0  # Data type for tensors (e.g., float32, float16, etc.)
    quant_profile: int = 0  # Quantization profile (e.g., unquantized, QINT8, QINT4)
    context_length: int = 8192  # Context length in tokens
    logger: Logger = None


class BaseModel:
    def __init__(self, meta_params: Optional[MetaParams] = None):
        self.meta_params = meta_params or MetaParams()

    def calculate_size(self) -> int:
        pass

    def write_model(self) -> None:
        pass

    def read_model(self) -> OrderedDict[str, Any]:
        pass


class TokenType:
    NORMAL: int = 0
    BYTE: int = 1
    CONTROL: int = 2
    UNKNOWN: int = 3
    UNUSED: int = 4
    BOS: int = 5
    EOS: int = 6
    PAD: int = 7

    def __init__(self, processor: SentencePieceProcessor):
        self.processor = processor

    def get_token_type(self, token: str, index: int) -> int:
        # Determine token type
        if self.processor.is_byte(index):
            return self.BYTE
        elif self.processor.is_control(index):
            return self.CONTROL
        elif self.processor.is_unknown(index):
            return self.UNKNOWN
        elif self.processor.is_unused(index):
            return self.UNUSED
        elif token == "<s>":
            return self.BOS
        elif token == "</s>":
            return self.EOS
        elif token == "<pad>":
            return self.PAD
        return self.NORMAL


class Magic:
    """Class to handle ALT model file format."""

    ALT = 0x616C7400  # 'alt' in hex; 8 bytes
    GENERAL = 0xCAFEBABE  # 8 bytes
    PARAMETERS = 0xDEADBEEF  # 8 bytes
    TOKENIZER = 0xBADDCAFE  # 8 bytes
    TENSORS = 0xFACEFEED  # 8 bytes
    END = 0x0FFFFFFF  # 8 bytes
    ALIGNMENT = 32  # Default 32-byte alignment
    VERSION = 1  # ALT model file format

    def __init__(self, meta_params: Optional[MetaParams] = None):
        """Initialize the Magic class with optional MetaParams."""
        self.meta_params = meta_params or MetaParams()
        self.alt = self.meta_params.alt
        self.verbose = self.meta_params.verbose
        self.logger = self.meta_params.logger

    def is_alt(self, marker: int) -> bool:
        """Check if the marker is the start marker."""
        return marker == self.ALT

    def is_general(self, marker: int) -> bool:
        return marker == self.GENERAL

    def is_parameters(self, marker: int) -> bool:
        return marker == self.PARAMETERS

    def is_tokenizer(self, marker: int) -> bool:
        return marker == self.TOKENIZER

    def is_tensors(self, marker: int) -> bool:
        return marker == self.TENSORS

    def is_end(self, marker: int) -> bool:
        """Check if the marker is the end marker."""
        return marker == self.END

    def calculate_alignment(self) -> int:
        """Calculate necessary padding to align to 32 bytes."""
        return (Magic.ALIGNMENT - (self.alt.tell() % Magic.ALIGNMENT)) % Magic.ALIGNMENT

    def write_alignment(self):
        """Write alignment padding to the ALT file."""
        padding_needed = self.calculate_alignment(self.alt)
        if padding_needed > 0:
            self.alt.write(b"\x00" * padding_needed)
            if self.verbose:
                self.logger.debug(f"Aligned offset with {padding_needed} bytes of padding.")

    def read_alignment(self):
        """Read alignment padding from the ALT file."""
        padding_needed = self.calculate_alignment(self.alt)
        if padding_needed > 0:
            padding = self.alt.read(padding_needed)
            if padding != b"\x00" * padding_needed:
                raise ValueError("Alignment padding contains non-zero bytes.")
            if self.verbose:
                self.logger.debug(f"Skipped {padding_needed} bytes of padding.")


class MagicModel(BaseModel):
    def __init__(self, meta_params: MetaParams):
        super().__init__(meta_params)
        self.magic = Magic(meta_params)

    def calculate_size(self) -> int:
        pass

    def write_model(self) -> None:
        pass

    def read_model(self) -> OrderedDict[str, Any]:
        pass


class TokenizerModel(BaseModel):
    def __init__(self, meta_params: MetaParams):
        super().__init__(meta_params)
        self.alt = meta_params.alt
        self.path = Path(meta_params.model_dir) / "tokenizer.model"
        self.verbose = meta_params.verbose
        self.logger = meta_params.logger
        self.processor = SentencePieceProcessor(str(self.path))
        self.vocab_size = self.processor.vocab_size()
        self.magic = Magic(meta_params)
        self.token_type = TokenType(self.processor)

    def calculate_size(self) -> int:
        # Calculate the size of the tokenizer section
        size = 4 * 5  # vocab_size, bos_id, eos_id, pad_id, unk_id
        for index in range(self.vocab_size):
            token = self.processor.id_to_piece(index)
            token_bytes = token.encode("utf-8")
            size += 4  # token_len
            size += len(token_bytes)  # token_data
            size += 4  # token_score
            size += 4  # token_type
        return size

    def write_model(self) -> None:
        # Write the Tokenizer section_marker
        self.alt.write(struct.pack("q", Magic.TOKENIZER))

        # Write the Tokenizer section_size
        section_size = self.calculate_size()
        self.alt.write(struct.pack("q", section_size))

        # vocab_size: 4 bytes
        self.alt.write(struct.pack("i", self.vocab_size))

        # bos_id, eos_id, pad_id, unk_id: each 4 bytes
        self.alt.write(struct.pack("i", self.processor.bos_id()))
        self.alt.write(struct.pack("i", self.processor.eos_id()))
        self.alt.write(struct.pack("i", self.processor.pad_id()))
        self.alt.write(struct.pack("i", self.processor.unk_id()))

        # For each token in the vocabulary:
        for index in range(self.vocab_size):
            # Extract token related metadata
            token = self.processor.id_to_piece(index)
            token_bytes = token.encode("utf-8")
            token_len = len(token_bytes)
            token_score = self.processor.get_score(index)
            token_type = self.token_type.get_token_type(token, index)

            # Write token_len and token_data
            self.alt.write(struct.pack("i", token_len))
            self.alt.write(token_bytes)

            # Write token_score
            self.alt.write(struct.pack("f", token_score))

            # Write token_type
            self.alt.write(struct.pack("i", token_type))

        # Add alignment padding
        self.magic.write_alignment()

    def read_model(self) -> OrderedDict:
        """Reads the Tokenizer section from the ALT file."""
        # Align the section with the marker
        self.magic.read_alignment()

        # Read the Tokenizer section_marker
        section_marker = struct.unpack("q", self.alt.read(8))[0]
        if not self.magic.is_tokenizer(section_marker):
            raise ValueError(f"Invalid section marker: {section_marker}")
        self.logger.debug(f"Validated section marker: {section_marker}")

        # Read the Tokenizer section_size
        section_size = struct.unpack("q", self.alt.read(8))[0]
        self.logger.debug(f"Tokenizer Section Size: {section_size}")

        # Read vocab_size
        vocab_size = struct.unpack("i", self.alt.read(4))[0]
        self.logger.debug(f"Vocab Size: {vocab_size}")

        # Read bos_id, eos_id, pad_id, unk_id
        bos_id, eos_id, pad_id, unk_id = struct.unpack("4i", self.alt.read(16))
        self.logger.debug(f"BOS_ID: {bos_id}")
        self.logger.debug(f"EOS_ID: {eos_id}")
        self.logger.debug(f"PAD_ID: {pad_id}")
        self.logger.debug(f"UNK_ID: {unk_id}")

        # Read tokens
        vocab = []
        for _ in range(vocab_size):
            # Read token_len
            token_len = struct.unpack("i", self.alt.read(4))[0]
            # Read token_data
            token = self.alt.read(token_len).decode("utf-8")
            # Read token_score
            token_score = struct.unpack("f", self.alt.read(4))[0]
            # Read token_type
            token_type = struct.unpack("i", self.alt.read(4))[0]
            # Store token information
            token_info = OrderedDict(len=token_len, token=token, score=token_score, type=token_type)
            vocab.append(token_info)
            # Log for debugging
            self.logger.debug(f"Token Info: {token_info}")

        # Construct the result
        tokenizer_data = OrderedDict(
            vocab_size=vocab_size,
            bos_id=bos_id,
            eos_id=eos_id,
            pad_id=pad_id,
            unk_id=unk_id,
            vocab=vocab,
        )

        return tokenizer_data


class AltModel(BaseModel):
    def __init__(self, meta_params: MetaParams):
        super().__init__(meta_params)
        self.model_dir = meta_params.model_dir
        self.path = Path(self.model_dir) / "model.alt"
        self.verbose = meta_params.verbose
        self.logger = meta_params.logger

    def calculate_size(self) -> int:
        # Calculate total size of the ALT model
        return 8 + 8 + 4 + 4  # Marker, Size, Version, Alignment

    def write_model(self) -> None:
        try:
            self.logger.debug(f"Writing ALT model to {self.path}")
            with open(self.path, "wb") as alt:
                # Set the file object
                self.meta_params.alt = alt
                # Create the magic instance
                magic = Magic(self.meta_params)
                # Create the tokenizer instance
                tokenizer = Tokenizer(self.meta_params)
                # Write the ALT header
                self.write_header()
                # Write the tokenizer section
                self.tokenizer.write_model()
                # Write the terminator
                self.write_terminator()
        except Exception as e:
            logging.error(f"Failed to write ALT model: {e}")

    def write_header(self) -> None:
        alt_file = self.meta_params.alt
        magic = self.magic

        # Write the ALT header section
        # section_marker: 'ALT' in hex (0x616C7400), 8 bytes
        alt_file.write(struct.pack("<q", magic.ALT))

        # section_size: total size of the header section (2 fields of 4 bytes each), so 8 bytes
        section_size = 8  # 4 bytes for magic_version, 4 bytes for magic_alignment
        alt_file.write(struct.pack("<q", section_size))

        # magic_version: 4 bytes, format as 'i'
        alt_file.write(struct.pack("<i", magic.VERSION))

        # magic_alignment: 4 bytes, format as 'i'
        alt_file.write(struct.pack("<i", magic.ALIGNMENT))

        # Add alignment padding if necessary
        alignment_padding = magic.calculate_alignment(alt_file.tell())
        if alignment_padding:
            alt_file.write(b"\x00" * alignment_padding)

    def write_terminator(self) -> None:
        alt_file = self.meta_params.alt
        magic = self.magic

        # Write the END marker
        alt_file.write(struct.pack("<q", magic.END))
        # No padding or alignment after the END marker

    def read_model(self) -> OrderedDict:
        # Implementation for reading the ALT model (if needed)
        pass


def get_logger(name=None, level=logging.DEBUG):
    """Get a default logger instance with specified configuration."""
    LOGGER_FORMAT = "%(asctime)s - %(filename)s:%(lineno)d - %(levelname)s - %(message)s"
    logger = logging.getLogger(name)
    if not logger.hasHandlers():
        handler = logging.StreamHandler(stream=sys.stdout)
        formatter = logging.Formatter(LOGGER_FORMAT)
        handler.setFormatter(formatter)
        logger.addHandler(handler)
        logger.setLevel(level)
    return logger


def parse_args() -> Namespace:
    parser = ArgumentParser(
        description="Convert a SentencePiece model to the ALT model file format.",
    )
    parser.add_argument(
        "model_dir",
        type=str,
        help="Path to the directory containing the SentencePiece model.",
    )
    parser.add_argument(
        "-v",
        "--verbose",
        action="store_true",
        help="Enable verbose output.",
    )
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    logger = get_logger(__name__, logging.DEBUG if args.verbose else logging.INFO)
    meta_params = MetaParams(model_dir=args.model_dir, verbose=args.verbose, logger=logger)
    alt = Alt(meta_params)
    alt.write_model()


if __name__ == "__main__":
    main()
