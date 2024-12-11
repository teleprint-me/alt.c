## MNIST MLP

### 1. Network Structure

#### Input Layer
- **Input ($X$)**: $ 768 \times 1 $ (flattened MNIST image, 28x28 pixels).
- $X$ is a column vector, one sample at a time.

#### Hidden Layer 1
- **Weights ($W_1$)**: $128 \times 768$ (128 hidden neurons, 768 inputs).
- **Bias ($b_1$)**: $128 \times 1$ (one bias per hidden neuron).
- **Activation Output ($a_1$)**: $128 \times 1$ after applying the activation function.

#### Hidden Layer 2
- **Weights ($W_2$)**: $10 \times 128$ (10 outputs, 128 inputs from the first hidden layer).
- **Bias ($b_2$)**: $10 \times 1$ (one bias per output neuron).
- **Activation Output ($a_2$)**: $10 \times 1$ after applying the activation function.

#### Output Layer
- **Final Output ($o$)**: $10 \times 1$, corresponding to the 10 classes (digits 0-9).

---

### 2. Forward Pass

#### Hidden Layer 1
1. Compute the input to the first hidden layer:
   $$
   z_1 = W_1 \cdot X + b_1
   $$
   - Dimensions:
     - $W_1$: $128 \times 768$
     - $X$: $768 \times 1$
     - $b_1$: $128 \times 1$
     - $z_1$: $128 \times 1$

2. Apply the activation function:
   $$
   a_1 = \sigma(z_1)
   $$
   - Dimensions:
     - $a_1$: $128 \times 1$

#### Hidden Layer 2
1. Compute the input to the second hidden layer:
   $$
   z_2 = W_2 \cdot a_1 + b_2
   $$
   - Dimensions:
     - $W_2$: $10 \times 128$
     - $a_1$: $128 \times 1$
     - $b_2$: $10 \times 1$
     - $z_2$: $10 \times 1$

2. Apply the activation function:
   $$
   a_2 = \sigma(z_2)
   $$
   - Dimensions:
     - $a_2$: $10 \times 1$

#### Output Layer
The output of the network:
$$
o = a_2
$$
- $o$: $10 \times 1$ (probabilities for each class, often using softmax for classification).

---

### 3. Backward Pass

#### Step 1: Output Layer Gradient
Compute the error at the output:
$$
e = t - o
$$
- $t$: $10 \times 1$ (target vector, one-hot encoded for MNIST).

Gradient of the loss with respect to the output:
$$
g_o = e \cdot \sigma'(z_2)
$$
- Dimensions:
  - $g_o$: $10 \times 1$

---

#### Step 2: Gradients for Hidden Layer 2
Gradients for weights and biases in the second hidden layer:
$$
\Delta W_2 = a_1^T \cdot g_o
$$
$$
\Delta b_2 = \sum g_o
$$
- Dimensions:
  - $a_1^T$: $128 \times 1$ transposed to $1 \times 128$
  - $g_o$: $10 \times 1$
  - $\Delta W_2$: $10 \times 128$
  - $\Delta b_2$: $10 \times 1$

---

#### Step 3: Hidden Layer 1 Gradient
Compute the gradient flowing backward from the second hidden layer:
$$
g_h = (g_o \cdot W_2^T) \cdot \sigma'(z_1)
$$
- Dimensions:
  - $g_o$: $10 \times 1$
  - $W_2^T$: $128 \times 10$ (transpose of $W_2$)
  - $g_h$: $128 \times 1$

Gradients for weights and biases in the first hidden layer:
$$
\Delta W_1 = X^T \cdot g_h
$$
$$
\Delta b_1 = \sum g_h
$$
- Dimensions:
  - $X^T$: $1 \times 768$
  - $g_h$: $128 \times 1$
  - $\Delta W_1$: $128 \times 768$
  - $\Delta b_1$: $128 \times 1$

---

### 4. Update Rules
For all weights and biases:
$$
W_{new} = W_{old} + \eta \cdot \Delta W
$$
$$
b_{new} = b_{old} + \eta \cdot \Delta b
$$

Where $ \eta $ is the learning rate.

---

### Summary of Dimensions
- $W_1$: $128 \times 768$
- $b_1$: $128 \times 1$
- $W_2$: $10 \times 128$
- $b_2$: $10 \times 1$
- Gradients align perfectly with their respective weight and bias matrices.
