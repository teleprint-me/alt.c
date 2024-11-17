"""
Module: alt/loader.py

Loads required model data from a given directory.
"""

import json
import sys
from typing import Any, Dict, List

import torch
from huggingface_hub import model_info
from huggingface_hub.hf_api import ModelInfo
from sentencepiece import SentencePieceProcessor

from alt.base import BaseType, CLIParams
from alt.logger import get_default_logger


class ModelLoader(BaseType):
    def __init__(self, cli_params: CLIParams):
        super().__init__(cli_params)

    def load_model_info(self) -> ModelInfo:
        """Loads metadata from the Hugging Face repo."""
        repo_name = "/".join(self.directory.parts[-2:])
        try:
            info = model_info(repo_name)
            self.logger.debug("Model info loaded successfully.")
        except Exception:
            self.logger.error("Use huggingface-cli login to register metadata.")
        return info

    def load_model_config(self) -> Dict[str, Any]:
        """Loads model configuration from config.json."""
        config_path = self.directory / "config.json"
        try:
            with config_path.open("r") as f:
                self.config = json.load(f)
                self.logger.debug("Model configuration loaded successfully.")
        except FileNotFoundError:
            self.logger.error(f"config.json not found in {config_path}")
            sys.exit(1)
        except json.JSONDecodeError:
            self.logger.error("Could not decode config.json")
            sys.exit(1)
        return self.config

    def load_model_tokenizer(self) -> SentencePieceProcessor:
        """Loads the SentencePiece tokenizer model."""
        tokenizer_path = self.directory / "tokenizer.model"
        try:
            self.tokenizer = SentencePieceProcessor(model_file=str(tokenizer_path))
            self.logger.debug("Tokenizer model loaded successfully.")
        except Exception as e:
            self.logger.error(f"Failed to load SentencePiece tokenizer: {e}")
            sys.exit(1)
        return self.tokenizer

    def load_model_parts(self, pattern="pytorch_model-*.bin") -> List[Dict[str, torch.Tensor]]:
        """Loads model weight parts according to a specified file pattern."""
        model_parts = []
        for part_path in sorted(self.directory.glob(pattern)):
            try:
                model_part = torch.load(part_path, map_location="cpu", weights_only=True)
                model_parts.append(model_part)
                self.logger.debug(f"Loaded model part: {part_path.name}")
            except Exception as e:
                self.logger.error(f"Failed to load model part {part_path}: {e}")
                sys.exit(1)

        if not model_parts:
            self.logger.error(
                f"No model parts found in {self.directory} matching pattern '{pattern}'"
            )
            sys.exit(1)

        self.parts = model_parts
        return self.parts

    def load_all(self) -> None:
        """Convenience method to load all data at once."""
        self.load_model_info()
        self.load_model_config()
        self.load_model_tokenizer()
        self.load_model_parts()
        self.logger.debug("All model data loaded successfully.")


# Example Usage
if __name__ == "__main__":
    import logging
    from argparse import ArgumentParser

    parser = ArgumentParser()
    parser.add_argument("directory", help="Path to the models files.")
    parser.add_argument("-v", "--verbose", action="store_true", help="Enable verbose output.")
    args = parser.parse_args()

    logger = get_default_logger(__name__, logging.INFO)
    cli_params = CLIParams(directory=args.directory, verbose=args.verbose, logger=logger)
    loader = ModelLoader(cli_params)
    loader.load_all()

    if not args.verbose:
        logger.info("Fin.")
