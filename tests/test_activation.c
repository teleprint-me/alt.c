/**
 * @file tests/test_activation.c
 * 
 * @brief tests for activation functions
 */

#include <stdio.h>

#include "interface/activation.h"

void test_gelu(void) {
    double test_values[] = {-2.0, -1.0, 0.0, 1.0, 2.0};
    int num_tests = sizeof(test_values) / sizeof(test_values[0]);

    printf("Testing GELU Activation Function:\n");
    for (int i = 0; i < num_tests; i++) {
        double x = test_values[i];
        double exact = activate_gelu_exact(x);
        double approx = activate_gelu_approximation(x);
        printf("Input: %.2f, GELU (Exact): %.5f, GELU (Approx): %.5f\n", x, exact, approx);
    }
}

void test_activation_functions(void) {
    double test_values[] = {-2.0, -1.0, 0.0, 1.0, 2.0};
    int num_tests = sizeof(test_values) / sizeof(test_values[0]);

    printf("Testing activation functions:\n");

    for (int i = 0; i < num_tests; i++) {
        double x = test_values[i];
        printf("Input: %.2f\n", x);
        printf("  Binary Step: %.2f\n", (double) activate_binary_step(x));
        printf("  Sigmoid: %.5f\n", (double) activate_sigmoid(x));
        printf("  Tanh: %.5f\n", (double) activate_tanh(x));
        printf("  ReLU: %.2f\n", (double) activate_relu(x));
        printf("  SiLU: %.5f\n", (double) activate_silu(x));
    }
}

int main(void) {
    test_activation_functions();
    test_gelu();
    return 0;
}
