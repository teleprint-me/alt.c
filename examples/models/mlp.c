/**
 * @file examples/models/mlp.c
 *
 * @brief A Multi-layer perceptron completely from scratch in pure C.
 *
 * The goal is to train a MLP model to recognize english characters.
 *
 * - a-z
 * - A-Z
 * - 0-9
 * - Punctuation
 * - etc.
 *
 * Requirements:
 * - Hyperparameters should be configurable at runtime.
 * - The dataset can be automatically generated at runtime.
 * - The model should be as simple and straightforward as reasonably possible.
 * - Helper functions are to be utilized to ensure code is modular, readable, and maintainable.
 * - Multi-threading can be considered if single-threading is too slow.
 * - Use 32-bit floats for enhanced learning.
 * - Use one-hot encoding for probabilities.
 * - Use SDG to keep the implementation simple and straightforward.
 *
 * Keep it lightweight and simple, don't assume or anticipate anything, and reiterate as needed!
 *
 * Oh, and have fun!
 */

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

#include "interface/logger.h"
#include "interface/random.h"

// Data structures

// Can be used for anything requiring an array and array length
// e.g. an object requiring sample related information.
typedef struct Vector {
    uint32_t width; // cols
    float* data; // flat array
} Vector;

// Useful for handling the weights in the model
typedef struct Matrix {
    uint32_t width; // cols
    uint32_t height; // rows
    float* data; // flat matrix
} Matrix;

// User input model hyperparameters for runtime configuration
typedef struct Parameters {
    float error_threshold; /**< Threshold for early stopping. */
    float learning_rate; /**< Learning rate for gradient descent. */
    uint32_t n_threads; /**< Number of CPU threads to use. */
    uint32_t n_epochs; /**< Number of training epochs. */
    uint32_t n_layers; /**< Number of layers (connections). */
    uint32_t* layer_sizes; /**< Sizes of each layer (input, hidden, output). */
} Parameters;

// Model layers
typedef struct Layer {
    Matrix* weights;
    Vector* biases;
    Vector* activations;
    Vector* gradients;
} Layer;

// Useful for SDG and Epoch metrics
typedef struct ErrorTracker {
    Matrix* layer_errors; /**< Per-layer errors for the current sample. */
    Vector* epoch_errors; /**< Aggregate errors for each epoch. */
} ErrorTracker;

// Multi-layer perceptron
typedef struct MLP {
    Parameters* params;
    ErrorTracker* tracker;
    Layer* layers;
} MLP;

// POSIX forward pass
typedef struct MLPForwardArgs {
    uint32_t thread_count; /**< Total number of threads. */
    uint32_t thread_id; /**< Thread ID for this thread. */
    Vector* inputs; /**< Input vector (e.g., MNIST pixels). */
    Vector* outputs; /**< Output activations vector. */
    Vector* biases; /**< Bias vector. */
    Matrix* weights; /**< Flattened weight matrix. */
} MLPForwardArgs;

// POSIX backward pass
typedef struct MLPBackwardArgs {
    uint32_t thread_count; /**< Total number of threads. */
    uint32_t thread_id; /**< Thread ID for this thread. */
    float learning_rate; /**< Learning rate for gradient descent. */
    Vector* targets; /**< One-hot encoded target vector. */
    Vector* inputs; /**< Input vector (e.g., MNIST pixels). */
    Vector* outputs; /**< Output gradients vector. */
    Vector* biases; /**< Bias vector. */
    Matrix* weights; /**< Flattened weight matrix. */
} MLPBackwardArgs;

// Utility structures

typedef struct Dataset {
    uint32_t start;
    uint32_t end;
    uint32_t length;
    uint8_t* samples;
} Dataset;

typedef struct IntegerList {
    char* input_list; /**< Original input string */
    uint32_t* output_list; /**< Parsed list of integers */
    uint32_t count; /**< Number of elements in the list */
} IntegerList;

// Prototypes

// Utilities

void print_progress(char* title, float percentage, uint32_t width, char ch);

IntegerList* list_create_integers(const char* input_list);
void list_free_integers(IntegerList* list);

// Vector operations

Vector* vector_create(uint32_t width);
void vector_free(Vector* vector);

// Matrix operations

Matrix* matrix_create(uint32_t height, uint32_t width);
void matrix_free(Matrix* matrix);

float matrix_get_element(const Matrix* matrix, const uint32_t row, const uint32_t col);
int matrix_set_element(Matrix* matrix, uint32_t row, uint32_t col, const float value);

Matrix* matrix_transpose(Matrix* matrix);

void matrix_print_flat(Matrix* matrix);
void matrix_print_grid(Matrix* matrix);

// Dataset operations

Dataset* dataset_create(uint32_t start, uint32_t end);
void dataset_free(Dataset* dataset);

uint32_t dataset_shuffle(Dataset* dataset);
void dataset_print(Dataset* dataset);

// Implementations

// Utilities

// @ref https://stackoverflow.com/a/36315819/20035933
void print_progress(char* title, float percentage, uint32_t width, char ch) {
    char bar[width + 1];
    for (uint32_t i = 0; i < width; i++) {
        bar[i] = ch;
    }
    bar[width] = '\0'; // Null-terminate the bar for safety

    uint32_t progress = (uint32_t) (percentage * 100 + 0.5f); // Round percentage
    uint32_t left = (uint32_t) (percentage * width + 0.5f); // Round bar width
    uint32_t right = width - left;

    printf("\r%s: %3u%% [%.*s%*s]", title, progress, left, bar, right, "");
    fflush(stdout);
}

IntegerList* list_create_integers(const char* input_list) {
    if (!input_list) {
        LOG_ERROR("%s: Input list is NULL.\n", __func__);
        return NULL;
    }

    // Allocate memory for the IntegerList structure
    IntegerList* list = (IntegerList*) malloc(sizeof(IntegerList));
    if (!list) {
        LOG_ERROR("%s: Failed to allocate memory for IntegerList.\n", __func__);
        return NULL;
    }

    list->input_list = strdup(input_list); // Store the original input string
    list->count = 0;
    list->output_list = NULL;

    // Count the number of elements (commas + 1)
    for (const char* p = input_list; *p; p++) {
        if (*p == ',') {
            list->count++;
        }
    }
    list->count++; // Include the last element

    // Allocate memory for the parsed list
    list->output_list = (uint32_t*) malloc(sizeof(uint32_t) * list->count);
    if (!list->output_list) {
        LOG_ERROR("%s: Failed to allocate memory for output list.\n", __func__);
        free(list->input_list);
        free(list);
        return NULL;
    }

    // Parse the input string into integers
    const char* start = input_list;
    for (uint32_t i = 0; i < list->count; i++) {
        list->output_list[i] = (uint32_t) strtoul((char*) start, (char**) &start, 10);
        if (*start == ',') {
            start++;
        }
    }

    return list;
}

void list_free_integers(IntegerList* list) {
    if (list) {
        if (list->input_list) {
            free(list->input_list);
        }
        if (list->output_list) {
            free(list->output_list);
        }
        free(list);
    }
}

// Vector operations

// Useful for managing one-dimensional arrays
Vector* vector_create(uint32_t width) {
    Vector* vector = (Vector*) malloc(sizeof(Vector));
    if (!vector) {
        return NULL;
    }

    vector->data = (float*) malloc(sizeof(float) * width);
    if (!vector->data) {
        free(vector);
        return NULL;
    }

    for (uint32_t i = 0; i < width; i++) {
        vector->data[i] = random_linear(); // normalized values between 0 and 1
    }

    vector->width = width;

    return vector;
}

void vector_free(Vector* vector) {
    if (vector) {
        if (vector->data) {
            free(vector->data);
        }
        free(vector);
    }
}

// Matrix operations

// Simplifies operations related to weights
Matrix* matrix_create(uint32_t height, uint32_t width) {
    Matrix* matrix = (Matrix*) malloc(sizeof(Matrix));
    if (!matrix) {
        return NULL;
    }

    matrix->data = (float*) malloc(sizeof(float) * height * width);
    if (!matrix->data) {
        free(matrix);
        return NULL;
    }

    for (uint32_t i = 0; i < (height * width); i++) {
        matrix->data[i] = random_linear(); // normalized values between 0 and 1
    }

    matrix->width = width;
    matrix->height = height;

    return matrix;
}

void matrix_free(Matrix* matrix) {
    if (matrix) {
        if (matrix->data) {
            free(matrix->data);
        }
        free(matrix);
    }
}

// Utilities for getting and setting elements for matrices to minimize errors
float matrix_get_element(const Matrix* matrix, const uint32_t row, const uint32_t col) {
    if (row >= matrix->height || col >= matrix->width) {
        return NAN;
    }
    return matrix->data[row * matrix->width + col];
}

int matrix_set_element(Matrix* matrix, uint32_t row, uint32_t col, const float value) {
    if (row >= matrix->height || col >= matrix->width) {
        return 1; // error
    }
    matrix->data[row * matrix->width + col] = value;
    return 0; // success
}

// Utility for simplifying required transpose operations
Matrix* matrix_transpose(Matrix* matrix) {
    // Create a new matrix with swapped rows and cols
    Matrix* T = matrix_create(matrix->width, matrix->height);
    if (!T) {
        return NULL;
    }

    // Populate the transposed matrix
    for (uint32_t row = 0; row < matrix->height; row++) {
        for (uint32_t col = 0; col < matrix->width; col++) {
            // Element at (i, j) in original becomes (j, i) in transposed
            T->data[col * matrix->height + row] = matrix->data[row * matrix->width + col];
        }
    }

    return T;
}

// Utilities for matrix-friendly stdout operations
void matrix_print_flat(Matrix* matrix) {
    for (uint32_t i = 0; i < (matrix->height * matrix->width); i++) {
        printf("%.6f ", (double) matrix->data[i]);
    }
    printf("\n");
}

void matrix_print_grid(Matrix* matrix) {
    for (uint32_t i = 0; i < matrix->height; i++) {
        for (uint32_t j = 0; j < matrix->width; j++) {
            printf("%.6f ", (double) matrix->data[i * matrix->width + j]);
        }
        printf("\n");
    }
}

// Dataset managements

Dataset* dataset_create(uint32_t start, uint32_t end) {
    if (start >= end) {
        return NULL; // end must be greater than start
    }

    Dataset* dataset = (Dataset*) malloc(sizeof(Dataset));
    if (!dataset) {
        return NULL;
    }

    dataset->start = start;
    dataset->end = end;
    dataset->length = end - start;
    dataset->samples = (uint8_t*) malloc(sizeof(uint8_t) * dataset->length);
    if (!dataset->samples) {
        free(dataset);
        return NULL;
    }

    // Populate samples
    for (uint32_t i = 0, s = start; i < dataset->length; i++, s++) {
        dataset->samples[i] = (uint8_t) s;
    }

    return dataset;
}

void dataset_free(Dataset* dataset) {
    if (dataset) {
        if (dataset->samples) {
            free(dataset->samples);
        }
        free(dataset);
    }
}

uint32_t dataset_shuffle(Dataset* dataset) {
    uint32_t sample_count = 0; // Track swaps

    if (!dataset || !dataset->samples) {
        return 0;
    }

    for (uint32_t i = 0; i < dataset->length; i++, sample_count++) {
        float progress = (float) i / (float) (dataset->length - 1);
        print_progress("Shuffling", progress, 50, '#'); // Track progress

        uint32_t j = rand() % (dataset->length - i); // Pick a random index

        // Swap samples[i] and samples[j]
        uint8_t sample = dataset->samples[i];
        dataset->samples[i] = dataset->samples[j];
        dataset->samples[j] = sample;
    }
    printf("\n");

    return sample_count;
}

void dataset_print(Dataset* dataset) {
    for (uint32_t i = 0; i < dataset->length; i++) {
        uint8_t code = dataset->samples[i];
        printf("index=%d, code=%d, char=%c\n", i, code, code);
    }
}

Vector* one_hot_encode(uint8_t input, Dataset* dataset) {
    if ((uint32_t) input < dataset->start || (uint32_t) input >= dataset->end) {
        LOG_ERROR("%s: Input is out of bounds.\n", __func__);
        return NULL; // Out of range
    }

    uint32_t range = dataset->end - dataset->start;
    Vector* vector = vector_create(range);
    if (!vector) {
        return NULL; // Memory allocation failed
    }

    for (uint32_t i = 0; i < range; i++) {
        vector->data[i] = (i == (input - dataset->start)) ? 1.0f : 0.0f;
    }

    return vector;
}

// MLP model implementation

Parameters* mlp_create_params(
    float error_threshold,
    float learning_rate,
    uint32_t n_threads,
    uint32_t n_epochs,
    uint32_t n_layers,
    uint32_t* layer_sizes
) {
    // Allocate memory for hyperparameters
    Parameters* params = (Parameters*) malloc(sizeof(Parameters));
    if (!params) {
        LOG_ERROR("%s: Failed to allocate memory for Parameters.\n", __func__);
        return NULL;
    }

    // Allocate memory for the layer sizes
    params->layer_sizes = (uint32_t*) malloc(sizeof(uint32_t) * n_layers);
    if (!params->layer_sizes) {
        LOG_ERROR("%s: Failed to allocate memory for layer sizes.\n", __func__);
        free(params);
        return NULL;
    }

    // Copy the layer sizes to parameters
    for (uint32_t i = 0; i < n_layers; i++) {
        params->layer_sizes[i] = layer_sizes[i];
    }

    // Set hyperparameter values
    params->error_threshold = error_threshold;
    params->learning_rate = learning_rate;
    params->n_threads = n_threads;
    params->n_epochs = n_epochs;
    params->n_layers = n_layers;

    return params;
}

void mlp_free_params(Parameters* params) {
    if (params) {
        if (params->layer_sizes) {
            free(params->layer_sizes);
        }
        free(params);
    }
}

/// @note Handle model saving loading last because the implementation is complicated and verbose.
void print_usage(const char* program_name) {
    fprintf(stderr, "Usage: %s <char> [options]\n", program_name);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "\t--range <list> Range of learned characters (default: 32,127)\n");
    fprintf(stderr, "\t--layer-sizes <list> Number of neurons in each layer (default: 1,42,95)\n");
    fprintf(stderr, "\t--threads <int> Number of CPU threads (default: auto)\n");
    fprintf(stderr, "\t--seed <int> Early stopping threshold (default: current time)\n");
    fprintf(stderr, "\t--epochs <int> Number of epochs to train (default: 1)\n");
    fprintf(stderr, "\t--learning-rate <float> Learning rate (default: 0.1)\n");
    fprintf(stderr, "\t--error-threshold <float> Early stopping threshold (default: 0.01)\n");
    fprintf(
        stderr, "\t--model <path> Path to save/load the model (default: models/char/model.alt)\n"
    );
}

int main(int argc, char* argv[]) {
    global_logger.log_level = LOG_LEVEL_DEBUG;

    if (argc < 2 || !argv[1]) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    // Default seed for rng
    uint32_t seed = (uint32_t) time(NULL);

    // Default range for learned characters
    uint32_t char_start = 32;
    uint32_t char_end = 127;

    /// @todo Default model file path
    char* model_file_path = "models/char/model.alt";

    // Create default hyperparameters instance for mlp configuration
    Parameters* params = mlp_create_params(
        /* error_threshold */ 0.05f,
        /* learning_rate */ 0.01f,
        /* n_threads */ sysconf(_SC_NPROCESSORS_ONLN), // Default available CPU thread count
        /* n_epochs */ 1, // Number of training cycles
        /* n_layers */ 3, // Number of connected layers
        /* layer_sizes */ (uint32_t[]){1, 45, 95} // input, hidden, output
    );

    // Parse CLI arguments
    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "--range") == 0 && i + 1 < argc) {
            IntegerList* range = list_create_integers(argv[++i]);
            if (!range || range->count != 2) {
                LOG_ERROR(
                    "%s: Invalid range. Expected two values separated by a comma.\n", __func__
                );
                mlp_free_params(params);
                return EXIT_FAILURE;
            }
            char_start = range->output_list[0];
            char_end = range->output_list[1];
            list_free_integers(range);
        } else if (strcmp(argv[i], "--layer-sizes") == 0 && i + 1 < argc) {
            free(params->layer_sizes); /// @warning Free pre-allocated memory
            IntegerList* list = list_create_integers(argv[++i]);
            if (!list) {
                LOG_ERROR("%s: Invalid layer sizes format.\n", __func__);
                mlp_free_params(params);
                return EXIT_FAILURE;
            }
            params->n_layers = list->count;
            params->layer_sizes = (uint32_t*) malloc(sizeof(uint32_t) * params->n_layers);
            memcpy(params->layer_sizes, list->output_list, sizeof(uint32_t) * params->n_layers);
            list_free_integers(list);
        } else if (strcmp(argv[i], "--threads") == 0 && i + 1 < argc) {
            params->n_threads = (uint32_t) atoi(argv[++i]);
        } else if (strcmp(argv[i], "--epochs") == 0 && i + 1 < argc) {
            params->n_epochs = (uint32_t) atoi(argv[++i]);
        } else if (strcmp(argv[i], "--learning-rate") == 0 && i + 1 < argc) {
            params->learning_rate = atof(argv[++i]);
        } else if (strcmp(argv[i], "--error-threshold") == 0 && i + 1 < argc) {
            params->error_threshold = atof(argv[++i]);
        } else if (strcmp(argv[i], "--seed") == 0 && i + 1 < argc) {
            seed = (uint32_t) atoi(argv[++i]);
        } else if (strcmp(argv[i], "--model") == 0 && i + 1 < argc) {
            model_file_path = argv[++i];
        } else {
            mlp_free_params(params);
            print_usage(argv[0]);
            return EXIT_FAILURE;
        }
    }

    LOG_DEBUG("%s: path=%s\n", __func__, model_file_path);
    LOG_DEBUG("%s: seed=%d\n", __func__, seed);
    random_seed(seed); // Seed the rng

    Dataset* dataset = dataset_create(char_start, char_end);
    dataset_print(dataset);
    dataset_shuffle(dataset);

    uint8_t input = (uint8_t) argv[1][0] % dataset->length;
    if (input < dataset->start) {
        input += dataset->start;
    }

    Vector* one_hot = one_hot_encode(input, dataset);
    if (!one_hot) {
        dataset_free(dataset);
        mlp_free_params(params);
        printf("Encoding failed for '%c'\n", input);
        return EXIT_FAILURE;
    }
    printf("One-hot encoding for '%c':\n", input);
    for (uint32_t i = 0; i < one_hot->width; i++) {
        printf("%.0f ", (double) one_hot->data[i]);
    }
    printf("\n");

    /// @todo training code goes here

    // cleanup
    vector_free(one_hot);
    dataset_free(dataset);
    mlp_free_params(params);

    return EXIT_SUCCESS;
}
