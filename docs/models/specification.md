---
title: "ALT Model File Format Specification"
type: "technical"
version: "v1"
date: "2024-07-05"
license: "cc-by-nc-sa-4.0"
---

# ALT Model File Format Specification

This document outlines the ALT (Altiera) model file format designed to simplify parsing in C-based executors. The format is organized into structured, sequential sections, each marked by unique identifiers and predefined fields.

## Purpose

This specification aims to facilitate efficient loading and utilization of the Mistral 7B v0.1 model licensed under the Apache License for various applications in natural language processing.

## Objective

A Python script will be utilized to implement this specification, allowing users to convert model files into the custom binary format. The script will accept a directory path, parse the contents of that path, and output a binary file according to the outlined structure. It will also include error handling and validation checks to ensure the integrity of the generated format. This specification serves as a general guide rather than a strict rule.

## File Layout

The ALT file contains the following sections in order:

1. **Start Marker** - Identifies the file format.
2. **Parameters Section** - Contains model-specific hyperparameters essential for model execution.
3. **Tokenizer Section** - Holds tokenizer vocabulary and special token IDs used during processing.
4. **Tensor Section** - Contains tensor metadata and binary data required for model inference.
5. **End Marker** - Marks the end of the file.

### File Alignment

Each section is 32-byte aligned, padded with `0x00` bytes as necessary. The file uses **little-endian** byte order by default.

### File Extension

Model files will be appended with a `.alt` suffix to mitigate confusion with GGUF file formats, as they will be incompatible and require export.

### File Reading and Writing

- The read and write processes should involve the following steps:
  1. **Magic Value Check**: The first 4 bytes contain the "alt" magic value. This verifies that the file is in the expected format and compatible with the current implementation.
  2. **Padding Calculation**: Calculate padding to ensure 32-byte alignment. If the total size of the current position is not aligned, pad with `0x00` bytes as necessary to maintain efficient memory access.
  3. **Section Header Validation**:
     - **Section Marker**: Validate the model section marker against an expected unique magic value in hexadecimal.
       - This marker denotes the beginning of a new section utilized by the model.
       - The marker is an 8-byte label with type `int64`.
     - **Section Size**: Read the model section size to determine the total size of the section.
       - This size indicates the absolute total size of the section, represented by the start and end offsets.
       - The size may be utilized to jump forward to a following section.
       - The size is 8 bytes with type `int64`.
  4. **Section Reading/Writing**:
     - Proceed to read or write each field according to its defined structure:
       - Use types purposefully:
         - Prefer `int32` for integers where possible for efficiency.
         - Use `int64` for fields that need larger integer representations.
         - Use `float32` for floating-point values as required.
       - Ensure that strings are prefixed with their lengths:
         - The length of the string as `int32`.
         - The string as a "utf-8" character string.
       - Store lists, arrays, tensors, etc., as contiguous streams, including:
         - Total count and shape information.
         - The number of dimensions for each sequence.
         - The elements of the object's composition (e.g., [0.1, 0.2, 0.3, 0.4, ...]).
         - All elements must be of a single type.
  5. **End Marker Check**: Repeat the reading/writing process until the end marker (4 bytes, int32) is reached, which indicates the conclusion of the current section and helps verify its integrity. After reaching the end marker, the process should terminate gracefully or reset for another section.

- Most fields will be represented as `int32` for efficiency, while string and floating-point types are retained as necessary to accurately capture model-specific details.
- Each field must adhere to the defined data types, ensuring that the section is correctly read in its entirety.
- Include validation checks during the reading process to confirm that all expected fields are present and formatted correctly, handling any discrepancies gracefully.

### File Structure with Alignment

- **Start Marker**: 
  - Identifies the file format.

- **Parameters Section**: 
  - Stores essential hyperparameters for inference.
  - **Alignment**: Applied preceding this section.

- **Tokenizer Section**: 
  - Contains the tokenizer vocabulary and special token IDs.
  - **Alignment**: Applied preceding this section.

- **Tensor Section**: 
  - Stores weights and other necessary tensors.
  - **Alignment**: Applied preceding this section.

- **End Marker**: 
  - Marks the end of the file.
  - **Alignment**: Applied preceding this section.

### Alignment Calculation

- Calculate the alignment offset as follows:
  - **ALIGNMENT** = 32
  - **POSITION** = file.tell()
  - **OFFSET** = POSITION % ALIGNMENT
  - **PAD** = (ALIGNMENT - OFFSET) % ALIGNMENT
  - If **PAD** > 0, insert **PAD** bytes of `0x00` padding.

### 1. **Start Marker**

- **Purpose**: Identifies the file format as an ALT (Altiera) file format.

- **Field**:
  - `Magic Number (4 bytes, int32)`: `0x616C7463` ("altc" in hex).
    - Where `altc` is shorthand for Altiera Cunningham

- **Offset**: 
  - Starts at byte `0`
  - Ends at byte `4`
  - Followed by a 32-byte aligned offset.

- **Validation**: When reading the file, ensure the first 4 bytes match the specified magic number to verify format compatibility.

### 2. **Parameters Section**

- **Purpose**: Stores essential hyperparameters for inference, ensuring that the model has the necessary configurations for operation.

- **Alignment**: Add 32-byte alignment as previously defined.
  - Begins on the next 32-byte aligned offset after the **Start Marker**.

- **Header**:
  - **Marker**: (8 bytes, int64) `0xDEADBEEF` - A unique identifier for the **Parameters Section**.
  - **Size**: (8 bytes, int64) - Indicates the total size of the **Parameters Section**.

- **Structure**: The parameters section will include the following fields (in the specified order):
  - **Model Type**: 
    - **Length**: (int32) - The length of the model's identifier string.
    - **Identifier**: (string) - The model's identifier (e.g., "mistral").

  - **Model Configuration** (4 bytes each, int32):
    - `hidden_size` (int32): Embedding dimension (hidden size).
    - `num_hidden_layers` (int32): Number of transformer layers (e.g., `32`).
    - `intermediate_size` (int32): Size of the feed-forward networks, typically calculated as `4 * hidden_size` (e.g., `14336`).
    - `num_attention_heads` (int32): Total number of attention heads (e.g., `32`).
    - `num_key_value_heads` (int32): Number of key-value heads for Grouped-Query Attention (default matches `num_attention_heads` if not specified).
    - `sliding_window` (int32): Size of the sliding window attention (e.g., `4096`).
    - `rope_theta` (float32): Parameter for rotary embeddings (default `10000.0`).
    - `rms_norm_eps` (float32): Epsilon for RMS normalization (default `1e-5`).

  - **Model Parameters**:
    - `head_size` (int32): Computed as `hidden_size / num_attention_heads`.
    - `data_type` (int32): Data type indicator (e.g., `0` for float32, `1` for float16, `2` for qint8).
    - `context_length` (int32): Maximum context length for the model (e.g., `8192`).

### 3. **Tokenizer Section**

- **Purpose**: Contains the tokenizer vocabulary and special token IDs necessary for model operations.

- **Alignment**: Add 32-byte alignment as previously defined.
  - Begins on the next 32-byte aligned offset after the **Parameters Section**.

- **Header**:
  - **Marker**: (8 bytes, int64) `0xBADDCAFE`.
  - **Size**: (8 bytes, int64) - Indicates the total size of the tokenizer section.

- **Structure**:
  - **Tokenizer Metadata** (4 bytes each, int32):
    - `vocab_size`: Number of tokens.
    - `bos_id`: Beginning-of-sequence token ID.
    - `eos_id`: End-of-sequence token ID.
    - `pad_id`: Padding token ID.
    - `unk_id`: Unknown token ID.
  - **Vocabulary Tokens**:
    - **Each Token**:
      - **Token Length** (4 bytes, int32): Byte length of the token string.
      - **Token Data** (variable): UTF-8 encoded token string.

### 4. **Tensor Section**

- **Purpose**: Stores weights and other necessary tensors, sequentially ordered for efficient access.

- **Alignment**: Add 32-byte alignment as previously defined.
  - Begins on the next 32-byte aligned offset after the **Tokenizer Section**.

- **Header**:
  - **Marker**: (8 bytes, int64) `0xFACEFEED`.
  - **Size**: (8 bytes, int64) - Indicates the total size of the **Tensor Section**.

- **Structure**:
  - **Tensor Metadata**:
    - `tensor_count` (8 bytes, int64): Total number of tensors.
    - `shape_count` (8 bytes, int64): Total number of shape elements.
  - **Each Tensor**:
    - **Tensor Metadata**:
      - `dimensions` (4 bytes, int32): Number of dimensions.
      - `name_length` (4 bytes, int32): Length of the tensor name.
      - `data_type` (4 bytes, int32): Data type.
    - **Shape Dimensions** (4 bytes each, int32): Sequential dimensions of the tensor.
    - **Tensor Name** (variable): UTF-8 encoded name.
    - **Tensor Data** (variable): Raw data, following the specified `data_type`.

### 5. **End Marker**

- **Purpose**: Marks the end of the file.

- **Alignment**: Add 32-byte alignment as previously defined.
  - Begins on the next 32-byte aligned offset after the **Tensor Section**.

- **Field**:
  - **End Marker**: (4 bytes, int32) `0xFFFFFFF`.

### Example Layout Summary

| Offset       | Field                 | Size   | Description                                     |
|--------------|-----------------------|--------|-------------------------------------------------|
| 0            | Start Marker          | 4      | Magic number (`0x616C7463`, "altc")             |
| 4            | Padding               | 28     | Padding to align to the next 32-byte boundary   |
| 32           | Parameters Section    | Varies | Hyperparameters prefixed by section header      |
| Aligned      | Tokenizer Section     | Varies | Vocabulary and special token IDs                |
| Aligned      | Tensor Section        | Varies | Tensor metadata and binary data                 |
| Aligned      | End Marker            | 4      | End marker (`0xFFFFFFF`)                        |

### Key Notes

1. **Endianess**: Files are little-endian by default. If not specified, assume little-endian.
2. **Alignment**: Each section begins on a 32-byte aligned offset, facilitated by padding as required. The `alt/lib/magic.py` module provides `write_align_offset` and `read_align_offset` functions for handling this alignment during both reading and writing processes.
3. **Token Encoding**: Token strings are UTF-8 encoded to handle multi-byte characters, ensuring compatibility with varied vocabularies and applications across different languages.
4. **Dimension Ordering**: Shape dimensions are stored in sequential order to support tensor indexing operations effectively, improving the performance of tensor-related computations.

## Conclusion

This specification provides a clear layout and simplifies parsing by maintaining a sequential structure, making it efficient and compatible with C-based executors. It serves as a solid foundation for current implementations while allowing for future extensions or modifications as needed.

---

<p align="center">Copyright (C) 2024 Austin Berrio</p>
