---
title: "Quantization for Large Language Models"
type: "technical"
version: "v1"
date: "2024-07-05"
license: "cc-by-nc-sa-4.0"
---

# Quantization for Large Language Models

Quantization reduces memory usage and computation by representing floating-point values with smaller, integer-based formats (such as 4-bit or 8-bit integers). Each quantization block has a scaling factor, $\delta$, that maps the floating-point range to the integer range for storage and processing efficiency.

### General Formula for Delta Calculation

For a signed **n-bit** quantization, $\delta$ is calculated as:

$$
\delta = \frac{\text{max} - \text{min}}{2^{n-1} - 1}
$$

where:

- **max** and **min** are the maximum and minimum values within the block.
- $2^{n-1} - 1$ represents the maximum positive value for a signed **n-bit** integer.

This formula ensures that $\delta$ scales floating-point values to fit within the signed integer range, preserving as much information as possible.

### q8 Quantization (8-bit)

For **q8**, values are quantized to fit within the signed 8-bit integer range of $[-128, 127]$.

#### 1. Delta Calculation

$$
\delta = \frac{\text{max} - \text{min}}{127}
$$

#### 2. Quantization

To quantize a floating-point value $v$ within the block:

$$
q = \text{round} \left(\frac{v - \text{min}}{\delta}\right)
$$

- $q$ is the quantized 8-bit integer value, stored in `elements` of the `block_q8_0` structure.

#### 3. Dequantization

To reconstruct an approximate value from $q$:

$$
v_{\text{reconstructed}} = q \cdot \delta + \text{min}
$$

### q4 Quantization (4-bit)

For **q4**, values are mapped to fit within the signed 4-bit integer range of $[-8, 7]$. Since true 4-bit storage is unavailable, we pack two 4-bit values into each byte.

#### 1. Delta Calculation

$$
\delta = \frac{\text{max} - \text{min}}{7}
$$

#### 2. Quantization

For each floating-point value $v$ in the block:

$$
q = \text{round} \left(\frac{v - \text{min}}{\delta}\right)
$$

- Each quantized 4-bit value is stored in the `nibbles` array, packing two values per `int8_t` element.

#### 3. Packing Two q4 Values in an `int8_t` Element

Each `int8_t` in `nibbles` holds two 4-bit quantized values (`quant0` and `quant1`):

```c
quantized_nibbles[j] = (quant0 & 0x0F) | (quant1 << 4);
```

- `quant0` occupies the lower nibble (bits 0-3).
- `quant1` is shifted into the upper nibble (bits 4-7).

#### 4. Dequantization

To approximate the original floating-point value from $q$:

$$
v_{\text{reconstructed}} = q \cdot \delta + \text{min}
$$

### Bit Extraction for q4 Packed Data

To retrieve each 4-bit value from the packed `int8_t`:

```c
// Extract lower nibble (quant0)
quant0 = quantized_nibbles[j] & 0x0F;

// Extract upper nibble (quant1)
quant1 = (quantized_nibbles[j] >> 4) & 0x0F;
```

## Quantization Strategy for Transformer Layers

Quantization provides both computational efficiency and memory savings, but it introduces a trade-off between fidelity and compression. This section outlines **layer-specific considerations** for Mistral model quantization, balancing precision needs and computational benefits.

### Quantitative and Qualitative Effects of Quantization

#### Quantitative Analysis

Quantization reduces precision by mapping higher-bit floating-point values to a smaller integer format, impacting **memory usage** and **computational load** while enabling efficient deployment. Key takeaways include:

- **q8 Quantization**: This approach supports high fidelity across a range of applications and model sizes, providing significant compression with minimal performance loss. **8-bit quantization** emerges as optimal for balancing quality and efficiency, suitable for attention mechanisms, MLPs, and inference-focused scenarios.
- **q4 Quantization**: While q4 quantization offers substantial memory savings, it introduces noticeable quality degradation, particularly in smaller models (e.g., 7B parameters and below). Larger models (e.g., Mixtral at 56B parameters) can sustain q4 with lower fidelity loss due to greater parameter redundancy, although minor degradations still occur.

#### Qualitative Analysis

Quantization influences model comprehension, which is inherently more subjective:

- **Critical Layers**: Embedding, output, and normalization layers benefit from higher precision, ensuring stability and quality representation in critical model operations.
- **Attention Mechanisms (e.g., SWA and GQA)**: Selective quantization at q8 within these mechanisms balances memory efficiency with fidelity, supporting long-range dependency modeling without disrupting coherence.
- **Model Size Considerations**: Larger models experience minor qualitative impacts from quantization, whereas smaller models tend to show more noticeable changes in output quality, especially with q4 quantization.

### Key Insights from Quantization Evaluations

- **Consistency Across Tasks**: 8-bit quantized models maintain near-identical performance to full-precision baselines, achieving over 99% accuracy recovery across benchmarks.
- **Structural and Semantic Similarity**: Metrics like ROUGE, BERTScore, and STS confirm that q8 quantization preserves core semantic meaning and structural coherence across output, while q4 can introduce slight variability.
- **Model-Size Dependence**: Larger models (e.g., 70B, 405B) exhibit resilience to quantization, maintaining high fidelity even under q4, while smaller models require q8 for optimal performance retention.

### Layer-Specific Quantization Recommendations

Based on the Mistral model architecture, the following layers are identified for specific quantization strategies:

#### 1. High Precision Layers (16-bit or 32-bit)

These layers require higher fidelity to maintain model comprehension and quality.

- **Embedding Layer** (`model.embed_tokens.weight`): Essential for initial token representation, best stored in f16 or f32.
- **Output Layer** (`lm_head.weight`): Critical for accurate final predictions; higher precision ensures quality output.
- **Normalization Layers** (`input_layernorm.weight`, `post_attention_layernorm.weight`): Stabilizes the model; retain in f16 or f32 for consistent normalization across layers.

#### 2. Mid Precision Layers (8-bit)

These layers can benefit from q8 quantization, striking a balance between computational efficiency and quality retention.

- **Attention Mechanism Layers** (`q_proj`, `k_proj`, `v_proj`, `o_proj`): Essential for SWA and GQA. Quantizing these with q8 maintains high performance while providing memory savings.
- **MLP Layers** (`gate_proj`, `up_proj`, `down_proj`): Dense MLP layers are typically large and benefit significantly from q8 quantization without major loss in fidelity.

#### 3. Low Precision Layers (4-bit)

The q4 format, while not ideal for most primary layers, can be considered for certain auxiliary components if further compression is necessary, though care is advised to avoid performance loss.

## Quantization Profiles

These quantization profiles aim to balance computational efficiency and memory savings with fidelity across different layer types in the Mistral model. By adapting precision levels for each layer type, these profiles optimize model performance while retaining comprehension in critical layers.

### Overview

Two main quantization profiles are provided:

- **Q8 Profile**: A mid-range profile using a mix of f32, f16, and q8 precision for a balance between quality and efficiency.
- **Q4 Profile**: A maximal compression profile with more aggressive quantization, using f32, q8, and q4 to achieve substantial memory savings with some expected quality trade-offs.

### Precision Guide by Layer Type

| Layer Type             | General Recommendation | Q8 Profile Precision      | Q4 Profile Precision       | Reasoning |
|------------------------|------------------------|---------------------------|----------------------------|-----------|
| **Embedding Layer**    | f32                    | f32                       | f32                        | Ensures high-quality token embeddings for initial token representation. |
| **Output Layer**       | f16 or f32             | f32                       | f32                        | Higher precision here preserves output quality, especially under q4 compression. |
| **Layer Normalization**| f32                    | f32                       | f32                        | Full precision stabilizes the model, mitigating degradation effects. |
| **Attention Projections** | f16 or q8           | f16                       | q8                         | Balances fidelity and efficiency for SWA and GQA, supporting robust attention mechanisms. |
| **MLP Layers**         | f16 or q8              | q8                        | q4                         | Dense layers benefit from compression; q4 reduces memory with acceptable quality trade-offs. |
| **Auxiliary Layers**   | q4                    | q8                        | q4                         | Auxiliary components can be maximally compressed in q4, while q8 provides modest efficiency gains. |

### Profile Summaries

#### Q8 Profile (Mid-Range Fidelity)

The **Q8 Profile** uses:

- **f32 precision** for embedding, layer normalization, and output layers to retain full fidelity in these essential layers.
- **f16 precision** for attention projection layers to reduce memory and computation for attention mechanisms without major fidelity loss.
- **q8 precision** for MLP and auxiliary layers to achieve efficient memory use while maintaining a good balance in quality.

#### Q4 Profile (Maximal Compression)

The **Q4 Profile** prioritizes memory savings with:

- **f32 precision** for embedding, layer normalization, and output layers to mitigate comprehension degradation in core layers.
- **q8 precision** for attention layers to balance quality retention with efficient compression.
- **q4 precision** for MLP and auxiliary layers to maximize memory savings, a trade-off suitable for applications where efficiency is critical over quality.

### Considerations for Application

These profiles provide flexibility to adjust based on testing outcomes, specific use cases, or deployment requirements. Adjustments may be necessary to tailor fidelity and efficiency for particular model contexts or performance targets.

## Conclusion

Quantization is a powerful tool for optimizing memory and computation in large language models, enabling efficient deployment without substantially compromising quality when applied thoughtfully. This guide has outlined a detailed approach to balancing precision and compression across various model layers, ensuring that each layer’s fidelity aligns with its function and impact on model comprehension.

### Key Takeaways:

1. **Delta Calculation** provides a straightforward means to map floating-point ranges to integer-based formats, with specific considerations for 8-bit and 4-bit quantization.
   
2. **Layer-Specific Precision**: 
   - **High-fidelity layers** (embedding, normalization, and output) retain f32 or f16 precision to ensure stability and quality representation.
   - **Mid-fidelity layers** (attention and MLPs) balance compression with fidelity by utilizing q8, preserving comprehension in attention mechanisms without excessive memory use.
   - **Low-fidelity auxiliary layers** benefit from q4 quantization where aggressive compression is needed, especially in larger models with inherent parameter redundancy.

3. **Quantization Profiles**:
   - **Q8 Profile** offers a balanced approach for general use cases, combining f32, f16, and q8 to maintain high performance with efficient memory usage.
   - **Q4 Profile** maximizes compression with f32 for critical layers and q8 or q4 for attention and auxiliary layers, suited for memory-constrained deployments.

4. **Quantitative and Qualitative Impacts**:
   - **Quantitative gains** include significant reductions in memory and computation, especially in larger models where q8 can offer near-baseline accuracy.
   - **Qualitative assessments** reveal that larger models are more resilient to lower-bit quantization, while smaller models benefit more from q8 to retain coherence and structural accuracy.

This approach allows flexibility for adjusting precision across model sizes, ensuring that quantization supports both efficient inference and robust comprehension. As future models continue to evolve, this structured, layer-specific quantization strategy will help maintain model quality and deployment efficiency.

---

<p align="center">Copyright (C) 2024 Austin Berrio</p>
