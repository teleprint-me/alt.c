"""
Script: alt.inspect.hash

Usage Example:
    python -m alt.inspect.hash -m models/mistralai/Mistral-7B-Instruct-v0.1
"""

import sys
from argparse import ArgumentParser, Namespace
from hashlib import blake2b
from pathlib import Path

import torch


def load_model_parts(model_dir: str, pattern: str = "pytorch_model-*.bin") -> list[dict]:
    parts = []
    model_path = Path(model_dir)
    for part_path in sorted(model_path.glob(pattern)):
        try:
            model_part = torch.load(part_path, map_location="cpu", weights_only=True)
            parts.append(model_part)
            print(f"Loaded model part: {part_path.name}")
        except Exception as e:
            print(f"Error loading model part {part_path}: {e}")
            sys.exit(1)

    if not parts:
        print(f"Error: No model parts found in {model_dir} matching pattern '{pattern}'")
        sys.exit(1)

    return parts


def calculate_model_hash(model_parts: list[dict], chunk_size: int = 4096) -> str:
    all_hashes = blake2b(digest_size=32)  # 32-byte digest size (adjustable)
    for model_part in model_parts:
        for tensor_name, tensor_weights in model_part.items():
            tensor_hash = blake2b(digest_size=32)
            np_tensor = tensor_weights.to(torch.float16).cpu().numpy().astype("float16")

            # Process tensor in chunks for efficiency
            byte_data = np_tensor.tobytes()
            for i in range(0, len(byte_data), chunk_size):
                tensor_hash.update(byte_data[i : i + chunk_size])

            # Hash tensor name and shape as well
            tensor_hash.update(tensor_name.encode())
            tensor_hash.update(str(np_tensor.shape).encode())

            layer_hash = tensor_hash.hexdigest()
            print(f"{tensor_name} hash: {layer_hash}")

            # Aggregate tensor hash to the overall hash
            all_hashes.update(layer_hash.encode())

    return all_hashes.hexdigest()


def parse_args() -> Namespace:
    parser = ArgumentParser(description="Calculate a hash sum of model weights.")
    parser.add_argument(
        "-m",
        "--model-dir",
        type=str,
        required=True,
        help="Directory containing model parts (.bin files)",
    )
    parser.add_argument(
        "-p",
        "--pattern",
        type=str,
        default="pytorch_model-*.bin",
        help="File pattern for model parts (default: 'pytorch_model-*.bin')",
    )
    parser.add_argument(
        "-c",
        "--chunk-size",
        type=int,
        default=4096,
        help="The size per chunk for each tensor. Default is 4096.",
    )
    return parser.parse_args()


def main():
    args = parse_args()
    model_parts = load_model_parts(args.model_dir, args.pattern)
    hash_sum = calculate_model_hash(model_parts)
    print(f"Hash sum of entire model: {hash_sum}")


if __name__ == "__main__":
    main()
