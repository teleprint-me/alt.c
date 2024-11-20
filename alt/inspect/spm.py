"""
Script: alt.inspect.spm

Script for inspecting a SentencePiece `tokenizer.model` file.

Example Usage:
    python -m alt.inspect.spm -h
"""

import sys
from argparse import ArgumentParser

from sentencepiece import SentencePieceProcessor


# Function to read the SentencePiece tokenizer model
def read_model_tokenizer(model_path):
    try:
        tokenizer = SentencePieceProcessor(f"{model_path}/tokenizer.model")
        return tokenizer
    except Exception as e:
        print(f"Error loading tokenizer: {e}")
        sys.exit(1)


# Function to calculate and optionally log tokenizer size with verbosity
def calculate_tokenizer_size(
    tokenizer, verbose=False, limit=None, inspect_token=None, raw_bytes=False
):
    total_size = 5 * 4  # 5 metadata integers (each 4 bytes)
    cumulative_offset = total_size

    # Print general metadata
    print(f"Vocab Size    : {tokenizer.vocab_size()}")
    print(f"BOS ID        : {tokenizer.bos_id()}")
    print(f"EOS ID        : {tokenizer.eos_id()}")
    print(f"PAD ID        : {tokenizer.pad_id()}")
    print(f"UNK ID        : {tokenizer.unk_id()}")

    # Process each token up to the specified limit
    for i in range(tokenizer.vocab_size()):
        if limit and i >= limit:
            break

        token = tokenizer.id_to_piece(i).encode("utf-8")
        token_size = 4 + len(token)  # 4 bytes for length + UTF-8 token data
        total_size += token_size

        if verbose or (inspect_token is not None and inspect_token == i):
            token_text = token.decode("utf-8", errors="replace")
            print(
                f"Token ID {i}: Text='{token_text}', Length={len(token)}, Encoded Size={token_size}, Cumulative Offset={cumulative_offset}"
            )
            if raw_bytes:
                print(f"Raw Bytes: {token}")

        cumulative_offset += token_size

    print(f"Total Tokenizer Size: {total_size} bytes")
    return total_size


def main():
    parser = ArgumentParser(description="Inspect a SentencePiece tokenizer model (.model) file.")
    parser.add_argument(
        "model_path",
        type=str,
        help="Path to the model directory containing 'tokenizer.model'.",
    )
    parser.add_argument("-v", "--verbose", action="store_true", help="Enable verbose token output.")
    parser.add_argument("--limit", type=int, help="Limit the number of tokens to output.")
    parser.add_argument("--inspect-token", type=int, help="Inspect a specific token ID in-depth.")
    parser.add_argument(
        "--raw-bytes", action="store_true", help="Display raw bytes for tokens in verbose output."
    )

    args = parser.parse_args()
    tokenizer = read_model_tokenizer(args.model_path)

    print("Tokenizer Metadata:")
    calculate_tokenizer_size(
        tokenizer,
        verbose=args.verbose,
        limit=args.limit,
        inspect_token=args.inspect_token,
        raw_bytes=args.raw_bytes,
    )


if __name__ == "__main__":
    main()
