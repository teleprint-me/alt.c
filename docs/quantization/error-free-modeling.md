---
title: "Quantization: Error-Free Modeling"
type: "algorithm"
version: "1"
date: "2024-11-24"
license: "cc-by-nc-sa-4.0"
---

# Quantization: Error-Free Modeling

### **Step 1: Quantization Math**

1. **Input** ($x$):

- Real number input to be quantized.

2. **Integer Domain** ($z_{\text{min}}, z_{\text{max}}$):

- The range of integers available for quantization (e.g., for $8$-bit integers: $[-128, 127]$).
- Integer domain size:

$$
z_{\text{domain}} = z_{\text{max}} - z_{\text{min}}
$$

3. **Real Domain** ($r_{\text{min}}, r_{\text{max}}$):

- Calculated from the magnitude of the input ($x$).
- Define:

$$
r_{\text{max}} = |x|, \quad r_{\text{min}} = -r_{\text{max}}
$$

- Real domain size:

$$
r_{\text{domain}} = r_{\text{max}} - r_{\text{min}}
$$

4. **Squeezing Factor ($\alpha$)**:

- Ensures the real domain fits within the integer domain:

$$
\alpha = 
\begin{cases} 
1, & \text{if } r_{\text{domain}} \leq z_{\text{domain}} \\
\frac{z_{\text{domain}}}{r_{\text{domain}}}, & \text{otherwise.}
\end{cases}
$$

5. **Base Step Size** ($s_{\text{base}}$):

- Represents the granularity of the quantization process:

$$
s_{\text{base}} = \frac{r_{\text{domain}}}{z_{\text{domain}}}
$$

6. **Scalar** ($s$):

- Incorporates $\alpha$ to adjust the step size when squeezing is applied:

$$
s = s_{\text{base}} \cdot \alpha
$$

7. **Quantized Value** ($q$):

- Maps the real input to the integer domain:

$$
q = \text{round}\left(\frac{x}{s}\right)
$$

8. **Residual** ($r$):

- Captures the sub-integer precision lost during quantization:

$$
r = x - q \cdot s
$$

---

### **Step 2: Dequantization Math**

1. **Reconstructed Value** ($x'$):

- Combines the quantized value, scalar, and residual to restore the original input:

$$
x' = q \cdot s / \alpha + r
$$

---

### **Step 3: Storage Requirements**

To achieve error-free reconstruction, the following components are stored:

1. **Quantized Value** ($q$) — $8$ bits.
2. **Scalar** ($s$) — $32$ bits (float).
3. **Squeezing Factor** ($\alpha$) — $32$ bits (float).
4. **Residual** ($r$) — $32$ bits (float).

**Total Storage**:

- $8 + 32 + 32 + 32 = 104$ bits ($13$ bytes) per quantized value.

---

### **Example: Walkthrough for $x = -170.665173$**

1. **Define Integer Domain** ($z_{\text{min}}, z_{\text{max}}$):

$$
z_{\text{min}} = -128, \quad z_{\text{max}} = 127, \quad z_{\text{domain}} = 255
$$

2. **Calculate Real Domain** ($r_{\text{min}}, r_{\text{max}}, r_{\text{domain}}$):

$$
r_{\text{max}} = |x| = 170.665173, \quad r_{\text{min}} = -170.665173
$$

$$
r_{\text{domain}} = r_{\text{max}} - r_{\text{min}} = 341.330346
$$

3. **Calculate Squeezing Factor ($\alpha$)**:

$$
\alpha = \frac{z_{\text{domain}}}{r_{\text{domain}}} = \frac{255}{341.330346} \approx 0.747077
$$

4. **Calculate Base Step Size ($s_{\text{base}}$)**:

$$
s_{\text{base}} = \frac{r_{\text{domain}}}{z_{\text{domain}}} = \frac{341.330346}{255} \approx 1.338552
$$

5. **Calculate Scalar ($s$)**:

$$
s = s_{\text{base}} \cdot \alpha = 1.338552 \cdot 0.747077 \approx 1.000000
$$

6. **Quantize**:

$$
q = \text{round}\left(\frac{x}{s}\right) = \text{round}\left(\frac{-170.665173}{1.000000}\right) = -171
$$

7. **Calculate Residual ($r$)**:

$$
r = x - q \cdot s = -170.665173 - (-171 \cdot 1.000000) = 0.334827
$$

---

### **Step 4: Reconstruction**

1. **Reconstruct the Value**:

$$
x' = q \cdot s / \alpha + r = (-171 \cdot 1.000000 / 0.747077) + 0.334827
$$

$$
x' \approx -170.665173
$$

2. **Check for Error**:

$$
\text{Absolute Error} = |x - x'| = 0
$$

$$
\text{Relative Error} = \frac{\text{Absolute Error}}{|x|} = 0
$$

---

### **Step 5: Optimizations**

1. **Reduce Residual Precision**:
   - Store the residual ($r$) with lower precision (e.g., $16$-bit float or $8$-bit fixed point).

2. **Eliminate Alpha for $\alpha = 1$**:
   - Skip storing $\alpha$ when squeezing isn’t applied.

3. **Dynamic Storage Adjustment**:
   - Use a hybrid approach to store only the necessary components for each input.

---

### **Option 1: 16-bit Components + 8-bit Quantized Value**

- **Storage**: $2 \times 3 + 1 = 7$ bytes.
  - Scalar: $2$ bytes (16-bit float or fixed-point).
  - Alpha: $2$ bytes.
  - Residual: $2$ bytes.
  - Quantized Value: $1$ byte ($8$-bit integer).

- **Upside**: 
  - Higher efficiency in computation compared to 32-bit float-based components.
  - Retains precision for components better than 8-bit while reducing storage from full 32-bit representation.

- **Downside**:
  - Still nearly doubles the storage requirement compared to a single 4-byte float.

---

### **Option 2: Fully Quantized Components**

- Quantize scalar, alpha, and residual into $8$-bit precision.

- **Storage**: $1 \times 3 + 1 = 4$ bytes.
  - Scalar: $1$ byte.
  - Alpha: $1$ byte.
  - Residual: $1$ byte.
  - Quantized Value: $1$ byte.

- **Upside**:
  - Matches the storage of a $32$-bit float ($4$ bytes).
  - Gains efficiency in computation due to $8$-bit operations.

- **Downside**:
  - Precision loss for all components could lead to cumulative errors during dequantization.
  - Limited range for quantized components might fail for edge cases.

---

### **Key Considerations**

#### **Precision vs. Storage**
- **16-bit precision** strikes a middle ground by preserving more accuracy while halving the storage for components compared to $32$-bit floats.
- **8-bit precision** maximizes storage savings but risks excessive cumulative error due to aggressive quantization of components.

#### **Efficiency**
- Smaller bit-width components allow for faster computations (e.g., SIMD operations) and reduced memory bandwidth usage, improving efficiency in constrained environments.

#### **Application Context**
- If reconstruction accuracy is critical, $16$-bit precision might be preferable.
- If storage and computational efficiency are the primary concerns, fully $8$-bit quantized components could be an option.

---

<p align="center">Copyright (C) 2024 Austin Berrio</p>
