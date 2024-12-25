"""
Module: alt/general.py

## **2. General Section**

### **Purpose**
The General Section provides high-level metadata that applies to the entire model, including configuration details, versioning, and attribution. This section supports model loading, version control, and traceability.

### **Structure**
The General Section consists of three parts: **Header**, **Fields**, and **Alignment**.

### **Header**

| Field            | Description                   | Data Type | Size (bytes) | Notes            |
|------------------|-------------------------------|-----------|--------------|------------------|
| `section_marker` | Identifies General Section    | `int64`   | 8            | `0xCAFEBABE`     |
| `section_size`   | Total size of General Section | `int64`   | 8            | Includes padding |

### **Fields**

- **Length Encoding**: Each UTF-8 string is prefixed by a 4-byte integer representing its byte length.

| Field               | Description                    | Data Type | Notes                             |
|---------------------|--------------------------------|-----------|-----------------------------------|
| `model_type_len`    | Length of model name string    | `int32`   | Prefix for `model_name`           |
| `model_type`        | Model name (UTF-8)             | `string`  | e.g., "mistral"                   |
| `base_model_len`    | Length of model name string    | `int32`   | Prefix for `model_name`           |
| `base_model`        | Model name (UTF-8)             | `string`  | e.g., "mistralai/Mistral-7B-v0.1" |
| `author_len`        | Length of author name string   | `int32`   | Prefix for `author`               |
| `author`            | Author name (UTF-8)            | `string`  | e.g., "MistralAI"                 |
| `created_at_len`    | Length of creation date string | `int32`   | Prefix for `date_created`         |
| `created_at`        | Model creation date (UTF-8)    | `string`  | Format `YYYY-MM-DD`               |
| `last_modified_len` | Length of modified date string | `int32`   | Prefix for `last_modified`        |
| `last_modified`     | Last modified date (UTF-8)     | `string`  | Format `YYYY-MM-DD`               |
| `license_len`       | Length of license string       | `int32`   | Prefix for `license`              |
| `license`           | Model license type (UTF-8)     | `string`  | e.g., "Apache-2.0"                |
| `uuid_len`          | Length of UUID string          | `int32`   | Prefix for `uuid`                 |
| `uuid`              | Unique model identifier        | `string`  | e.g., "c1355a8e-..."              |

### **Alignment**

The section must align to the next 32-byte boundary using padding. Add `0x00` bytes as necessary to reach the required alignment.

### **Parsing Steps**

1. **Verify Section Marker**:
   - Read the first 8 bytes and confirm they match `0xCAFEBABE`.
2. **Read Section Size**:
   - Retrieve the 8-byte size of the General Section.
3. **Parse Model Information**:
   - For each UTF-8 string field:
     - Read the 4-byte length prefix.
     - Parse the string data of the specified length.
4. **Apply Alignment**:
   - Add padding with `0x00` bytes as necessary to align to the next 32-byte boundary.
"""

import struct
from collections import OrderedDict
from typing import Any, Optional
from uuid import uuid4

from alt.base import BaseModel, CLIParams
from alt.loader import ModelLoader
from alt.magic import MagicReader, MagicWriter


class GeneralModel(BaseModel):
    def __init__(self, cli_params: Optional[CLIParams] = None):
        # Inherit self.cli_params and self.magic_type
        super().__init__(cli_params)

        # Add the magic writer and reader
        self.magic_writer = MagicWriter(cli_params)
        self.magic_reader = MagicReader(cli_params)

        # Get huggingface_hub.hf_api.ModelInfo
        self.model_info = ModelLoader(cli_params).load_model_info()

        # Extract the model info from the repo
        self.model_type = self.model_info.config["model_type"]
        self.model_type_len = len(self.model_type)

        self.base_model = self.model_info.card_data.base_model
        self.base_model_len = len(self.base_model)

        self.author = self.model_info.author
        self.author_len = len(self.author)

        self.created_at = self.model_info.created_at.isoformat()
        self.created_at_len = len(self.created_at)

        self.last_modified = self.model_info.last_modified.isoformat()
        self.last_modified_len = len(self.last_modified)

        self.license = self.model_info.card_data.license
        self.license_len = len(self.license)

        # Generate a unique identifier for the model file
        self.uuid = str(uuid4())  # Convert UUID to string
        self.uuid_len = len(self.uuid)

    def calculate_size(self) -> int:
        """
        Calculate the size of the General Section.
        This includes the length prefix (4 bytes) and actual string data for each field.
        """
        size = 4 + self.author_len
        size += 4 + self.base_model_len
        size += 4 + self.model_type_len
        size += 4 + self.created_at_len
        size += 4 + self.last_modified_len
        size += 4 + self.license_len
        size += 4 + self.uuid_len
        return size

    def write_model(self) -> None:
        """
        Write the General Section data to the file.
        """
        # Set the section marker and size
        self.magic_writer.write_section_marker(self.magic_type.GENERAL, self.calculate_size())

        # Write metadata fields with their length prefixes
        fields = [
            (self.model_type_len, self.model_type),
            (self.base_model_len, self.base_model),
            (self.author_len, self.author),
            (self.created_at_len, self.created_at),
            (self.last_modified_len, self.last_modified),
            (self.license_len, self.license),
            (self.uuid_len, self.uuid),
        ]
        for length, value in fields:
            self.alt_file.write(struct.pack("i", length))  # Write length prefix (int32)
            self.alt_file.write(value.encode("utf-8"))  # Write string data

        # Write alignment padding
        self.magic_writer.write_alignment()

    def read_model(self) -> OrderedDict[str, Any]:
        """Read the General Section data from the file."""
        # Get the section marker and size
        marker, size = self.magic_reader.read_section_marker()
        # Validate the marker and size
        if not self.magic_type.is_general(marker):
            raise ValueError(f"Invalid section marker: {marker}")
        if size != self.calculate_size():
            raise ValueError(f"Section size mismatch: expected {self.calculate_size()}, got {size}")

        # Read the general metadata from the file
        metadata = OrderedDict()
        fields = [
            "model_type",
            "base_model",
            "author",
            "created_at",
            "last_modified",
            "license",
            "uuid",
        ]
        for field in fields:
            length = struct.unpack("i", self.alt_file.read(4))[0]
            value = self.alt_file.read(length).decode("utf-8")
            metadata[field] = value

        # Read alignment padding
        self.magic_reader.read_alignment()

        return metadata


# Example Usage
if __name__ == "__main__":
    import logging
    from argparse import ArgumentParser
    from pathlib import Path

    from alt.logger import get_default_logger
    from alt.magic import MagicModel

    parser = ArgumentParser()
    parser.add_argument("directory", help="Path to the model files.")
    parser.add_argument("-v", "--verbose", action="store_true", help="Enable verbose output.")
    args = parser.parse_args()

    # Setup logger and CLI parameters
    logger = get_default_logger(__name__, logging.DEBUG if args.verbose else logging.INFO)
    path = Path(args.directory)
    filename = "general.alt"
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

        # Read and validate End Marker
        magic.reader.read_end_marker()

    # Display General Section Metadata
    logger.info("General Section Metadata:")
    for key, value in general_data.items():
        print(f"{key}: {value}")

    logger.info("Operation complete.")
