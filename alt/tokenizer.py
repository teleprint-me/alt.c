"""
Module: alt/tokenizer.py

## **4. Tokenizer Section**

### **Purpose**

The Tokenizer Section contains vocabulary and special token IDs needed for input tokenization, supporting diverse tokenization schemes (e.g., SentencePiece) and enabling the model to handle unique token types, scores, and token-specific metadata.

### **Structure**

The Tokenizer Section has three main parts: the **Header**, **Tokenizer Metadata**, and **Vocabulary Tokens**.

### **Header**

| Field            | Description                     | Data Type | Size (bytes) | Notes          |
|------------------|---------------------------------|-----------|--------------|----------------|
| `section_marker` | Identifies Tokenizer Section    | `int64`   | 8            | `0xBADDCAFE`   |
| `section_size`   | Total size of Tokenizer Section | `int64`   | 8            | Varies in size |

### **Fields**

#### **Tokenizer Metadata**

| Field         | Description                     | Data Type | Size (bytes) | Notes        |
|---------------|---------------------------------|-----------|--------------|--------------|
| `vocab_size`  | Total number of tokens          | `int32`   | 4            |              |
| `bos_id`      | Beginning-of-sequence token ID  | `int32`   | 4            |              |
| `eos_id`      | End-of-sequence token ID        | `int32`   | 4            |              |
| `pad_id`      | Padding token ID                | `int32`   | 4            |              |
| `unk_id`      | Unknown token ID                | `int32`   | 4            |              |
| `seq_len`     | Maximum sequence length         | `int32`   | 4            | e.g., `8192` |

#### **Vocabulary Tokens**

Each token in the vocabulary is stored sequentially and contains additional fields for its type and score, enabling the tokenizer to handle special tokens and different token types efficiently.

| Field         | Description                      | Data Type | Size (bytes) | Notes                           |
|---------------|----------------------------------|-----------|--------------|---------------------------------|
| `token_len`   | Length of each token (per token) | `int32`   | 4            | Prefix for each token's data    |
| `token_data`  | Token string data (UTF-8)        | `string`  | Variable     | UTF-8 encoded byte sequence     |
| `token_score` | Score or frequency of the token  | `float32` | 4            | E.g., log probability           |
| `token_type`  | Token type                       | `int32`   | 4            | Enum: `NORMAL`, `UNKNOWN`, etc. |

- **Token Types**:
  - Using a type enum based on the GGUF `TokenType`, the tokenizer can classify tokens as `NORMAL`, `UNKNOWN`, `CONTROL`, `USER_DEFINED`, `UNUSED`, or `BYTE`.

### **Alignment**

The Tokenizer Section must align to the next 32-byte boundary after all tokens are written. Use `0x00` bytes for padding as needed.

### **Parsing Steps**

1. **Verify Section Marker**:
   - Confirm the 8-byte section marker (`0xBADDCAFE`) for the Tokenizer Section.
2. **Read Section Size**:
   - Retrieve the section size (8 bytes) to determine the total byte length of the section.
3. **Parse Tokenizer Metadata**:
   - Read `vocab_size`, `bos_id`, `eos_id`, `pad_id`, and `unk_id`.
4. **Parse Vocabulary Tokens**:
   - For each token:
     - Read the 4-byte `token_len` and then retrieve `token_data` (UTF-8 string).
     - Extract `token_score` (e.g., log probability).
     - Retrieve `token_type` to classify the token.
5. **Apply Alignment**:
   - Add padding with `0x00` bytes if needed to reach the next 32-byte boundary.

### **Token Types Enum**

Using an enum for token types allows parsers to easily identify special tokens. Here's a proposed enum based on GGUF:

```python
class TokenType(IntEnum):
    NORMAL       = 0
    UNKNOWN      = 1
    CONTROL      = 2
    USER_DEFINED = 3
    UNUSED       = 4
    BYTE         = 5
```

### **Example Binary Layout**

Let's demonstrate a sample Tokenizer Section layout with three tokens, including token type and score fields:

| Offset | Field            | Data         | Size (bytes) | Notes                          |
|--------|------------------|--------------|--------------|--------------------------------|
| 0x00   | `section_marker` | `0xBADDCAFE` | 8            | Tokenizer section identifier   |
| 0x08   | `section_size`   | `0x000000A0` | 8            | Size in bytes                  |
| 0x10   | `vocab_size`     | `0x00000003` | 4            | Vocabulary size of 3 tokens    |
| 0x14   | `bos_id`         | `0x00000001` | 4            | Beginning-of-sequence token ID |
| 0x18   | `eos_id`         | `0x00000002` | 4            | End-of-sequence token ID       |
| 0x1C   | `pad_id`         | `0x00000000` | 4            | Padding token ID               |
| 0x20   | `unk_id`         | `0x00000003` | 4            | Unknown token ID               |
| 0x24   | `token_len`      | `0x00000004` | 4            | Length for "hello"             |
| 0x28   | `token_data`     | "hello"      | Variable     | UTF-8 encoded token string     |
| 0x2C   | `token_score`    | `0.5`        | 4            | Token score                    |
| 0x30   | `token_type`     | `0x00000001` | 4            | Token type `NORMAL`            |
| 0x34   | `token_len`      | `0x00000004` | 4            | Length for "world"             |
| 0x38   | `token_data`     | "world"      | Variable     | UTF-8 encoded token string     |
| 0x3C   | `token_score`    | `0.8`        | 4            | Token score                    |
| 0x40   | `token_type`     | `0x00000001` | 4            | Token type `NORMAL`            |
| 0x44   | `token_len`      | `0x00000001` | 4            | Length for "!"                 |
| 0x48   | `token_data`     | "!"          | Variable     | UTF-8 encoded token string     |
| 0x4C   | `token_score`    | `-1.0`       | 4            | Token score                    |
| 0x50   | `token_type`     | `0x00000003` | 4            | Token type `NORMAL`            |
| 0x54   | Padding          | `0x00`       | 12           | Pad to next 32-byte boundary   |
"""

import struct
from collections import OrderedDict
from typing import Any

from alt.base import BaseModel, CLIParams, TokenType
from alt.loader import ModelLoader
from alt.magic import MagicReader, MagicWriter


class TokenizerModel(BaseModel):
    def __init__(self, cli_params: CLIParams):
        super().__init__(cli_params)

        # Loads sentencepiece processor
        self.processor = ModelLoader(cli_params).load_model_tokenizer()
        self.vocab_size = self.processor.vocab_size()

        # Create a token type object for identifying token types
        self.token_type = TokenType(self.processor)

        # Magic writer and reader
        self.magic_writer = MagicWriter(cli_params)
        self.magic_reader = MagicReader(cli_params)

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
        # Write the Tokenizer section_marker and section_size
        self.magic_writer.write_section_marker(self.magic_type.TOKENIZER, self.calculate_size())

        # vocab_size: 4 bytes
        self.alt_file.write(struct.pack("i", self.vocab_size))

        # bos_id, eos_id, pad_id, unk_id: each 4 bytes
        self.alt_file.write(struct.pack("i", self.processor.bos_id()))
        self.alt_file.write(struct.pack("i", self.processor.eos_id()))
        self.alt_file.write(struct.pack("i", self.processor.pad_id()))
        self.alt_file.write(struct.pack("i", self.processor.unk_id()))

        # For each token in the vocabulary:
        for index in range(self.vocab_size):
            # Extract token related metadata
            token = self.processor.id_to_piece(index)
            token_bytes = token.encode("utf-8")
            token_len = len(token_bytes)
            token_score = self.processor.get_score(index)
            token_type = self.token_type.get_type(index, token)

            # Write token_len and token_data
            self.alt_file.write(struct.pack("i", token_len))
            self.alt_file.write(token_bytes)

            # Write token_score
            self.alt_file.write(struct.pack("f", token_score))

            # Write token_type
            self.alt_file.write(struct.pack("i", token_type))

        # Add alignment padding
        self.magic_writer.write_alignment()

    def read_model(self) -> OrderedDict[str, Any]:
        """Reads the Tokenizer section from the ALT file."""
        # Align the section with the marker
        self.magic_reader.read_alignment()

        # Read the Tokenizer section_marker
        marker, size = self.magic_reader.read_section_marker()
        if not self.magic_type.is_tokenizer(marker):
            raise ValueError(f"Invalid Tokenizer Section marker: {marker}")
        if size != self.calculate_size():
            raise ValueError(f"Section size mismatch: expected {self.calculate_size()}, got {size}")

        # Read vocab_size
        vocab_size = struct.unpack("i", self.alt_file.read(4))[0]

        # Read bos_id, eos_id, pad_id, unk_id
        bos_id, eos_id, pad_id, unk_id = struct.unpack("4i", self.alt_file.read(16))

        # Read tokens
        vocab = []
        for _ in range(vocab_size):
            # Read token_len
            token_len = struct.unpack("i", self.alt_file.read(4))[0]
            # Read token_data
            token = self.alt_file.read(token_len).decode("utf-8")
            # Read token_score
            token_score = struct.unpack("f", self.alt_file.read(4))[0]
            # Read token_type
            token_type = struct.unpack("i", self.alt_file.read(4))[0]
            # Store token information
            token_info = OrderedDict(len=token_len, token=token, score=token_score, type=token_type)
            vocab.append(token_info)
            # Log for debugging

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


# Example Usage
if __name__ == "__main__":
    import logging
    from argparse import ArgumentParser
    from pathlib import Path

    from alt.general import GeneralModel
    from alt.logger import get_default_logger
    from alt.magic import MagicModel
    from alt.parameters import ParametersModel

    parser = ArgumentParser()
    parser.add_argument("directory", help="Path to the model files.")
    parser.add_argument("-v", "--verbose", action="store_true", help="Enable verbose output.")
    args = parser.parse_args()

    # Setup logger and CLI parameters
    logger = get_default_logger(__name__, logging.DEBUG if args.verbose else logging.INFO)
    path = Path(args.directory)
    filename = "tokenizer.alt"
    cli_params = CLIParams(
        directory=args.directory,
        filename=filename,
        verbose=args.verbose,
        logger=logger,
    )

    # Write the ALT file
    logger.info("Writing the ALT file...")
    with open(path / filename, "wb") as alt_write:
        cli_params.alt_file = alt_write

        # Write Start Section
        magic = MagicModel(cli_params=cli_params)
        magic.write_model()

        # Write General Section
        general = GeneralModel(cli_params=cli_params)
        general.write_model()

        hparams = ParametersModel(cli_params=cli_params)
        hparams.write_model()

        tokenizer = TokenizerModel(cli_params=cli_params)
        tokenizer.write_model()

        # Write End Marker
        magic.writer.write_end_marker()

    logger.info("ALT file written successfully.")

    # Read and validate the ALT file
    logger.info("Reading the ALT file...")
    with open(path / filename, "rb") as alt_read:
        cli_params.alt_file = alt_read

        # Read and validate Start Section
        magic_data = magic.read_model()

        # Read and validate General Section
        general_data = general.read_model()

        hparams_data = hparams.read_model()

        tokenizer_data = tokenizer.read_model()

        # Read and validate End Marker
        magic.reader.read_end_marker()

    # Display Metadata
    logger.info("Tokenizer Model Metadata:")
    metadata = OrderedDict()
    metadata.update(magic_data)
    metadata.update(general_data)
    metadata.update(hparams_data)
    metadata.update(tokenizer_data)
    for key, value in metadata.items():
        if "vocab" == key:
            if args.verbose:
                for v in value:
                    print(v)
            continue
        print(f"{key}: {value}")

    logger.info("Operation complete.")
