"""
Script: alt.inspect.torch

Script for inspecting torch model tensor names, shapes, and weights.

Example Usage:
    python -m alt.inspect.torch -h
"""

import sys
from argparse import ArgumentParser, Namespace
from pathlib import Path
from typing import Dict

import torch


# Get each part of a multi-file PyTorch model
def read_torch_model_parts(
    model_dir: str, pattern: str = "pytorch_model-*.bin", verbose: bool = False
) -> list[Dict[str, torch.Tensor]]:
    """Load the model parts from a given directory."""
    # Get the model parts from a given path
    model_paths = sorted(Path(model_dir).glob(pattern))
    if not model_paths:
        print(f"Error: No model parts found in {model_dir} matching pattern '{pattern}'")
        sys.exit(1)

    # Load and append the model parts to a list
    model_parts = []
    count = 0
    for path in model_paths:
        if verbose:
            print(f"Loading model part: {path.name}")
        model_parts.append(torch.load(path, map_location="cpu", weights_only=True))
        count += 1

    if verbose:
        print(f"Model has {count} parts.")

    # Return the model parts as a list
    return model_parts


def print_layer_names(model_parts: list, verbose: bool = False):
    """Print the names of the layers in the model."""
    for i, part in enumerate(model_parts):
        if verbose:
            print(f"Model Part {i}: Layer Names")
        for name in part.keys():
            print(f"Layer: {name}")


def print_layer_shapes(model_parts: list, verbose: bool = False):
    """Print the shapes of the layers in the model."""
    for i, part in enumerate(model_parts):
        if verbose:
            print(f"Model Part {i}: Layer Shapes")
        for name, weight in part.items():
            print(f"Layer: {name}, Shape: {weight.shape}")


def print_layer_weights(model_parts: list, threshold: int = 10, verbose: bool = False):
    """Print the weights of the layers in the model, summarizing when too large."""
    for i, part in enumerate(model_parts):
        if verbose:
            print(f"Model Part {i}: Layer Weights")
        for name, weight in part.items():
            if weight.numel() <= threshold:  # Adjust this threshold as needed
                print(f"Layer: {name}, Weights: {weight}")
            elif verbose:
                print(f"Layer: {name}, Weights: {weight}")
            else:
                print(
                    f"Layer: {name}, "
                    "Weights Summary: "
                    f"Mean={weight.mean().item()}, "
                    f"Std={weight.std().item()}"
                )


def print_layer_block_count(model_parts: list, verbose: bool = False):
    layer_indices = set()
    layers = 0  # total number of blocks
    sub_layers = 0  # total number of layers within a single block
    for i, part in enumerate(model_parts):
        if verbose:
            print(f"Model Part {i}: Block Count")
        for name, tensor in part.items():
            keys = name.split(".")
            is_layer = len(keys) > 2 and keys[1] == "layers"
            # Check if this is a layer with an index (i.e., a standard numbered layer)
            if is_layer:
                layer_index = keys[2]
                if layer_index not in layer_indices:
                    layer_indices.add(layer_index)
                    layers += 1
                    sub_layers = 0
                sub_layers += 1
            if not is_layer and verbose:
                # Skip non-standard, unique layers like embed_tokens or lm_head
                print(f"Skipping non-standard layer: {name}")

    print(f"Model has {layers} blocks with {sub_layers} sub-layers per block.")


def inspect_model(model_parts: list, args: Namespace):
    """Inspect the model based on the specified arguments."""
    if args.names:
        print_layer_names(model_parts, args.verbose)
    elif args.shapes:
        print_layer_shapes(model_parts, args.verbose)
    elif args.weights:
        print_layer_weights(model_parts, args.weights_threshold, args.verbose)
    elif args.layers:
        print_layer_block_count(model_parts, args.verbose)
    else:
        print("At least one of --names, --shapes, or --weights must be specified.")
        sys.exit(1)


def parse_args():
    parser = ArgumentParser(description="Inspect PyTorch model tensor names, shapes, and weights.")
    parser.add_argument(
        "model_path",
        type=str,
        help="Path to the model directory containing 'pytorch_model-*.bin'.",
    )
    parser.add_argument(
        "-p",
        "--pattern",
        type=str,
        default="pytorch_model-*.bin",
        help="Pattern for the model file names. Default is 'pytorch_model-*.bin'.",
    )
    parser.add_argument(
        "-n",
        "--names",
        action="store_true",
        help="Print detailed information about layers.",
    )
    parser.add_argument(
        "-s",
        "--shapes",
        action="store_true",
        help="Print detailed information about layers and shapes.",
    )
    parser.add_argument(
        "-w",
        "--weights",
        action="store_true",
        help="Print detailed information about layers, shapes, and weights.",
    )
    parser.add_argument(
        "-t",
        "--weights-threshold",
        type=int,  # Ensure this is treated as an integer
        default=10,
        help="Threshold for printing weights directly (default: 10).",
    )
    parser.add_argument(
        "-l",
        "--layers",
        action="store_true",
        help="Output the number of layer per model part.",
    )
    parser.add_argument("-v", "--verbose", action="store_true", help="Increase verbosity.")
    return parser.parse_args()


def main():
    args = parse_args()
    model_parts = read_torch_model_parts(args.model_path, args.pattern, args.verbose)
    inspect_model(model_parts, args)


if __name__ == "__main__":
    main()
