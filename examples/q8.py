"""
Mirror the C implementation to the letter.

Only one input and one output is allowed.

Do not deviate.

Lest ye be forsaken.
"""

from dataclasses import dataclass
from random import random, seed


@dataclass
class Q8:
    scalar: float = 0.0
    alpha: float = 0.0
    bits: int = 0
    residual: float = 0.0  # Additional variable to store residual precision


def quantize_scalar_q8(value: float) -> Q8:
    q8 = Q8()

    # Define the maximum possible integer range (example: [z_min, z_max])
    z_domain: int = 255

    # Define the maximum possible real range (example: [r_min, r_max])
    r_max = abs(value)
    r_min = -r_max
    r_domain = r_max - r_min

    # Handle special case for zero input
    if r_domain == 0:
        q8.scalar = 1.0
        q8.alpha = 1.0
        q8.bits = 0
        return q8

    # Calculate alpha (squeezing ratio)
    alpha = 1.0
    if r_domain > z_domain:
        alpha = z_domain / r_domain
    q8.alpha = alpha

    # Calculate the base step size
    base_step_size = r_domain / z_domain  # Decouples the step size from the scalar

    # Quantize the value using the base step size
    q8.bits = int(round(value / base_step_size))

    # Store the residual precision
    q8.residual = value - (q8.bits * base_step_size)

    # Calculate the scalar based on the squeezed range
    q8.scalar = base_step_size * alpha

    return q8


def dequantize_scalar_q8(q8: Q8) -> float:
    # Reconstruct the real value using the quantized value, scalar, and residual
    return (q8.bits * (q8.scalar / q8.alpha)) + q8.residual


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
    # Example values to test the implementation
    seed(1337)
    values = sampler(10, 2**8 - 1)

    print("Testing Quantization and Dequantization:")
    for value in values:
        q8 = quantize_scalar_q8(value)
        reconstructed = dequantize_scalar_q8(q8)
        print(
            f"Input: {value:.6f}, Quantized: {q8.bits}, Scalar: {q8.scalar:.6f}, "
            f"Alpha: {q8.alpha:.6f}, Residual: {q8.residual:.6f}, Reconstructed: {reconstructed:.6f}"
        )
        abs_error = abs(value - reconstructed)
        rel_error = abs_error / abs(value) if abs(value) > 1e-6 else 0.0
        print(f"Absolute Error: {abs_error:.6f}, Relative Error: {rel_error:.6f}")


if __name__ == "__main__":
    main()
