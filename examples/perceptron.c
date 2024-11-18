/**
 * @file examples/perceptron.c
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Hyperparameters structure
typedef struct Parameters {
    // float32
    float learning_rate;
    // uint32
    unsigned int n_epochs;
    unsigned int n_samples;
    unsigned int n_inputs;
    unsigned int n_outputs;
} Parameters;

// Perceptron structure
typedef struct Perceptron {
    float bias;
    float* x; // inputs
    float* w; // weights
    float* o; // outputs
    Parameters* params;
} Perceptron;

// Create Parameters
Parameters* create_parameters(
    unsigned int n_inputs,
    unsigned int n_outputs,
    unsigned int n_epochs,
    unsigned int n_samples,
    float learning_rate
) {
    Parameters* params = (Parameters*) malloc(sizeof(Parameters));
    params->n_inputs = n_inputs;
    params->n_outputs = n_outputs ? n_outputs : 1;
    params->n_epochs = n_epochs ? n_epochs : 10;
    params->n_samples = n_samples ? n_samples : 1;
    params->learning_rate = learning_rate ? learning_rate : 1e-5f; // 1 * 10-e5
    return params;
}

void free_parameters(Parameters* params) {
    if (params) {
        free(params);
    }
}

// private function for creating perceptrons
// @note avoiding tensors for now as it'll complicate the implementation. better to keep it simple.
void create_vector(float** x, unsigned int limit) {
    *x = (float*) malloc(limit * sizeof(float)); // Allocate memory
    for (unsigned int i = 0; i < limit; i++) {
        (*x)[i] = 0.0f; // Zero-initialize
    }
}

// Initialize perceptron
Perceptron* create_perceptron(Parameters* params, float bias) {
    // allocate the perceptron
    Perceptron* perceptron = (Perceptron*) malloc(sizeof(Perceptron));
    // allocate the inputs, weights, and outputs
    create_vector(&perceptron->x, params->n_inputs);
    create_vector(&perceptron->w, params->n_inputs);
    create_vector(&perceptron->o, params->n_outputs);
    perceptron->params = params; // set hyperparameters
    perceptron->bias = bias; // set the bias
    return perceptron;
}

// Free perceptron
void free_perceptron(Perceptron* p) {
    if (p) {
        if (p->x) {
            free(p->x);
        }
        if (p->w) {
            free(p->w);
        }
        if (p->o) {
            free(p->o);
        }
        free(p);
    }
}

// @note Can probably use xavier or he initialization. doesn't matter right now.
// something to think about.
void initialize_weights(Perceptron* p) {
    for (unsigned int i = 0; i < p->params->n_inputs; i++) {
        float norm = (float) rand() / (float) RAND_MAX; // normalize input
        p->w[i] = norm * 2 - 1; // set an interval of [-1, 1]
    }
}

void initialize_inputs(Perceptron* p, float* inputs) {
    // if n_inputs is 0, the loop will not execute
    for (unsigned int i = 0; i < p->params->n_inputs; i++) {
        p->x[i] = inputs[i];
    }
}

void initialize_outputs(Perceptron* p, float* outputs) {
    // if n_outputs is 0, the loop will not execute
    for (unsigned int i = 0; i < p->params->n_outputs; i++) {
        p->o[i] = outputs[i];
    }
}

float calculate_dot_product(Perceptron* p) {
    float sum = p->bias;
    for (size_t i = 0; i < p->params->n_inputs; i++) {
        sum += p->x[i] * p->w[i]; // weighted sum
    }
    return sum;
}

// 32-bit floating-point comparison
unsigned int compare_float(float a, float b, float threshold) {
    return fabsf(a - b) < (threshold ? threshold : 1e-7) ? 1 : 0; // 1 is true, 0 is false
}

// Activation function
// @note can probably use silu, relu, gelu, etc. doesn't matter right now.
// something to think about.
float binary_step_activation(float x) {
    return compare_float(x, 0.0f, 0.0f) ? 1.0f : 0.0f;
}

// Predict using the perceptron
int predict(Perceptron* p, float* inputs) {
    initialize_inputs(p, inputs);
    float sum = calculate_dot_product(p);
    return binary_step_activation(sum);
}

// Train perceptron using the perceptron learning rule
void train(Perceptron* p, float** inputs, float* outputs) {
    for (size_t epoch = 0; epoch < p->params->n_epochs; epoch++) {
        printf("Epoch %zu:\n", epoch + 1);
        for (size_t i = 0; i < p->params->n_samples; i++) {
            int output = predict(p, inputs[i]);
            int error = outputs[i] - output;

            // Update weights and bias
            for (size_t j = 0; j < p->params->n_inputs; j++) {
                p->w[j] += p->params->learning_rate * error * inputs[i][j];
            }
            p->bias += p->params->learning_rate * error;

            printf("  Sample %zu: Error = %d, New Weights = [", i, error);
            for (size_t j = 0; j < p->params->n_inputs; j++) {
                printf("%.2f", p->w[j]);
                if (j < p->params->n_inputs - 1) {
                    printf(", ");
                }
            }
            printf("], Bias = %.2f\n", p->bias);
        }
        printf("\n");
    }
}

int main() {
    // Seed once at the start of the program
    srand(time(NULL));

    // Logical AND dataset
    float** inputs = {
        {0, 0},
        {0, 1},
        {1, 0},
        {1, 1}
    };
    float* outputs = {0, 0, 0, 1}; // AND truth table

    // Create perceptron and parameters
    Parameters* params = create_parameters(
        /* n_inputs */ 2,
        /* n_outputs */ 1,
        /* n_epochs */ 10,
        /* n_samples */ 4,
        /* learning_rate */ 0.1f
    );
    Perceptron* perceptron = create_perceptron(params, /* bias */ 0.0f);

    // Initialize weights
    initialize_weights(perceptron);

    // Train perceptron
    train(perceptron, inputs, outputs);

    // Test perceptron
    printf("Testing perceptron:\n");
    for (unsigned int i = 0; i < params->n_samples; i++) {
        int prediction = predict(perceptron, inputs[i]);
        printf(
            "  Input: [%.0f, %.0f], Predicted: %d, Expected: %.0f\n",
            inputs[i][0],
            inputs[i][1],
            prediction,
            outputs[i]
        );
    }

    // Clean up
    free_perceptron(perceptron);
    free_parameters(params);

    return 0;
}
