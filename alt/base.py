"""
Module: alt/base.py

# ALT Model File Format Specification

This document outlines the ALT (Altiera) model file format, a structured binary format designed for efficient parsing in C-based executors. The format is organized into sequential sections, each marked by unique identifiers and predefined fields, optimized for NLP models like Mistral 7B v0.1.

## Purpose

This specification defines a structured, minimalistic format for loading and utilizing the Mistral 7B v0.1 model (licensed under Apache License). It enables efficient model storage, parsing, and execution in environments where simplicity and performance are essential.

## Objective

A Python-based script will implement this specification, converting model files into the ALT binary format. This script will:

- Accept a directory path containing model data.
- Parse the directory contents.
- Output a binary `.alt` file following the ALT specification.
- Include error handling and validation to ensure the integrity of the generated file.

> **Note**: This specification serves as a general guide and can be adapted for use with other models if needed.

## File Layout

The ALT file consists of the following sections in sequential order:

1. **Start Marker**: Identifies the file as an ALT format.
2. **General Section**: Contains high-level metadata, including name, version, author, and data type.
3. **HyperParameters Section**: Includes specific hyperparameters like layer configurations and attention settings.
4. **Tokenizer Section**: Holds vocabulary and special token IDs for model tokenization.
5. **Tensors Section**: Stores quantized or full-precision tensor data.
6. **End Marker**: A unique marker signifying the absolute end of the file.

### Section Structure

Each section is marked with:

- **Section Marker**: A 64-bit hexadecimal identifier (e.g., `GENERAL = 0xCAFEBABE`) denoting the section type.
- **Section Size**: A 64-bit integer representing the section size in bytes.
- **Section Data**: Binary data specific to the section's function.
"""

import logging
from collections import OrderedDict
from dataclasses import dataclass
from logging import Logger
from pathlib import Path
from typing import IO, Any, Optional

from sentencepiece import SentencePieceProcessor

from alt.logger import get_default_logger


# Definitions for data types and profiles
@dataclass(frozen=True)
class ProfileType:
    UNQUANTIZED = 0
    QINT8 = 1
    QINT4 = 2


@dataclass(frozen=True)
class DataType:
    FLOAT32 = 0
    FLOAT16 = 1
    QINT8 = 2
    QINT4 = 3

    @staticmethod
    def is_quant_profile(data_type: int) -> bool:
        return data_type in (DataType.QINT8, DataType.QINT4)

    @staticmethod
    def is_packed(data_type: int) -> bool:
        return data_type == DataType.QINT4

    @staticmethod
    def get_quant_range(data_type: int) -> int:
        if data_type == DataType.QINT8:
            return 127
        elif data_type == DataType.QINT4:
            return 7
        raise ValueError("Invalid data type for quantization range")


# Token handling with SentencePiece
class TokenType:
    def __init__(self, processor: SentencePieceProcessor):
        self.processor = processor

    def get_type(self, index: int, token: str) -> int:
        if self.processor.is_byte(index):
            return TokenType.BYTE
        elif self.processor.is_control(index):
            return TokenType.CONTROL
        elif self.processor.is_unknown(index):
            return TokenType.UNKNOWN
        elif self.processor.is_unused(index):
            return TokenType.UNUSED
        elif token == "<s>":
            return TokenType.BOS
        elif token == "</s>":
            return TokenType.EOS
        elif token == "<pad>":
            return TokenType.PAD
        return TokenType.NORMAL


@dataclass(frozen=True)
class MagicType:
    ALT = 0x616C7400  # 'alt' in hex; 8 bytes
    GENERAL = 0xCAFEBABE  # 8 bytes
    PARAMETERS = 0xDEADBEEF  # 8 bytes
    TOKENIZER = 0xBADDCAFE  # 8 bytes
    TENSORS = 0xFACEFEED  # 8 bytes
    END = 0x0FFFFFFF  # 8 bytes
    ALIGNMENT = 32  # Default 32-byte alignment
    VERSION = 2  # ALT model file format

    @staticmethod
    def is_valid(marker: int) -> bool:
        return marker in {
            MagicType.ALT,
            MagicType.END,
            MagicType.GENERAL,
            MagicType.PARAMETERS,
            MagicType.TENSORS,
            MagicType.TOKENIZER,
        }

    @staticmethod
    def is_alt(marker: int) -> bool:
        """Check if the marker is the start marker."""
        return marker == MagicType.ALT

    @staticmethod
    def is_general(marker: int) -> bool:
        return marker == MagicType.GENERAL

    @staticmethod
    def is_parameters(marker: int) -> bool:
        return marker == MagicType.PARAMETERS

    @staticmethod
    def is_tokenizer(marker: int) -> bool:
        return marker == MagicType.TOKENIZER

    @staticmethod
    def is_tensors(marker: int) -> bool:
        return marker == MagicType.TENSORS

    @staticmethod
    def is_end(marker: int) -> bool:
        """Check if the marker is the end marker."""
        return marker == MagicType.END

    @staticmethod
    def is_version(value: int) -> bool:
        return value == MagicType.VERSION

    @staticmethod
    def is_aligned(value: int) -> bool:
        return value == MagicType.ALIGNMENT


@dataclass
class CLIParams:
    alt_file: Optional[IO] = None
    directory: str = ""
    filename: str = "model.alt"  # Default output file name
    verbose: bool = False
    data_type: DataType = DataType.FLOAT32
    profile_type: ProfileType = ProfileType.UNQUANTIZED
    context_length: int = 8192
    logger: Optional[logging.Logger] = None


class BaseType:
    def __init__(self, cli_params: Optional[CLIParams] = None):
        self.cli_params = cli_params if cli_params else CLIParams()
        if not self.cli_params.logger:
            self.cli_params.logger = get_default_logger(self.__name__, logging.INFO)
        if self.cli_params.verbose:
            self.cli_params.logger.setLevel(logging.DEBUG)
        self.magic_type = MagicType()

    @property
    def logger(self) -> Logger:
        return self.cli_params.logger

    @property
    def alt_file(self) -> IO:
        return self.cli_params.alt_file

    @property
    def directory(self) -> Path:
        return Path(self.cli_params.directory)

    @property
    def filepath(self) -> Path:
        path = self.directory / self.cli_params.filename
        if not path.suffix == ".alt":
            raise ValueError("Output file must have a .alt extension")
        return path


# Base model class for handling ALT format models
class BaseModel(BaseType):
    def __init__(self, cli_params: Optional[CLIParams] = None):
        super().__init__(cli_params)

    def calculate_size(self) -> int:
        raise NotImplementedError("This method should be implemented by subclasses.")

    def write_model(self) -> None:
        raise NotImplementedError("This method should be implemented by subclasses.")

    def read_model(self) -> OrderedDict[str, Any]:
        raise NotImplementedError("This method should be implemented by subclasses.")


class BaseMagic(BaseType):
    def __init__(self, cli_params: Optional[CLIParams] = None):
        super().__init__(cli_params)

    def calculate_padding(self) -> int:
        """Calculate necessary padding to align to 32 bytes."""
        alignment = self.magic_type.ALIGNMENT
        position = self.alt_file.tell()
        return (alignment - (position % alignment)) % alignment
