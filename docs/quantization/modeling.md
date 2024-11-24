---
title: "Quantization Modeling"
type: "technical"
version: "1"
date: "2024-11-21"
license: "cc-by-nc-sa-4.0"
---

# Quantization Modeling

Quantization reduces memory and computational overhead by representing floating-point values using smaller, integer-based formats. Common formats include 4-bit, 8-bit, 16-bit, and 32-bit integers. During quantization, the range $[-n, n]$ is mapped to $n$-bit signed integers using fixed scaling factors to ensure consistent precision.

## N-Bit Modeling

Quantization allows floating-point values to be mapped to their integer counterparts with varying levels of precision. As precision decreases, the loss of information increases. The amount of information preserved depends on the physical limits of the defined set, making quantization a trade-off between memory efficiency and data fidelity.

## **Floating-point and Integer Sets**

The floating-point set of real values defines the **input domain**, while the integer set of values defines the **output domain**.

### **Defining the Input Domain**

The input domain is defined as the set of floating-point values, denoted as $\mathbb{R}$.

#### **The Set of Real Values**

The set $\mathbb{R}$ consists of all floating-point values $r$:

$$
\mathbb{R} = \{r \mid r \in \mathbb{R}\}
$$

#### **Interval of the Input Domain**

The interval of $\mathbb{R}$ can be represented as a range function, denoted $I(x)$, where $x$ represents the magnitude of the input domain. This interval includes all values between $-x$ and $x$:

$$
I(x) = [-x, x]
$$

Here, $x$ defines the maximum absolute value of the floating-point input domain. This interval ensures that the floating-point values are bounded to a specified range.

#### **Defining Inputs for the Real Domain**

To fully describe the input domain, we must explicitly define the upper and lower bounds:

- **Maximum value** ($x_{\text{max}}$):

$$
x_{\text{max}} = 
\begin{cases} 
x & \text{if } x > 0 \\
-x & \text{if } x < 0
\end{cases}
$$

- **Minimum value** ($x_{\text{min}}$):

$$
x_{\text{min}} = 
\begin{cases} 
-x & \text{if } x > 0 \\
x & \text{if } x < 0
\end{cases}
$$

These bounds constrain the domain of $\mathbb{R}$. Values outside $[x_{\text{min}}, x_{\text{max}}]$ are clamped or excluded.

#### **Input Domain Example**

To better understand the interval mapping, let’s consider an example where the floating-point value $r$ is normalized within the range $[-1.0, 1.0]$. This range is commonly chosen for applications in signal processing due to its symmetry around zero and compact representation.

##### **1. Definition of the Input Range**

Let $r = 1.0$. The range $I(x)$ for this domain is:

$$
I(1.0) = [-1.0, 1.0]
$$

This range specifies that all input values are constrained to the interval $[-1.0, 1.0]$.

##### **2. Why Choose $[-1.0, 1.0]$?**

The choice of this range is deliberate and advantageous for several reasons:

1. **Normalization**:

   - Normalized inputs improve numerical stability in algorithms that process floating-point values, such as gradient descent in neural networks.
   - By limiting the range of inputs, we simplify computations, reduce dynamic range requirements, and ensure consistency in numerical operations.

2. **Symmetry Around Zero**:

   - The range is symmetric around zero, making it ideal for systems that model balanced processes, such as audio signals or centered image pixel values.

3. **Compact Representation**:

   - In practice, values within $[-1.0, 1.0]$ can be easily scaled to match other domains, such as $[0, 1]$ for probabilities or $[-128, 127]$ for 8-bit quantized integers.

4. **Practical Use Case**:

   - Consider audio signals, which are often normalized to $[-1.0, 1.0]$ to fit within the constraints of digital hardware while preserving fidelity.
   - Similarly, in image processing, pixel intensities are scaled to $[-1.0, 1.0]$ for computational efficiency in deep learning models.

##### **3. Boundary Behavior**

Values outside the range $[-1.0, 1.0]$ are clamped or rejected. For instance:

- An input of $r = 1.2$ would be clamped to $r = 1.0$.
- An input of $r = -1.5$ would be clamped to $r = -1.0$.

This ensures all inputs adhere to the defined domain, avoiding undefined behavior or numerical instability.

##### **4. Extending the Range**

While $[-1.0, 1.0]$ is a common choice, the interval can be extended based on application needs. For example:

- In signal processing, dynamic ranges of $[-32768, 32767]$ might be used to represent 16-bit audio signals.
- For scaled probabilities, a range of $[0, 1]$ might be more appropriate.

By understanding the rationale behind these intervals, we gain flexibility in defining the input domain while maintaining precision and stability.

##### **Visualizing the Example**
To further enhance comprehension, consider a simple visualization. The normalized range $[-1.0, 1.0]$ can be depicted on a number line:

```
-1.0         0.0         1.0
  |-----------|-----------|
```

Inputs $r$ are bounded within this range. Clamped values (e.g., $r = 1.2$ or $r = -1.5$) are shown as snapping to the nearest boundary.

##### **Conclusion of Example**
This example illustrates how a carefully chosen input domain, like $[-1.0, 1.0]$, serves as a foundation for practical and efficient quantization. By normalizing inputs within this range, we ensure compatibility with downstream systems and maintain the integrity of numerical computations.

### **Defining the Output Domain**

The output domain is defined as the set of integer values, denoted as $\mathbb{Z}$.

#### **The Set of Integer Values**

The set $\mathbb{Z}$ consists of all integer values $z$:

$$
\mathbb{Z} = \{z \mid z \in \mathbb{Z}\}
$$

#### **Interval of the Output Domain**

The interval of $\mathbb{Z}$ can be represented as a range function, denoted $I(y)$, where $y$ represents the magnitude of the output domain. This interval includes all integer values between $-y$ and $y$, inclusive:

$$
I(y) = [-y, y]
$$

This interval defines the bounds within which all quantized integer values lie.

#### **Defining Outputs for the Integer Domain**

To fully describe the output domain, we explicitly define its upper and lower bounds:

- **Maximum value** ($z_{\text{max}}$):

$$
z_{\text{max}} = 
\begin{cases} 
y & \text{if } y > 0 \\
-y & \text{if } y < 0
\end{cases}
$$

- **Minimum value** ($z_{\text{min}}$):

$$
z_{\text{min}} = 
\begin{cases} 
-y & \text{if } y > 0 \\
y & \text{if } y < 0
\end{cases}
$$

These bounds constrain the range of $\mathbb{Z}$, ensuring that all quantized integer values fall within $[z_{\text{min}}, z_{\text{max}}]$. Any value outside this range is clamped.

#### **Output Domain Example**

Let us consider an 8-bit signed integer representation, commonly used in digital systems. In this case:

- The range of values is:

$$
I(127) = [-128, 127]
$$

- **Maximum value**: $z_{\text{max}} = 127$
- **Minimum value**: $z_{\text{min}} = -128$

This interval corresponds to the range of representable values for an 8-bit signed integer. Any quantized value falling outside this range would be clamped to the nearest boundary ($-128$ or $127$).

##### **Why Choose $[-128, 127]$ for 8-bit Integers?**

The interval $[-128, 127]$ is derived from the properties of signed integers in digital systems:
1. **Signed Representation**: An $n$-bit signed integer uses 1 bit for the sign and $n-1$ bits for the magnitude. This allows representation of values from $-2^{n-1}$ to $2^{n-1} - 1$.
2. **Compactness**: This range balances positive and negative values, making it ideal for symmetric datasets such as audio signals or neural network weights.

##### **Connection to the Input Domain**

This output range maps directly to the input domain $[-1.0, 1.0]$, ensuring a proportional and reversible relationship. For example:
- A floating-point value of $1.0$ corresponds to $z = 127$.
- A floating-point value of $-1.0$ corresponds to $z = -128$.
- The quantization scalar $s$ bridges the two domains, ensuring linear mapping.

## **Mapping Floating-point and Integer Sets**

To convert values between representational types, we define a quantization domain $\mathbb{Q}$ that maps between the input set of floating-point values ($\mathbb{R}$) and the output set of integer values ($\mathbb{Z}$).

### **Mapping Transformations**

A mapping consists of two primary transformations:

1. Mapping from the floating-point input domain ($\mathbb{R}$) to the quantized output domain ($\mathbb{Z}$).
2. The inverse mapping, approximating floating-point values from quantized integers, within a margin of error.

### **Mapping Inputs to Outputs**

#### **1. Forward Mapping (Quantization)**

We define the forward quantization mapping as:

$$
\mathbb{Q_{\text{input}}}: \mathbb{R}^+ \to 2^{\mathbb{R}}, \quad x \mapsto [-x, x]
$$

Here:

- $x$ represents the magnitude of the input domain.
- The interval $[-x, x]$ constrains the set of floating-point values $\mathbb{R}$.

The corresponding quantized output set is defined as:

$$
\mathbb{Q_{\text{output}}}: 2^{\mathbb{R}} \to \mathbb{Z}^+, \quad r \mapsto z
$$

Where $r \in [-x, x]$ and $z \in [-y, y]$.

#### **2. Inverse Mapping (Dequantization)**

The inverse mapping approximates floating-point values from quantized integers:

$$
\mathbb{Q_{\text{reverse}}}: \mathbb{Z}^+ \to 2^{\mathbb{R}}, \quad z \mapsto \hat{r}
$$

Here:

- $z$ is the quantized integer value.
- $\hat{r}$ represents the dequantized value, an approximation of the original floating-point input $r$.

This process involves reconstructing $r$ from $z$ using the proportionality defined by the scalar $s$, with a bounded error margin.

### **Formalizing the Mapping**

To ensure precision in the mapping, we introduce the scalar $s$, which linearly relates the intervals of $\mathbb{R}$ and $\mathbb{Z}$:

$$
s = \frac{r_{\text{max}} - r_{\text{min}}}{z_{\text{max}} - z_{\text{min}}}
$$

Using $s$, the quantization process is defined as:

#### **1. Quantization (Real to Integer)**:

For $r \in [r_{\text{min}}, r_{\text{max}}]$:

$$
z = \text{round}\left(\frac{r - r_{\text{min}}}{s}\right) + z_{\text{min}}
$$

This maps floating-point values to integers.

#### **2. Dequantization (Integer to Real)**:

For $z \in [z_{\text{min}}, z_{\text{max}}]$:

$$
\hat{r} = s \cdot (z - z_{\text{min}}) + r_{\text{min}}
$$

This reconstructs the floating-point approximation from integers.

### **Error Bounds**

Quantization introduces a bounded error due to rounding during the mapping process:

- The quantization error $e_q$ is:

$$
e_q = r - \hat{r}
$$

Where $\hat{r}$ is the dequantized approximation of $r$.

- The absolute error is bounded by the scalar:

$$
\lvert e_q \rvert \leq \frac{s}{2}
$$

### **Visualization of the Mapping**

For $[-1.0, 1.0]$ mapped to $[-128, 127]$ (8-bit signed integers):

- Input Domain ($\mathbb{R}$): $[-1.0, 1.0]$
- Output Domain ($\mathbb{Z}$): $[-128, 127]$
- Scalar:

$$
s = \frac{1.0 - (-1.0)}{127 - (-128)} = \frac{2.0}{255} = 0.007843
$$

## **An Algorithm for N-Bit Quantization**

Quantization simplifies the representation of floating-point values by mapping them to a discrete set of integers. While quantizing a single input is straightforward, handling a set of samples allows for more robust calculations of dynamic ranges and scaling factors. This is particularly useful for real-world applications, where data distributions guide the quantization process.

### **Quantization Overview**

Quantization can be broken into five main components:

1. **Calculating the Scalar Value**: Deriving the proportional scaling factor between input and output ranges.
2. **Clamping Input Values**: Ensuring the input values lie within the representable range.
3. **Quantizing Input Values**: Transforming floating-point inputs to integers.
4. **Dequantizing Output Values**: Reconstructing approximate floating-point values.
5. **Error Handling**: Testing and observing errors between inputs and outputs to validate quantization fidelity.

### **1. Calculating the Scalar Value**

The scalar value defines the proportional relationship between the floating-point input range and the integer output range.

#### **Mapping the Input Range**

The input range is defined by its maximum and minimum values:
- **Maximum value**:

$$
r_{\text{max}} = 
\begin{cases} 
x & \text{if } x > 0 \\
-x & \text{if } x < 0
\end{cases}
$$

- **Minimum value**:

$$
r_{\text{min}} = 
\begin{cases} 
-x & \text{if } x > 0 \\
x & \text{if } x < 0
\end{cases}
$$

#### **Mapping the Output Range**

Similarly, the output range is defined as:

- **Maximum value**:

$$
z_{\text{max}} = 
\begin{cases} 
y & \text{if } y > 0 \\
-y & \text{if } y < 0
\end{cases}
$$

- **Minimum value**:

$$
z_{\text{min}} = 
\begin{cases} 
-y & \text{if } y > 0 \\
y & \text{if } y < 0
\end{cases}
$$

#### **Calculating the Scalar Value**

The scalar value $s$ relates the ranges of input and output:

$$
s = \frac{r_{\text{max}} - r_{\text{min}}}{z_{\text{max}} - z_{\text{min}}}
$$

#### **Scalar Implementation**

```c
// Function to calculate the scaling factor based on the input range and bit precision
double scale(double max_reals, unsigned int max_bits) {
    double r_max = fabs(max_reals); // max real
    double r_min = -r_max;          // min real

    signed int z_max = (1 << (max_bits - 1)) - 1; // max integer
    signed int z_min = -(1 << (max_bits - 1));    // min integer

    return (r_max - r_min) / (double)(z_max - z_min);
}
```

### **2. Clamping Input Values**

Clamping ensures that any input value outside the representable range is constrained to the nearest boundary.

#### **Clamping Algorithm**

The clamping function is defined as:

$$
\text{clamp}(x, \text{min}, \text{max}) =
\begin{cases} 
\text{min} & \text{if } x < \text{min} \\
\text{max} & \text{if } x > \text{max} \\
x & \text{otherwise}
\end{cases}
$$

#### **Clamping Implementation**

```c
// Function to clamp a floating-point value to a range
double clamp(double x, double min, double max) {
    if (x < min) {
        return min;
    }
    if (x > max) {
        return max;
    }
    return x;
}
```

### **3. Quantizing Input Values**

Quantization maps a floating-point value $r$ to an integer $z$:

$$
z = \text{round}\left(\frac{r - r_{\text{min}}}{s}\right) + z_{\text{min}}
$$

#### **Implementation**

```c
// Function to quantize a floating-point value to 8-bits
signed char quantize_qint8(double x, double scalar) {
    return (signed char) round(x / scalar);
}
```

### **4. Dequantizing Output Values**

Dequantization reconstructs an approximate floating-point value $\hat{r}$ from an integer $z$:

$$
\hat{r} = s \cdot (z - z_{\text{min}}) + r_{\text{min}}
$$

#### **Implementation**

```c
// Function to dequantize a quantized integer from 8-bits
double dequantize_qint8(signed char q, double scalar) {
    return q * scalar;
}
```

### **5. Error Handling**

Quantization introduces errors due to rounding and mapping floating-point inputs to discrete integers. Observing these errors is crucial for evaluating the quality of the quantization process.

#### **Quantization Errors**

##### **Absolute Error**:

The difference between the original value ($r$) and its dequantized approximation ($\hat{r}$):

$$
e_{\text{absolute}} = |r - \hat{r}|
$$

##### **Relative Error**:

The error as a fraction of the original value ($r$):

$$
e_{\text{relative}} = \frac{|r - \hat{r}|}{|r|}
$$

If $|r| \approx 0$, the relative error is defined as $0$ to avoid division by zero.

#### **Implementation**

```c
// Function to calculate quantization error
void error(double x, double x_prime, double *abs_error, double *rel_error) {
    // Calculate absolute error
    *abs_error = fabs(x - x_prime);

    // Calculate relative error (avoid division by zero)
    if (fabs(x) > 1e-6) {
        *rel_error = fabs(*abs_error / x);
    } else {
        *rel_error = 0.0;
    }
}
```

This function can be integrated into the quantization pipeline to monitor and validate errors for individual samples or entire datasets.

### **Algorithm Overview**

#### **1. Setup**

- Define the input and output ranges:
  - Floating-point range: $[r_{\text{min}}, r_{\text{max}}]$
  - Integer range: $[z_{\text{min}}, z_{\text{max}}]$
- Compute the scalar value $s$ to scale the ranges proportionally:

$$
s = \frac{r_{\text{max}} - r_{\text{min}}}{z_{\text{max}} - z_{\text{min}}}
$$

#### **2. Process Each Sample**

##### **Clamping**:

Constrain the input value $r$ to the input range:

$$
r_{\text{clamped}} = 
\begin{cases} 
r_{\text{min}} & \text{if } r < r_{\text{min}} \\
r_{\text{max}} & \text{if } r > r_{\text{max}} \\
r & \text{otherwise}
\end{cases}
$$

##### **Quantization**:

Map the clamped value to an integer:

$$
z = \text{round}\left(\frac{r_{\text{clamped}} - r_{\text{min}}}{s}\right) + z_{\text{min}}
$$

##### **Dequantization** (Optional):

Reconstruct the floating-point approximation from the quantized value:

$$
\hat{r} = s \cdot (z - z_{\text{min}}) + r_{\text{min}}
$$

##### **Error Calculation** (Optional):

Compute the absolute and relative errors between $r$ and $\hat{r}$.

#### **3. Output**

- Use the quantized values $z$ or reconstructed values $\hat{r}$ as needed.
- Summarize results with:

##### Average absolute error:

$$
\text{Average Absolute Error} = \frac{\sum e_{\text{absolute}}}{N}
$$

##### Average relative error:

$$
\text{Average Relative Error} = \frac{\sum e_{\text{relative}}}{N}
$$

## **Conclusion**

Quantization is a fundamental technique for reducing computational and memory requirements by mapping continuous floating-point values to discrete integer representations. This process is critical in domains like machine learning, digital signal processing, and hardware-constrained systems, where efficiency is paramount.

Through this framework, we have outlined a robust and modular approach to quantization:
1. **Scaling**: Establishing a proportional relationship between input and output ranges ensures fidelity.
2. **Clamping**: Restricting inputs to representable ranges guarantees numerical stability.
3. **Quantization and Dequantization**: Efficiently transforming values between domains while preserving essential characteristics.
4. **Error Analysis**: Validating the process with absolute and relative error metrics ensures practical usability.

By following this structured methodology, users can implement quantization algorithms for a wide range of applications. The provided C implementations and mathematical definitions offer a flexible foundation for experimentation and optimization. Additionally, error analysis enables fine-tuning and ensures that the trade-offs between precision and efficiency align with application-specific requirements.

This framework not only demystifies the quantization process but also bridges theory and practice, empowering developers to deploy high-performance systems while maintaining accuracy and resource efficiency.

---

<p align="center">Copyright (C) 2024 Austin Berrio</p>
