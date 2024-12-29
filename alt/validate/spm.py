"""
Script: alt/validate/spm.py
"""

import logging
from argparse import ArgumentParser, Namespace
from collections import OrderedDict
from typing import Any

from alt.base import BaseModel, CLIParams
from alt.general import GeneralModel
from alt.logger import get_default_logger
from alt.magic import MagicModel
from alt.parameters import ParametersModel
from alt.tokenizer import TokenizerModel


class AltTokenizer(BaseModel):
    def __init__(self, cli_params: CLIParams):
        # Set input parameters
        self.cli_params = cli_params

        # Create model sections
        self.magic = MagicModel(cli_params=cli_params)
        self.general = GeneralModel(cli_params=cli_params)
        self.hparams = ParametersModel(cli_params=cli_params)
        self.tokenizer = TokenizerModel(cli_params=cli_params)

    def write_model(self) -> None:
        # Write the ALT file
        self.logger.info("Writing the ALT file...")
        with open(self.filepath, "wb") as alt_write:
            # Update the IO file
            self.alt_file = alt_write
            # Write model sections
            self.magic.write_model()
            self.general.write_model()
            self.hparams.write_model()
            self.tokenizer.write_model()
            # Write model end of file
            self.magic.writer.write_end_marker()
        self.logger.info("ALT file written successfully.")

    def read_model(self) -> OrderedDict[str, Any]:
        # Read and validate the ALT file
        self.logger.info("Reading the ALT file...")
        metadata = OrderedDict()
        with open(self.filepath, "rb") as alt_read:
            # Update the IO file
            self.alt_file = alt_read
            # Read model sections and aggregate metadata
            metadata.update(self.magic.read_model())
            metadata.update(self.general.read_model())
            metadata.update(self.hparams.read_model())
            metadata.update(self.tokenizer.read_model())
            # Read and validate End Marker
            self.magic.reader.read_end_marker()
        return metadata


def parse_args() -> Namespace:
    parser = ArgumentParser()
    parser.add_argument("directory", help="Path to the model files.")
    parser.add_argument(
        "-f",
        "--filename",
        default="tokenizer.alt",
        help="Output filename. Default is 'tokenizer.alt'",
    )
    parser.add_argument("-v", "--verbose", action="store_true", help="Enable verbose output.")
    return parser.parse_args()


def main():
    args = parse_args()

    # Setup logger and CLI parameters
    filename = args.filename
    logger = get_default_logger(__name__, logging.DEBUG if args.verbose else logging.INFO)
    cli_params = CLIParams(
        directory=args.directory,
        filename=filename,
        verbose=args.verbose,
        logger=logger,
    )

    alt_tokenizer = AltTokenizer(cli_params)
    metadata = alt_tokenizer.read_model()
    logger.info("Tokenizer Model Metadata:")
    for key, value in metadata.items():
        if "vocab" == key:
            if args.verbose:
                for v in value:
                    print(v)
            continue
        print(f"{key}: {value}")
    logger.info(f"Successfully created {alt_tokenizer.filepath}.")


if __name__ == "__main__":
    main()
