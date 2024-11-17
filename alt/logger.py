"""
Module: alt/logger.py

Automate the default logger instance.
"""

import logging
import sys
from logging import Logger

LOGGER_FORMAT = "%(asctime)s - %(filename)s:%(lineno)d - %(levelname)s - %(message)s"


def get_default_logger(
    name: str = None,
    level: int = logging.DEBUG,
    fmt: str = LOGGER_FORMAT,
) -> Logger:
    """Get a default logger instance with specified configuration."""
    logger = logging.getLogger(name if name else "alt")
    if not logger.hasHandlers():
        handler = logging.StreamHandler(stream=sys.stdout)
        formatter = logging.Formatter(fmt)
        handler.setFormatter(formatter)
        logger.addHandler(handler)
        logger.setLevel(level)
    return logger
