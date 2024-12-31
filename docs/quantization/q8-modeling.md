---
title: "Quantization Modeling: Q8"
type: "technical"
version: 1
date: "2024-11-21"
modified: "2024-12-30"
license: "cc-by-nc-sa-4.0"
---

# **Quantization Modeling: Q8**

Quantization reduces memory and computational overhead by representing floating-point values using smaller, integer-based formats (e.g., 4-bit or 8-bit integers). For Q8 quantization, the range $[-128, 127]$ is mapped to 8-bit signed integers, with a fixed scaling factor ensuring consistent precision.

## **Q8 Mathematical Model**

### **Range and Scaling Factor**

Quantization reduces precision by mapping a continuous range of floating-point values to a discrete range of integers. For Q8, the representable range is defined by signed 8-bit integers, which span:

$$
v_{\text{max}} = (2^7 - 1) = 127 \quad \text{and} \quad v_{\text{min}} = -(2^7) = -128
$$

#### **Why $127$ and $-128$?**

1. **Bit Representation**:
   - A signed 8-bit integer reserves 1 bit for the sign, leaving 7 bits for the value.
   - The largest positive value is:

$$
2^7 - 1 = 127
$$

   - The smallest negative value (including zero) is:
$$
-(2^7) = -128
$$

   - Together, this forms the range $[-128, 127]$, with a total of $255$ unique representable values.

2. **Inclusion of Zero**:
   - The interval includes zero, which acts as the midpoint of the range:

$$
v_{\text{min}} = -128 \quad \text{and} \quad v_{\text{max}} = 127
$$

#### **Range and Step Size**

The quantization range is represented as $[-128, 127]$, and the step size or granularity of quantization is determined by the **fixed scalar** $s$, defined as:

$$
s = \frac{v_{\text{max}} - v_{\text{min}}}{(2 \cdot v_{\text{max}}) + 1}
$$

Let’s break this down:

1. **Range Calculation**:

$$
v_{\text{max}} - v_{\text{min}} = 127 - (-128) = 255
$$

This represents the total spread of the signed 8-bit range.

2. **Normalization Denominator**:

$$
(2 \cdot v_{\text{max}}) + 1 = 255
$$

This restores the offset introduced by including zero as part of the signed range.

3. **Resulting Scalar**: Substituting these values:

$$
s = \frac{255}{255} = 1
$$

#### **Key Observations**

1. **Fixed Scalar**:
   - The scalar $s$ becomes $1.0$, acting as a theoretical identity in this implementation.
   - It reflects the relationship between the range and normalization process.

2. **Relationship to Normalization**:
   - Here, $v_{\text{min}}$ and $v_{\text{max}}$ ensure clamping within $[-128, 127]$.
   - The denominator implicitly incorporates the scalar's behavior.
   - The scalar is implicitly baked into the normalization and quantization equations:

$$\text{normalized} = \frac{v_{\text{clamped}} - v_{\text{min}}}{v_{\text{max}} - v_{\text{min}}}$$

3. **Simplified Form**:
   - The scalar does not explicitly alter the computation in this implementation. Instead, it represents the conceptual link between the range and step size.

#### **Practical Use in Quantization**

The range and scaling factor play a critical role in the following steps:

1. **Clamping**: Ensures the input value $v$ fits within the representable range.
2. **Normalization**: Converts $v_{\text{clamped}}$ to a normalized value in $[0, 1]$.
3. **Quantization**: Maps the normalized value to the discrete Q8 range.
4. **Dequantization**: Reconstructs $v$ from $q$ using the inverse of normalization.

### **Quantization**

#### **Clamping**:

First, $v$ is clamped to ensure it fits within the quantization range:

$$
v_{\text{clamped}} = \max(v_{\text{min}}, \min(v, v_{\text{max}}))
$$

#### **Normalization**:

The clamped value is normalized to a range of $[0, 1]$:

$$
\text{normalized} = \frac{v_{\text{clamped}} - v_{\text{min}}}{v_{\text{max}} - v_{\text{min}}}
$$

#### **Quantization**:

The normalized value is scaled to $[-128, 127]$ and rounded to the nearest integer:

$$
q = \text{round}(\text{normalized} \cdot v_{\text{max}})
$$

### **Dequantization**

To reconstruct the approximate floating-point value:

1. Normalize $q$ back to $[0, 1]$:

$$
\text{normalized} = \frac{q}{v_{\text{max}}}
$$

2. Scale back to the original range:

$$
v_{\text{restored}} = \text{normalized} \cdot (v_{\text{max}} - v_{\text{min}}) + v_{\text{min}}
$$

This ensures that the restored value $v_{\text{restored}}$ is an approximation of $v$, with quantization error bounded by the step size $s$.

## **Implementation**

### **C Implementation**

Below is a minimal implementation of Q8 quantization and dequantization:

```c
#include <math.h>
#include <stdint.h>
#include <stdio.h>

#define CLAMP(value, min, max) fmaxf(min, fminf(value, max))

typedef struct {
    float min; /**< Minimum representable value */
    float max; /**< Maximum representable value */
    unsigned char quant; /**< Quantized scalar value */
} Q8;

// 8-bit integer quantization
Q8 quantize_scalar_q8(float value) {
    Q8 q8;

    q8.max = 127.0f; // Signed 8-bit max
    q8.min = -128.0f; // Signed 8-bit min

    // Clamp to range
    float clamped = CLAMP(value, q8.min, q8.max);

    // Normalize and quantize
    float normalized = (clamped - q8.min) / (q8.max - q8.min); // (v + 128) / 255
    q8.quant = (unsigned char)roundf(normalized * q8.max);

    return q8;
}

// Dequantize back to floating-point
float dequantize_scalar_q8(Q8 q8) {
    float normalized = (signed char)q8.quant / q8.max; // norm = q / 127
    return normalized * (q8.max - q8.min) + q8.min; // (norm * 255) - 128
}
```

### **Python Implementation**

```python
def clamp(v, v_min, v_max):
    return max(v_min, min(v, v_max))


def quantize_scalar_q8(v):
    v_max, v_min = 127, -128

    # Clamp and normalize
    clamped = clamp(v, v_min, v_max)
    normalized = (clamped - v_min) / (v_max - v_min)  # (v + 128) / 255

    # Quantize
    q = round(normalized * v_max)
    return q


def dequantize_scalar_q8(q):
    v_max, v_min = 127, -128

    # Denormalize
    denormalized = q / v_max
    v = denormalized * (v_max - v_min) + v_min
    return v


# Example Usage
v = 27.12345
q = quantize_scalar_q8(v)
restored = dequantize_scalar_q8(q)
error = v - restored

print(f"Input: {v}, Quantized: {q}, Restored: {restored}, Error: {error}")
```

## **Testing Results**

### **Sample Output**

For $v = 27.12345$:

| **Step**          | **Value**   |
|-------------------|-------------|
| Input Value       | $27.12345$  |
| Clamped Value     | $27.12345$  |
| Normalized Value  | $0.6083$    |
| Quantized Value   | $77$        |
| Dequantized Value | $26.6063$   |
| Error             | $0.5171$    |

## **Analysis**

1. **Error Analysis**:
   - The error is proportional to the step size $s$, which is approximately $1/255$.

2. **Scalability**:
   - This approach extends seamlessly to row and block quantization.

3. **Key Limitations**:
   - The fixed range $[-128, 127]$ restricts values outside this interval.
   - Precision loss is inherent due to rounding during quantization.
