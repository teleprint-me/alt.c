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

#### **Model Information**

- **Length Encoding**: Each UTF-8 string is prefixed by a 4-byte integer representing its byte length.

| Field            | Description                   | Data Type | Notes                        |
|------------------|-------------------------------|-----------|------------------------------|
| `model_name_len` | Length of model name string   | `int32`   | Prefix for `model_name`      |
| `model_name`     | Model name (UTF-8)            | `string`  | e.g., "mistral"              |
| `author_len`     | Length of author name string  | `int32`   | Prefix for `author`          |
| `author`         | Author name (UTF-8)           | `string`  | e.g., "MistralAI"            |
| `uuid_len`       | Length of UUID string         | `int32`   | Prefix for `uuid`            |
| `uuid`           | Unique model identifier       | `string`  | e.g., "c1355a8e-..."         |

#### **Optional Metadata**

| Field               | Description                    | Data Type | Notes                      |
|---------------------|--------------------------------|-----------|----------------------------|
| `date_created_len`  | Length of creation date string | `int32`   | Prefix for `date_created`  |
| `date_created`      | Model creation date (UTF-8)    | `string`  | Format `YYYY-MM-DD`        |
| `last_modified_len` | Length of modified date string | `int32`   | Prefix for `last_modified` |
| `last_modified`     | Last modified date (UTF-8)     | `string`  | Format `YYYY-MM-DD`        |
| `license_len`       | Length of license string       | `int32`   | Prefix for `license`       |
| `license`           | Model license type (UTF-8)     | `string`  | e.g., "Apache-2.0"         |

### **Alignment**

The section must align to the next 32-byte boundary using padding. Add `0x00` bytes as necessary to reach the required alignment.

### **Parsing Steps**

1. **Verify Section Marker**:
   - Read the first 8 bytes and confirm they match `0xCAFEBABE`.
2. **Read Section Size**:
   - Retrieve the 8-byte size of the General Section, including padding.
3. **Parse Configuration**:
   - Read each `int32` field for `version`, `data_type`, `quant_profile`, and `context_len`.
4. **Parse Model Information**:
   - For each UTF-8 string field:
     - Read the 4-byte length prefix.
     - Parse the string data of the specified length.
5. **Parse Optional Metadata**:
   - If present, repeat the length-and-string parsing process for optional fields like `date_created` and `license`.
6. **Apply Alignment**:
   - Add padding with `0x00` bytes as necessary to align to the next 32-byte boundary.
"""

from collections import OrderedDict
from typing import Any, Optional

from alt.base import BaseModel, CLIParams
from alt.loader import ModelLoader


class GeneralModel(BaseModel):
    def __init__(self, cli_params: Optional[CLIParams] = None):
        super().__init__(cli_params)  # Inherit self.cli_params and self.magic_type
        self.loader = ModelLoader(cli_params)
        self.model_info = self.loader.load_model_info()  # Returns huggingface_hub.hf_api.ModelInfo

    def calculate_size(self) -> int:
        pass

    def write_model(self) -> None:
        pass

    def read_model(self) -> OrderedDict[str, Any]:
        pass
