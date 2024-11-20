"""
Script: alt.cli.convert

Convert the Mistral model to the ALT model file format.

Usage:
    python -m alt.cli.convert [options]

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
