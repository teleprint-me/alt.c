"""
Module: alt/logger.py

Automate the default logger instance.
"""

import logging
import sys


def get_default_logger(name=None, level=logging.DEBUG):
    """Get a default logger instance with specified configuration."""
    LOGGER_FORMAT = "%(asctime)s - %(filename)s:%(lineno)d - %(levelname)s - %(message)s"
    logger = logging.getLogger(name)
    if not logger.hasHandlers():
        handler = logging.StreamHandler(stream=sys.stdout)
        formatter = logging.Formatter(LOGGER_FORMAT)
        handler.setFormatter(formatter)
        logger.addHandler(handler)
        logger.setLevel(level)
    return logger
