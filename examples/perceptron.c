/**
 * @file perceptron.c
 */

#include <stdio.h>

// Activation function
int step_function(float x) {
    return x >= 0 ? 1 : 0;
}

// Perceptron structure
typedef struct Perceptron {
    float weights[2]; // For two inputs
    float bias;
    float learning_rate;
} perceptron_t;

// Initialize perceptron
void initialize_perceptron(perceptron_t* p, float learning_rate) {
    p->weights[0] = 0.0; // Initialize weights to 0
    p->weights[1] = 0.0;
    p->bias = 0.0;
    p->learning_rate = learning_rate;
}

// Predict using the perceptron
int predict(perceptron_t* p, float x1, float x2) {
    float sum = p->weights[0] * x1 + p->weights[1] * x2 + p->bias;
    return step_function(sum);
}

// Train the perceptron
void train(perceptron_t* p, float inputs[][2], int outputs[], int numSamples, int epochs) {
    for (int epoch = 0; epoch < epochs; epoch++) {
        for (int i = 0; i < numSamples; i++) {
            float x1 = inputs[i][0];
            float x2 = inputs[i][1];
            int target = outputs[i];
            int prediction = predict(p, x1, x2);

            // Update weights and bias if there is an error
            int error = target - prediction;
            p->weights[0] += p->learning_rate * error * x1;
            p->weights[1] += p->learning_rate * error * x2;
            p->bias += p->learning_rate * error;
        }
    }
}

int main() {
    // Training data for an AND gate
    float inputs[4][2] = {
        {0, 0},
        {0, 1},
        {1, 0},
        {1, 1}
    };
    int outputs[4] = {0, 0, 0, 1}; // AND gate output

    // Initialize perceptron
    perceptron_t p;
    initialize_perceptron(&p, 0.1);

    // Train the perceptron
    train(&p, inputs, outputs, 4, 10);

    // Test the perceptron
    printf("Testing the perceptron:\n");
    for (int i = 0; i < 4; i++) {
        int result = predict(&p, inputs[i][0], inputs[i][1]);
        printf("Input: %.1f, %.1f -> Output: %d\n", inputs[i][0], inputs[i][1], result);
    }

    return 0;
}
