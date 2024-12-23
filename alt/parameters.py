"""
Module: alt/parameters.py

### **3. Parameters Section**

#### **Purpose**

The Parameters Section provides the core configuration values necessary for model inference. These parameters define the model's structure, including its layers, attention mechanism, and normalization details, ensuring accurate and efficient operation.

#### **Structure**

The Parameters Section includes three main parts: **Header**, **Fields**, and **Alignment**.

#### **Header**

| Field            | Description                   | Data Type | Size (bytes) | Notes               |
|------------------|-------------------------------|-----------|--------------|---------------------|
| `section_marker` | Identifies Parameters Section | `int64`   | 8            | Set to `0xDEADBEEF` |
| `section_size`   | Total size of Parameters      | `int64`   | 8            | Includes padding    |

#### **Fields**

##### **Model Configuration**

| Field                     | Description                       | Data Type   | Size (bytes) | Notes                             |
|---------------------------|-----------------------------------|-------------|--------------|-----------------------------------|
| `hidden_act_len`          | Length of activation function     | `int32`     | 4            | Prefix for `hidden_act`           |
| `hidden_act`              | Activation function               | `string`    | Variable     | E.g. "silu"                       |
| `tie_word_embeddings`     |                                   | `bool`      | 1            | Optional                          |
| `hidden_size`             | Embedding dimension (hidden size) | `int32`     | 4            | E.g., `4096`                      |
| `intermediate_size`       | Feed-forward network size         | `int32`     | 4            | Typically `4 * hidden_size`       |
| `max_position_embeddings` | Maximum positions                 | `int32`     | 4            | Default `32768`                   |
| `num_attention_heads`     | Total number of attention heads   | `int32`     | 4            | E.g., `32`                        |
| `num_hidden_layers`       | Number of transformer layers      | `int32`     | 4            | E.g., `32`                        |
| `num_key_value_heads`     | Key-value heads for GQA           | `int32`     | 4            | Defaults to `num_attention_heads` |
| `sliding_window`          | Sliding window size               | `int32`     | 4            | E.g., `4096`                      |
| `rope_theta`              | Rotary embedding theta            | `float32`   | 4            | Default `10000.0`                 |
| `rms_norm_eps`            | Epsilon for RMS normalization     | `float32`   | 4            | Default `1e-5`                    |
| `initializer_range`       | Weight initialization range       | `float32`   | 4            | Optional                          |

##### **Derived Parameters**

| Field         | Description                                                | Data Type | Size (bytes) | Notes                                           |
|---------------|------------------------------------------------------------|-----------|--------------|-------------------------------------------------|
| `head_size`   | Head size, calculated from hidden size and attention heads | `int32`   | 4            | Computed as `hidden_size / num_attention_heads` |

#### **Alignment**

The section must align to the next 32-byte boundary using padding. Add `0x00` bytes as necessary to reach the required alignment.

#### **Parsing Steps**

1. **Verify Section Marker**:
   - Confirm the first 8 bytes match `0xDEADBEEF` to identify the section.
2. **Read Section Size**:
   - Retrieve the 8-byte value to determine the total byte length of the section.
3. **Parse Configuration Fields**:
   - Read each field in sequence as specified:
     - Parse integers for structural details (`hidden_size`, `num_hidden_layers`, etc.).
     - Parse floats for precision-based configurations (`rope_theta`, `rms_norm_eps`).
4. **Calculate Derived Parameters**:
   - Compute any derived fields (e.g., `head_size`) based on the primary configuration fields.
5. **Apply Alignment**:
   - Add padding bytes (`0x00`) to ensure the section aligns with the next 32-byte boundary.
"""

import struct
from collections import OrderedDict
from typing import Any, Optional

from alt.base import BaseModel, CLIParams
from alt.loader import ModelLoader
from alt.magic import MagicReader, MagicWriter


class ParametersModel(BaseModel):
    def __init__(self, cli_params: Optional[CLIParams] = None):
        super().__init__(cli_params)

        # Loads model configuration from config.json
        self.config = ModelLoader(cli_params).load_model_config()

        # Default values for model configuration

        # Type char*
        self.hidden_act = self.config.get("hidden_act", "silu")
        self.hidden_act_len = len(self.hidden_act)

        # Type bool is treated as type int
        self.tie_word_embeddings = self.config.get("tie_word_embeddings", False)

        # Type int
        self.hidden_size = self.config.get("hidden_size", 4096)
        self.intermediate_size = self.config.get("intermediate_size", 4 * self.hidden_size)
        self.max_position_embeddings = self.config.get("max_position_embeddings", 32768)
        self.num_attention_heads = self.config.get("num_attention_heads", 32)
        self.num_hidden_layers = self.config.get("num_hidden_layers", 32)
        self.num_key_value_heads = self.config.get("num_key_value_heads", self.num_attention_heads)
        self.sliding_window = self.config.get("sliding_window", 4096)

        # Type float
        self.rms_norm_eps = self.config.get("rms_norm_eps", 1e-5)
        self.rope_theta = self.config.get("rope_theta", 10000.0)
        self.initializer_range = self.config.get("initializer_range", 0.02)

        # Derived parameter (int)
        self.head_size = self.hidden_size // self.num_attention_heads

        # Magic writer and reader
        self.magic_writer = MagicWriter(cli_params)
        self.magic_reader = MagicReader(cli_params)

    def calculate_size(self) -> int:
        """
        Calculate the size of the Parameters Section.
        Each field is fixed-size (4 bytes for int32, 4 bytes for float32).
        """
        size = 4 + self.hidden_act_len  # len and string
        size += 1  # bool fields
        size += 4 * 7  # int32 fields
        size += 4 * 3  # float32 fields
        size += 4  # 1 int32 derived field (head_size)
        return size

    def write_model(self) -> None:
        """
        Write the Parameters Section to the file.
        """
        # Write section marker and size
        self.magic_writer.write_section_marker(self.magic_type.PARAMETERS, self.calculate_size())

        # Write fields
        fields = [
            (self.hidden_act_len, self.hidden_act),  # len + str
            self.tie_word_embeddings,  # bool as int
            self.hidden_size,  # int
            self.intermediate_size,  # int
            self.max_position_embeddings,  # int
            self.num_attention_heads,  # int
            self.num_hidden_layers,  # int
            self.num_key_value_heads,  # int
            self.sliding_window,  # int
            self.head_size,  # int (derived field)
            self.rms_norm_eps,  # float
            self.rope_theta,  # float
            self.initializer_range,  # float
        ]
        for value in fields:
            if isinstance(value, bool):
                self.alt_file.write(struct.pack("?", value))  # Write bool
            elif isinstance(value, int):  # Short-circuit
                self.alt_file.write(struct.pack("i", value))  # Write int32
            elif isinstance(value, float):
                self.alt_file.write(struct.pack("f", value))  # Write float32
            elif isinstance(value, tuple):
                k, v = value  # Unpack len and string
                self.alt_file.write(struct.pack("i", k))  # len prefix
                self.alt_file.write(v.encode("utf-8"))  # string data

        # Write alignment padding
        self.magic_writer.write_alignment()

    def read_model(self) -> OrderedDict[str, Any]:
        """
        Read the Parameters Section from the file.
        """
        # Read and validate section marker and size
        marker, size = self.magic_reader.read_section_marker()
        if not self.magic_type.is_parameters(marker):
            raise ValueError(f"Invalid Parameters Section marker: {marker}")
        if size != self.calculate_size():
            raise ValueError(f"Section size mismatch: expected {self.calculate_size()}, got {size}")

        # Read fields
        str_t = [
            "hidden_act",
        ]
        bool_t = [
            "tie_word_embeddings",
        ]
        int_t = [
            "hidden_size",
            "intermediate_size",
            "max_position_embeddings",
            "num_attention_heads",
            "num_hidden_layers",
            "num_key_value_heads",
            "sliding_window",
            "head_size",  # Derived field
        ]
        float_t = [
            "rms_norm_eps",
            "rope_theta",
            "initializer_range",
        ]
        fields = str_t + bool_t + int_t + float_t

        metadata = OrderedDict()
        for field in fields:
            value = None  # reset value
            if field in str_t:
                length = struct.unpack("i", self.alt_file.read(4))[0]
                value = self.alt_file.read(length).decode("utf-8")
            elif field in bool_t:
                value = struct.unpack("?", self.alt_file.read(4))[0]
            elif field in int_t:
                value = struct.unpack("i", self.alt_file.read(4))[0]
            elif field in float_t:
                value = struct.unpack("f", self.alt_file.read(4))[0]
            metadata[field] = value

        # Read alignment padding
        self.magic_reader.read_alignment()

        return metadata
