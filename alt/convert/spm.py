"""
Script: alt/convert/spm.py

Convert a SentencePiece model to the ALT model file format.

Usage:
    python -m alt.convert.spm [options]

Options:
    -h : Show the help message and exit.
    -v : Enable verbose output.
    <model_dir> : Path to the directory containing the sentencepiece model.

Alt Model:

Each section contains a general structure with the exception of the end marker which signifies the end of file. Each section consists of three parts: **Header**, **Fields**, and **Alignment**. The header consists of a section marker and section size. The section size excludes the size of the header and the alignment followed by each section. If section data contains a string, each UTF-8 string is prefixed by a 4-byte integer representing its byte length. Each section is followed by an alignment field, padded as `0x00` up to the next boundary marker.

ALT Start:

| Field             | Description                    | Data Type | Size (bytes) | Notes                |
|-------------------|--------------------------------|-----------|--------------|----------------------|
| `section_marker`  | Identifies file format as ALT  | `int64`   | 8            | `0x616C7400` ("alt") |
| `section_size`    | Total size of the Start Marker | `int64`   | 8            | Includes all fields  |
| `magic_version`   | Version of the ALT file format | `int32`   | 4            | E.g., `2`            |
| `magic_alignment` | Alignment requirement (bytes)  | `int32`   | 4            | E.g., `32`           |

Alt General:

| Field               | Description                    | Data Type | Size (bytes) | Notes                             |
|---------------------|--------------------------------|-----------|--------------|-----------------------------------|
| `section_marker`    | Identifies General Section     | `int64`   | 8            | `0xCAFEBABE`                      |
| `section_size`      | Total size of General Section  | `int64`   | 8            | Includes padding                  |
| `model_type_len`    | Length of model name string    | `int32`   | 4            | Prefix for `model_name`           |
| `model_type`        | Model name (UTF-8)             | `string`  | variable     | e.g., "mistral"                   |
| `base_model_len`    | Length of model name string    | `int32`   | 4            | Prefix for `model_name`           |
| `base_model`        | Model name (UTF-8)             | `string`  | variable     | e.g., "mistralai/Mistral-7B-v0.1" |
| `author_len`        | Length of author name string   | `int32`   | 4            | Prefix for `author`               |
| `author`            | Author name (UTF-8)            | `string`  | variable     | e.g., "MistralAI"                 |
| `created_at_len`    | Length of creation date string | `int32`   | 4            | Prefix for `date_created`         |
| `created_at`        | Model creation date (UTF-8)    | `string`  | variable     | Format `YYYY-MM-DD`               |
| `last_modified_len` | Length of modified date string | `int32`   | 4            | Prefix for `last_modified`        |
| `last_modified`     | Last modified date (UTF-8)     | `string`  | variable     | Format `YYYY-MM-DD`               |
| `license_len`       | Length of license string       | `int32`   | 4            | Prefix for `license`              |
| `license`           | Model license type (UTF-8)     | `string`  | variable     | e.g., "Apache-2.0"                |
| `uuid_len`          | Length of UUID string          | `int32`   | 4            | Prefix for `uuid`                 |
| `uuid`              | Unique model identifier        | `string`  | variable     | e.g., "c1355a8e-..."              |

Alt Parameters:

| Field                     | Description                                                | Data Type | Size (bytes) | Notes                             |
|---------------------------|------------------------------------------------------------|-----------|--------------|-----------------------------------|
| `section_marker`          | Identifies Parameters Section                              | `int64`   | 8            | Set to `0xDEADBEEF`               |
| `section_size`            | Total size of Parameters                                   | `int64`   | 8            | Includes padding                  |
| `hidden_act_len`          | Length of activation function                              | `int32`   | 4            | Prefix for `hidden_act`           |
| `hidden_act`              | Activation function                                        | `string`  | Variable     | E.g. "silu"                       |
| `tie_word_embeddings`     |                                                            | `bool`    | 1            | Optional                          |
| `hidden_size`             | Embedding dimension (hidden size)                          | `int32`   | 4            | E.g., `4096`                      |
| `intermediate_size`       | Feed-forward network size                                  | `int32`   | 4            | Typically `4 * hidden_size`       |
| `max_position_embeddings` | Maximum positions                                          | `int32`   | 4            | Default `32768`                   |
| `num_attention_heads`     | Total number of attention heads                            | `int32`   | 4            | E.g., `32`                        |
| `num_hidden_layers`       | Number of transformer layers                               | `int32`   | 4            | E.g., `32`                        |
| `num_key_value_heads`     | Key-value heads for GQA                                    | `int32`   | 4            | Defaults to `num_attention_heads` |
| `sliding_window`          | Sliding window size                                        | `int32`   | 4            | E.g., `4096`                      |
| `rope_theta`              | Rotary embedding theta                                     | `float32` | 4            | Default `10000.0`                 |
| `rms_norm_eps`            | Epsilon for RMS normalization                              | `float32` | 4            | Default `1e-5`                    |
| `initializer_range`       | Weight initialization range                                | `float32` | 4            | Optional                          |
| `head_size`               | Head size, calculated from hidden size and attention heads | `int32`   | 4            | Computed as `hidden_size / num_attention_heads` |

Alt Tokenizer:

| Field            | Description                      | Data Type | Size (bytes) | Notes          |
|------------------|----------------------------------|-----------|--------------|----------------|
| `section_marker` | Identifies Tokenizer Section     | `int64`   | 8            | `0xBADDCAFE`   |
| `section_size`   | Total size of Tokenizer Section  | `int64`   | 8            | Varies in size |
| `vocab_size`     | Total number of tokens           | `int32`   | 4            |                |
| `bos_id`         | Beginning-of-sequence token ID   | `int32`   | 4            |                |
| `eos_id`         | End-of-sequence token ID         | `int32`   | 4            |                |
| `pad_id`         | Padding token ID                 | `int32`   | 4            |                |
| `unk_id`         | Unknown token ID                 | `int32`   | 4            |                |
| `seq_len`        | Maximum sequence length          | `int32`   | 4            | e.g., `8192`   |
| `token_len`      | Length of each token (per token) | `int32`   | 4            | Prefix for each token's data    |
| `token_data`     | Token string data (UTF-8)        | `string`  | Variable     | UTF-8 encoded byte sequence     |
| `token_score`    | Score or frequency of the token  | `float32` | 4            | E.g., log probability           |
| `token_type`     | Token type                       | `int32`   | 4            | Enum: `NORMAL`, `UNKNOWN`, etc. |

Alt Tensors:

- This script omits the tensors as its purpose is to convert just the tokenizer.

ALT End:

| Field        | Description               | Data Type | Size (bytes) | Value        |
|--------------|---------------------------|-----------|--------------|--------------|
| `end_marker` | Marks the end of the file | `int64`   | 8            | `0x0FFFFFFF` |

- **End Marker Field**: A 8-byte integer (`int64`), set to `0x0FFFFFFF`, that uniquely identifies the end of the file.
- **No Alignment**: There is no padding or alignment after the End Marker; any attempt to read beyond this point is undefined.

NOTE: The byte order is inferred. It is not explicitly provided for flexibility.
"""

import logging
from argparse import ArgumentParser, Namespace
from collections import OrderedDict
from pathlib import Path
from typing import IO, Any, Optional

from sentencepiece import SentencePieceProcessor

from alt.base import CLIParams
from alt.general import GeneralModel
from alt.logger import get_default_logger
from alt.magic import MagicModel
from alt.parameters import ParametersModel
from alt.tokenizer import TokenizerModel


def parse_args() -> Namespace:
    parser = ArgumentParser()
    parser.add_argument("directory", help="Path to the model files.")
    parser.add_argument("-v", "--verbose", action="store_true", help="Enable verbose output.")
    return parser.parse_args()


def main():
    args = parse_args()

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


if __name__ == "__main__":
    main()
