/**
 * @file src/alt.c
 */

#include <math.h>
#include <stdio.h>

#define L 3 // Sequence length
#define DK 4 // Embedding size (d_k)
#define V 10 // Vocabulary size
#define D 4 // Embedding dimension

// n-dimensional layer
struct layer {
    float* weights;
    float* biases;
    float* gamma;
    float* beta;
};

// Function to embed tokens
void embed_tokens(
    const int* token_indices,
    int seq_len,
    const float embedding_matrix[V][D],
    float embeddings[L][D]
) {
    for (int i = 0; i < seq_len; i++) {
        int token_index = token_indices[i]; // Get the token index
        for (int j = 0; j < D; j++) {
            embeddings[i][j] = embedding_matrix[token_index][j]; // Copy the embedding vector
        }
    }
}

// Helper function to calculate dot product
float dot_product(const float* vec1, const float* vec2, int size) {
    float result = 0.0f;
    for (int i = 0; i < size; i++) {
        result += vec1[i] * vec2[i];
    }
    return result;
}

// Softmax function
void softmax(float* scores, int length) {
    float max_score = scores[0];
    for (int i = 1; i < length; i++) {
        if (scores[i] > max_score) {
            max_score = scores[i];
        }
    }

    float sum = 0.0f;
    for (int i = 0; i < length; i++) {
        scores[i] = exp(scores[i] - max_score); // Shift for numerical stability
        sum += scores[i];
    }

    for (int i = 0; i < length; i++) {
        scores[i] /= sum;
    }
}

// Self-attention function
void self_attention(
    const float* query, const float key[L][DK], const float value[L][DK], float* output
) {
    float scores[L];

    // Step 1: Compute attention scores
    for (int i = 0; i < L; i++) {
        scores[i] = dot_product(query, key[i], DK) / sqrt(DK); // Scaled dot product
    }

    // Step 2: Apply softmax to scores
    softmax(scores, L);

    // Step 3: Compute weighted sum of values
    for (int j = 0; j < DK; j++) {
        output[j] = 0.0f; // Initialize output
    }
    for (int i = 0; i < L; i++) { // Sequence length
        for (int j = 0; j < DK; j++) { // Embedding size
            output[j] += scores[i] * value[i][j];
        }
    }
}

void layer_norm(
    const float* input, int size, const float* gamma, const float* beta, float* output
) {
    // Compute mean
    float mean = 0.0f;
    for (int i = 0; i < size; i++) {
        mean += input[i];
    }
    mean /= size;

    // Compute variance
    float variance = 0.0f;
    for (int i = 0; i < size; i++) {
        variance += (input[i] - mean) * (input[i] - mean);
    }
    variance /= size;

    // Normalize and apply scale and shift
    for (int i = 0; i < size; i++) {
        output[i] = gamma[i] * (input[i] - mean) / sqrt(variance + 1e-5) + beta[i];
    }
}

void feed_forward(
    const float* input,
    int size,
    const float* weights1,
    const float* biases1,
    const float* weights2,
    const float* biases2,
    float* output
) {
    float intermediate[size * 4]; // Expand by a factor of 4

    // First layer: Linear transformation + activation (ReLU)
    for (int i = 0; i < size * 4; i++) {
        intermediate[i] = biases1[i];
        for (int j = 0; j < size; j++) {
            intermediate[i] += input[j] * weights1[i * size + j];
        }
        if (intermediate[i] < 0) {
            intermediate[i] = 0; // ReLU activation
        }
    }

    // Second layer: Linear transformation
    for (int i = 0; i < size; i++) {
        output[i] = biases2[i];
        for (int j = 0; j < size * 4; j++) {
            output[i] += intermediate[j] * weights2[i * (size * 4) + j];
        }
    }
}

void transformer_block(
    const float* input,
    const float key[L][DK],
    const float value[L][DK],
    const float* weights1,
    const float* biases1,
    const float* weights2,
    const float* biases2,
    const float* gamma1,
    const float* beta1,
    const float* gamma2,
    const float* beta2,
    float* output
) {
    float normed_input[DK];
    float attention_output[DK];
    float ffn_output[DK];

    // Layer Norm before self-attention
    layer_norm(input, DK, gamma1, beta1, normed_input);

    // Self-Attention
    self_attention(normed_input, key, value, attention_output);

    // Add & Norm before Feed-Forward
    for (int i = 0; i < DK; i++) {
        normed_input[i] = input[i] + attention_output[i]; // Residual connection
    }
    layer_norm(normed_input, DK, gamma2, beta2, normed_input);

    // Feed-Forward Network
    feed_forward(normed_input, DK, weights1, biases1, weights2, biases2, ffn_output);

    // Add final residual connection
    for (int i = 0; i < DK; i++) {
        output[i] = normed_input[i] + ffn_output[i];
    }
}
