"""
Script: examples/qk8.py
"""

import struct
from random import random, seed


def float32_to_float8(value: float) -> int:
    # Extract IEEE-754 components
    bits = struct.unpack("I", struct.pack("f", value))[0]
    sign = (bits >> 31) & 0x1
    exponent = (bits >> 23) & 0xFF
    mantissa = bits & 0x7FFFFF

    # Define parameters for 8-bit float
    e_bias_32 = 127
    e_bias_8 = 3
    max_exponent = 7
    min_exponent = 0

    # Calculate compressed exponent
    e_compressed = max(min((exponent - e_bias_32 + e_bias_8), max_exponent), min_exponent)

    # Calculate compressed mantissa (keep top 4 bits of 23-bit mantissa)
    m_compressed = (mantissa >> 19) & 0xF

    # Pack into 8-bit float
    float8 = (sign << 7) | (e_compressed << 4) | m_compressed
    return float8


def float8_to_float32(float8: int) -> float:
    # Extract components from 8-bit representation
    sign = (float8 >> 7) & 0x1
    e_compressed = (float8 >> 4) & 0x7
    m_compressed = float8 & 0xF

    # Define parameters for 8-bit float
    e_bias_32 = 127
    e_bias_8 = 3

    # Expand exponent
    e_expanded = e_compressed - e_bias_8 + e_bias_32

    # Expand mantissa
    m_expanded = m_compressed / 16.0

    # Reconstruct float
    value = ((-1) ** sign) * (2 ** (e_expanded - e_bias_32)) * (1 + m_expanded)
    return struct.unpack("f", struct.pack("f", value))[0]


def sampler(max_elements: int, z: int) -> list[float]:
    assert max_elements > 0
    assert z >= 2

    values = []
    for i in range(max_elements):
        # Generate a random value in the range [0, 1]
        normalized: float = random()
        # Map the normalized value to the range [-n, n-1]
        values.append(-z + (normalized * (2 * z - 1)))
    return values


def main():
    # Test cases
    seed(1337)
    values = sampler(10, 2**4 - 1)
    for v in values:
        compressed = float32_to_float8(v)
        decompressed = float8_to_float32(compressed)
        print(f"Original: {v}, Compressed: {compressed}, Decompressed: {decompressed}")


if __name__ == "__main__":
    main()
