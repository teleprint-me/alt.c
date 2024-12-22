r"""
Module: alt/magic.py

## File Alignment

Each section aligns to a 32-byte boundary. The required padding is calculated as follows:

- $\text{alignment} = 32$
- $\text{position} = \text{file.tell()}$
- $\text{offset} = \text{position} \mod \text{alignment}$
- $\text{pad} = (\text{alignment} - \text{offset}) \mod \text{alignment}$
- If $\text{pad} > 0$, insert \text{pad} bytes of `0x00` padding.

### Example

For a section ending at byte position $68$:

- $\text{offset} = 68 \mod 32 = 4$
- $\text{pad} = (32 - 4) \mod 32 = 28$
- **Result**: Insert $28$ bytes of `0x00` padding.

## **Start Marker**

The Start Marker identifies the file as adhering to the ALT format, establishing compatibility and alignment requirements for the rest of the file. This ensures parsers can validate the file efficiently and proceed with confidence. If the Start Marker is invalid, the file is considered incompatible, and parsing should halt.

### **Structure**

The Start Marker includes the **Header** and **Fields**, defining the format version, alignment rules, and section size.

### **Header**

| Field            | Description                    | Data Type | Size (bytes) | Notes                 |
|------------------|--------------------------------|-----------|--------------|-----------------------|
| `section_marker` | Identifies file format as ALT  | `int64`   | 8            | `0x616C7400` ("alt") |
| `section_size`   | Total size of the Start Marker | `int64`   | 8            | Includes all fields   |

### **Fields**

| Field             | Description                    | Data Type | Size (bytes) | Notes      |
|-------------------|--------------------------------|-----------|--------------|------------|
| `magic_version`   | Version of the ALT file format | `int32`   | 4            | E.g., `2`  |
| `magic_alignment` | Alignment requirement (bytes)  | `int32`   | 4            | E.g., `32` |

### **Alignment**

- The Start Marker does not include padding between fields.
- The entire section must align to the next 32-byte boundary. Add `0x00` bytes after the last field if necessary to meet alignment requirements.

### **Parsing Steps**

1. **Retrieve and Validate Header**:
   - Read the first 8 bytes and confirm the `magic_number` matches `0x616C7400`.
   - If the magic number is invalid, halt processing and raise an error.
2. **Read Section Size**:
   - Retrieve the next 8 bytes to determine the total size of the Start Marker, including any alignment padding.
3. **Parse Fields**:
   - Read `magic_version` to identify the ALT file format version.
   - Read `magic_alignment` to determine the alignment requirements for subsequent sections.
4. **Apply Alignment**:
   - Calculate the necessary padding to ensure alignment with the next 32-byte boundary.
   - Add `0x00` bytes as required.

## **End Marker**

The End Marker signals the absolute end of the file. Its presence confirms that all preceding sections have been read in full, and any attempt to read past this point is undefined behavior.

### **Structure**

| Field        | Description               | Data Type | Size (bytes) | Value        |
|--------------|---------------------------|-----------|--------------|--------------|
| `end_marker` | Marks the end of the file | `int64`   | 8            | `0x0FFFFFFF` |

- **End Marker Field**: A 8-byte integer (`int64`), set to `0x0FFFFFFF`, that uniquely identifies the end of the file.
- **No Alignment**: There is no padding or alignment after the End Marker; any attempt to read beyond this point is undefined.

### **Parsing Steps**

1. **Verify End Marker**:
   - Read the 8-byte integer value at the current position.
   - Confirm it matches `0x0FFFFFFF` to validate the end of the file.
2. **End of File**:
   - Parsing should terminate here. Any attempt to read past the End Marker is considered undefined behavior.

### **Example Layout Summary**

This summary consolidates the structure and layout of the ALT file format, showing each section's expected alignment and size:

| Offset  | Field                   | Size     | Description                                           |
|---------|-------------------------|----------|-------------------------------------------------------|
| 0       | Start Marker            | 16       | Magic number (`0x616C7400`, "alt") and `section_size` |
| 16      | Fields                  | 8        | `magic_version` and `magic_alignment`                  |
| 24      | Padding                 | 8        | Padding to align to the next 32-byte boundary         |
| 32      | General Section         | Variable | Metadata, including name, author, and UUID            |
| Aligned | HyperParameters Section | Variable | Model hyperparameters prefixed by section header      |
| Aligned | Tokenizer Section       | Variable | Vocabulary, token types, and special token IDs        |
| Aligned | Tensor Section          | Variable | Tensor metadata and packed binary data                |
| Aligned | End Marker              | 4        | End marker (`0x0FFFFFFF`) - Marks end of file         |

### Notes:

- **Aligned Sections**: Each section starts on a 32-byte boundary following its predecessor.
- **Padding**: Padding is only applied where necessary to reach the alignment requirement, and sections do not contain intra-field padding.
- **Variable Size Sections**: Sections like General, Tokenizer, and Tensors vary in size depending on model-specific details but are clearly marked by `section_marker` and `section_size`.
- **Byte Order**: The byte order is inferred. It is not explicitly provided for flexibility.
"""

import os
import struct
from collections import OrderedDict
from typing import Any, Optional, Tuple

from alt.base import BaseMagic, BaseModel, CLIParams


class MagicWriter(BaseMagic):
    def __init__(self, cli_params: Optional[CLIParams] = None):
        super().__init__(cli_params)

    def write_alignment(self) -> None:
        """Write alignment padding to the ALT file."""
        padding_needed = self.calculate_padding()
        if padding_needed > 0:
            self.alt_file.write(b"\x00" * padding_needed)
            self.logger.debug(f"Aligned offset with {padding_needed} bytes of padding.")

    def write_section_marker(self, marker: int, size: int) -> None:
        """Write a section marker and its size."""
        position = self.alt_file.tell()
        self.logger.debug(f"Writing section marker {marker:#x} with size {size} at {position}")
        self.alt_file.write(struct.pack("q", marker))  # 8 bytes
        self.alt_file.write(struct.pack("q", size))  # 8 bytes

    def write_end_marker(self) -> None:
        """Write the end marker."""
        position = self.alt_file.tell()
        self.logger.debug(f"Writing end marker at {position}")
        self.alt_file.write(struct.pack("q", self.magic_type.END))  # Now 8 bytes for consistency


class MagicReader(BaseMagic):
    def __init__(self, cli_params: Optional[CLIParams] = None):
        super().__init__(cli_params)

    def read_alignment(self):
        """Read alignment padding from the ALT file."""
        padding_needed = self.calculate_padding()
        if padding_needed > 0:
            padding = self.alt_file.read(padding_needed)
            if padding != b"\x00" * padding_needed:
                raise ValueError("Alignment padding contains non-zero bytes.")
            self.logger.debug(f"Skipped {padding_needed} bytes of padding.")

    def read_section_marker(self) -> Tuple[int, int]:
        """Read a section marker and its size."""
        marker = struct.unpack("q", self.alt_file.read(8))[0]
        if not self.magic_type.is_valid(marker):
            raise ValueError(f"Invalid section marker: {marker:#x}")
        size = struct.unpack("q", self.alt_file.read(8))[0]
        self.logger.debug(f"Read section marker: {marker:#x}, size: {size}")
        return marker, size

    def read_end_marker(self) -> None:
        """Read the end marker."""
        # NOTE: This is **not** a section handler. It is a null terminator.
        self.read_alignment()
        marker = struct.unpack("q", self.alt_file.read(8))[0]
        if not self.magic_type.is_end(marker):
            raise ValueError(f"Invalid end marker: {marker:#x}")
        self.logger.debug(f"Valid end marker: {marker:#x}")


class MagicModel(BaseModel):
    def __init__(self, cli_params: Optional[CLIParams] = None):
        super().__init__(cli_params)  # Inherit self.cli_params and self.magic_type

    def calculate_size(self) -> int:
        return 4 + 4  # version + alignment = 8 bytes

    def write_model(self) -> None:
        """Write the Start Marker with format version and alignment."""
        # Check write permissions
        if not os.access(self.filepath, os.W_OK):
            raise IOError(f"Write access is not permitted: {self.filepath}")

        # Set the magic writer
        self.writer = MagicWriter(self.cli_params)

        # Write from start of file
        self.writer.alt_file.seek(0)

        # Write the section marker and size
        section_size = self.calculate_size()  # version + alignment = 8 bytes
        self.writer.write_section_marker(self.magic_type.ALT, section_size)

        # Write the fields: 4 byte for version and 4 bytes for alignment
        self.writer.alt_file.write(struct.pack("i", self.magic_type.VERSION))
        self.writer.alt_file.write(struct.pack("i", self.magic_type.ALIGNMENT))
        self.logger.debug(f"Magic Section ends at position {self.alt_file.tell()}")

        # Write the alignment
        self.writer.write_alignment()

    def read_model(self) -> OrderedDict[str, Any]:
        """Read and validate the Start Marker."""
        # Check read permissions
        if not os.access(self.filepath, os.R_OK):
            raise IOError(f"Read access is not permitted: {self.filepath}")

        # Set the magic reader
        self.reader = MagicReader(self.cli_params)

        # Read from start of file
        self.alt_file.seek(0)

        # Read the marker and size
        marker, size = self.reader.read_section_marker()
        if not self.magic_type.is_alt(marker):
            raise ValueError(f"Invalid magic value: {marker}, Size: {size}")

        # Read the version and alignment values
        version = struct.unpack("i", self.alt_file.read(4))[0]
        alignment = struct.unpack("i", self.alt_file.read(4))[0]
        self.logger.debug(f"Magic Version: {version}, Magic Alignment: {alignment}")
        if not self.magic_type.is_version(version):
            raise ValueError(f"Invalid ALT version: {version}")
        if not self.magic_type.is_aligned(alignment):
            raise ValueError(f"Invalid ALT alignment: {alignment}")

        self.reader.read_alignment()
        self.logger.debug("Valid Start Marker read successfully.")
