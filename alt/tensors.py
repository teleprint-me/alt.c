"""
Module: alt/tensors.py

### **5. Tensor Section**

#### **Purpose**

The Tensor Section contains the model's weight data, supporting various tensor formats and allowing quantization for optimized storage and computation. The section is organized with a **Header** and **Metadata**, followed by **Per-Tensor Metadata** for each tensor in the model.

#### **Header Fields**

| Field            | Description                               | Data Type | Notes                                    |
|------------------|-------------------------------------------|-----------|------------------------------------------|
| `section_marker` | Uniquely identifies the section           | `int64`   | Set to `0xFACEFEED` for the Tensor Section |
| `section_size`   | Total byte size of the section            | `int64`   | E.g., `4900236` bytes for the Tensor Section |

#### **Configuration**

| Field           | Description                     | Data Type | Size (bytes) | Notes                                  |
|-----------------|---------------------------------|-----------|--------------|----------------------------------------|
| `data_type`     | Primary data type for tensors   | `int32`   | 4            | `0` for float32, `1` for float16, etc. |
| `quant_profile` | Profile for quantized data      | `int32`   | 4            | `0` if unquantized                     |
| `context_len`   | Maximum context length (tokens) | `int32`   | 4            | e.g., `8192`                           |

#### **Metadata Fields**

| Field            | Description                               | Data Type | Notes                                    |
|------------------|-------------------------------------------|-----------|------------------------------------------|
| `tensor_count`   | Total number of tensors in the section    | `int64`   | Includes tensors from all blocks and unique components |
| `shape_count`    | Total number of shape dimensions          | `int64`   | Sum of all dimensions for all tensors    |
| `block_count`    | Total number of blocks (standard layers)  | `int32`   | E.g., `32` for `Mistral-7B`              |
| `unique_count`   | Number of unique components               | `int32`   | E.g., `3` for `embed_tokens`, `lm_head`, `norm` |

#### **Per-Tensor Metadata**

Each tensor in the section is defined by a set of fields providing information on its role, shape, and storage requirements.

| Field              | Description                                 | Data Type | Notes                                    |
|--------------------|---------------------------------------------|-----------|------------------------------------------|
| `component_type`   | Primary model component type                | `int32`   | Identifies `"layers"`, `"embed_tokens"`, `"lm_head"`, `"norm"`, etc. |
| `block_index`      | Block index (positive) or unique (negative) | `int32`   | `0–31` for blocks, `-1` for `embed_tokens`, `-2` for `lm_head`, `-3` for `norm` |
| `layer_type`       | Subdivision within block, if any            | `int32`   | Maps to `"self_attn"`, `"mlp"`, `"input_layernorm"`, etc. |
| `projection_type`  | Subdivision of layer type, if any           | `int32`   | Maps to `"q_proj"`, `"v_proj"`, `"o_proj"`, etc. |
| `n_dims`           | Number of dimensions in the tensor          | `int32`   | E.g., `2` for matrices                 |
| `shape_dimensions` | List of dimension sizes per tensor          | `int32[]` | Sizes of each dimension                |
| `name_len`         | Length of the tensor name                   | `int32`   | Prefix for `tensor_name`               |
| `tensor_name`      | UTF-8 encoded tensor name                   | `string`  | E.g., `"model.layers.0.self_attn.q_proj.weight"` |
| `data_type`        | Storage format for tensor data              | `int32`   | Maps to `float32`, `float16`, `qint8`, `qint4` |
| `delta`            | Scaling factor for quantized data           | `float32` | Only for quantized tensors             |
| `min`              | Minimum value for range-based quantization  | `float32` | Optional; use `0` if not needed        |
| `max`              | Maximum value for range-based quantization  | `float32` | Optional; use `0` if not needed        |
| `packing_flag`     | Indicates packed/unpacked data              | `int8`    | `1` = packed, `0` = unpacked           |
| `tensor_data`      | Raw tensor data                             | Variable  | Serialized according to `data_type`    |

#### **Parsing Steps**

1. **Header Parsing**:
   - Read `section_marker` to confirm the section identity (`0xFACEFEED`).
   - Use `section_size` to calculate the total length of the Tensor Section, enabling efficient validation or skipping if necessary.

2. **Metadata Parsing**:
   - Retrieve `tensor_count`, `shape_count`, `block_count`, and `unique_count` to initialize parsing for the tensors in this section.

3. **Per-Tensor Metadata Parsing**:
   - For each tensor:
     - Read `component_type` and `block_index` to identify the tensor’s primary component and unique vs. block index.
     - Parse `layer_type` and `projection_type` for specific layer and projection identification within blocks.
     - Retrieve `n_dims`, `shape_dimensions`, and `tensor_name` to identify the tensor’s shape and name.
     - Check `data_type` and parse any quantization metadata (`delta`, `min`, `max`).
     - Use `packing_flag` to determine if data is packed before reading `tensor_data`.

4. **Tensor Data Extraction**:
   - Extract tensor data based on `data_type` and apply quantization metadata if applicable.

#### **Example Tensor Metadata**

For a tensor in a standard block layer (`self_attn.q_proj.weight`):

```json
{
  "component_type": "layers",
  "block_index": 0,
  "layer_type": "self_attn",
  "projection_type": "q_proj",
  "n_dims": 2,
  "shape_dimensions": [4096, 4096],
  "name_len": 38,
  "tensor_name": "model.layers.0.self_attn.q_proj.weight",
  "data_type": "float32",
  "delta": 0.0,
  "min": 0.0,
  "max": 0.0,
  "packing_flag": 0,
  "tensor_data": "..."
}
```

And for a unique component (`lm_head.weight`):

```json
{
  "component_type": "lm_head",
  "block_index": -2,
  "layer_type": "weight",
  "projection_type": "",
  "n_dims": 2,
  "shape_dimensions": [32000, 4096],
  "name_len": 14,
  "tensor_name": "lm_head.weight",
  "data_type": "float32",
  "delta": 0.0,
  "min": 0.0,
  "max": 0.0,
  "packing_flag": 0,
  "tensor_data": "..."
}
```

"""

from dataclasses import dataclass

from alt.base import BaseModel, CLIParams, DataType, ProfileType


class Tensors(BaseModel):
    def __init__(self, cli_params: CLIParams):
        super().__init__(cli_params)
        self.data_type = DataType()
        self.profile_type = ProfileType()
