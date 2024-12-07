/**
 * @file examples/models/mnist.c
 *
 * @brief MNIST implementation using the Multi-layer Perceptron
 *
 * @dataset data/mnist.tar.gz
 * @paths
 *    - data/mnist/training
 *    - data/mnist/testing
 *
 * @ref https://dl.acm.org/doi/10.5555/70405.70408
 * @ref https://yann.lecun.com/exdb/mnist/
 * @ref https://github.com/myleott/mnist_png.git
 */

// libc
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

// stb
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h> // For dataset management

// alt
#include "activation.h" // For layer activations
#include "path.h" // For path management
#include "random.h" // For weight initialization

// Training parameters
#define IMAGE_SIZE 28 * 28 // Flattened size of MNIST images
#define LEARNING_RATE 0.1f // Learning rate for gradient descent
#define EPOCHS 10000 // Maximum number of training epochs
#define ERROR_THRESHOLD 0.01f // Early stopping threshold for average error
#define NUM_THREADS sysconf(_SC_NPROCESSORS_ONLN) // Number of CPU threads available at runtime

// Structures

// Struct to hold a single image and label
typedef struct {
    float* pixels;
    int label;
} MNISTSample;

typedef struct {
    MNISTSample* samples; // Array of MNIST samples
    uint32_t length; // Number of loaded samples
} MNISTDataset;

typedef struct {
    float* weights; // Flattened weight matrix
    float* biases; // Bias vector
    float* activations; // Outputs of this layer
    float* gradients; // Gradients for backpropagation
    uint32_t input_size;
    uint32_t output_size;
} Layer;

typedef struct {
    Layer* layers;
    uint32_t num_layers;
} MLP;

typedef struct {
    float* inputs; // Input vector (e.g., pixels for MNIST)
    float* targets; // Target vector (NULL for forward pass)
    float* weights; // Flattened weight matrix
    float* biases; // Bias vector
    float* outputs; // Activations or gradients vector
    uint32_t rows; // Number of rows in the matrix (neurons in the layer)
    uint32_t cols; // Number of columns in the matrix (input size to the layer)
    uint32_t thread_id; // Thread ID
    uint32_t thread_count; // Total number of threads
    float learning_rate; // Learning rate (used in backward pass)
} ModelArgs;

// Prototypes

void print_progress(char* title, float percentage, uint32_t width, char ch);

MNISTDataset* mnist_dataset_create(uint32_t max_samples);
void mnist_dataset_free(MNISTDataset* dataset);
uint32_t mnist_dataset_load(const char* path, MNISTDataset* dataset);
uint32_t mnist_dataset_shuffle(MNISTDataset* dataset);

MLP* mlp_create(int input_size, int hidden_size, int output_size);
void mlp_free(MLP* model);
void mlp_forward(MLP* model, float* input);
void mlp_backward(MLP* model, float* input, float* target);
void mlp_train(MLP* model, MNISTDataset* dataset, uint32_t epochs, float error_threshold);
void mlp_save(MLP* model, const char* filepath);
void mlp_load(MLP* model, const char* filepath);

void* parallel_forward_pass(void* args);
void* parallel_backward_pass(void* args);

// Progress utility

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

// MNIST dataset implementation

MNISTDataset* mnist_dataset_create(uint32_t max_samples) {
    MNISTDataset* dataset = malloc(sizeof(MNISTDataset));
    if (!dataset) {
        fprintf(stderr, "Failed to allocate MNISTDataset.\n");
        return NULL;
    }

    dataset->samples = malloc(sizeof(MNISTSample) * max_samples);
    if (!dataset->samples) {
        fprintf(stderr, "Failed to allocate MNIST samples.\n");
        free(dataset);
        return NULL;
    }

    dataset->length = max_samples;
    for (uint32_t i = 0; i < max_samples; i++) {
        dataset->samples[i].pixels = malloc(sizeof(float) * IMAGE_SIZE);
        dataset->samples[i].label = -1;

        if (!dataset->samples[i].pixels) {
            fprintf(stderr, "Failed to allocate MNIST sample pixels.\n");
            for (uint32_t j = 0; j < i; j++) {
                free(dataset->samples[j].pixels);
            }
            free(dataset->samples);
            free(dataset);
            return NULL;
        }
    }

    return dataset;
}

void mnist_dataset_free(MNISTDataset* dataset) {
    if (dataset) {
        if (dataset->samples) {
            for (uint32_t i = 0; i < dataset->length; i++) {
                free(dataset->samples[i].pixels);
            }
            free(dataset->samples);
        }
        free(dataset);
    }
}

uint32_t mnist_dataset_load(const char* path, MNISTDataset* dataset) {
    if (!dataset || !dataset->samples) {
        fprintf(stderr, "Invalid MNIST dataset.\n");
        return 0;
    }

    PathEntry* entry = path_create_entry(path, 0, 1);
    if (!entry) {
        fprintf(stderr, "Failed to traverse path '%s'.\n", path);
        return 0;
    }

    uint32_t sample_count = 0;
    for (uint32_t i = 0; i < entry->length && sample_count < dataset->length; i++) {
        // Update progress bar
        float progress = (float) sample_count / (float) dataset->length;
        print_progress("Loading", progress, 50, '#');

        PathInfo* info = entry->info[i];

        // Skip non-file entries
        if (info->type != FILE_TYPE_REGULAR) {
            continue;
        }

        // Parse label from parent directory name
        char* parent_dir = path_dirname(info->path);
        int label = atoi(strrchr(parent_dir, '/') + 1);
        path_free_string(parent_dir);

        // Load image
        int width, height, channels;
        unsigned char* image_data = stbi_load(info->path, &width, &height, &channels, 1);
        if (!image_data) {
            fprintf(stderr, "Failed to load image '%s'.\n", info->path);
            continue;
        }

        // Ensure image dimensions match MNIST (28x28)
        if (width != 28 || height != 28) {
            fprintf(stderr, "Invalid dimensions for '%s'.\n", info->path);
            stbi_image_free(image_data);
            continue;
        }

        // Convert image data to float and normalize to [0, 1]
        for (int j = 0; j < IMAGE_SIZE; j++) {
            dataset->samples[sample_count].pixels[j] = image_data[j] / 255.0f;
        }
        dataset->samples[sample_count].label = label;

        stbi_image_free(image_data);
        sample_count++;
    }
    printf("\n");

    path_free_entry(entry);
    return sample_count;
}

uint32_t mnist_dataset_shuffle(MNISTDataset* dataset) {
    uint32_t sample_count = 0;

    if (dataset && dataset->samples) {
        srand((unsigned int) time(NULL)); // Seed for randomness
        for (uint32_t i = 0; i < dataset->length - 1; i++) {
            float progress = (float) i / (float) dataset->length;
            print_progress("Shuffling", progress, 50, '#'); // Track progress

            uint32_t j = rand() % (dataset->length - i); // Pick a random index

            // Swap samples[i] and samples[j]
            MNISTSample sample = dataset->samples[i];
            dataset->samples[i] = dataset->samples[j];
            dataset->samples[j] = sample;

            sample_count++; // Track swaps
        }
        printf("\n");
    }

    return sample_count;
}

// MLP model implementation

MLP* mlp_create(int input_size, int hidden_size, int output_size) {
    // Allocate memory for the MLP structure
    MLP* model = malloc(sizeof(MLP));
    if (!model) {
        fprintf(stderr, "Failed to allocate memory for MLP.\n");
        return NULL;
    }

    // Define the number of layers (input -> hidden -> output)
    model->num_layers = 2; // Hidden layer + Output layer
    model->layers = malloc(sizeof(Layer) * model->num_layers);
    if (!model->layers) {
        fprintf(stderr, "Failed to allocate memory for layers.\n");
        free(model);
        return NULL;
    }

    // Initialize the input->hidden layer
    Layer* input_hidden = &model->layers[0];
    input_hidden->input_size = input_size;
    input_hidden->output_size = hidden_size;
    input_hidden->weights = malloc(sizeof(float) * input_size * hidden_size);
    input_hidden->biases = malloc(sizeof(float) * hidden_size);
    input_hidden->activations = malloc(sizeof(float) * hidden_size);
    input_hidden->gradients = malloc(sizeof(float) * hidden_size);
    if (!input_hidden->weights || !input_hidden->biases || !input_hidden->activations
        || !input_hidden->gradients) {
        fprintf(stderr, "Failed to allocate memory for input->hidden layer.\n");
        mlp_free(model);
        return NULL;
    }

    // Initialize weights and biases
    for (int i = 0; i < input_size * hidden_size; i++) {
        input_hidden->weights[i] = random_xavier_glorot(input_size, hidden_size);
    }
    for (int i = 0; i < hidden_size; i++) {
        input_hidden->biases[i] = random_linear();
    }

    // Initialize the hidden->output layer
    Layer* hidden_output = &model->layers[1];
    hidden_output->input_size = hidden_size;
    hidden_output->output_size = output_size;
    hidden_output->weights = malloc(sizeof(float) * hidden_size * output_size);
    hidden_output->biases = malloc(sizeof(float) * output_size);
    hidden_output->activations = malloc(sizeof(float) * output_size);
    hidden_output->gradients = malloc(sizeof(float) * output_size);
    if (!hidden_output->weights || !hidden_output->biases || !hidden_output->activations
        || !hidden_output->gradients) {
        fprintf(stderr, "Failed to allocate memory for hidden->output layer.\n");
        mlp_free(model);
        return NULL;
    }

    // Initialize weights and biases
    for (int i = 0; i < hidden_size * output_size; i++) {
        hidden_output->weights[i] = random_xavier_glorot(hidden_size, output_size);
    }
    for (int i = 0; i < output_size; i++) {
        hidden_output->biases[i] = random_linear();
    }

    return model;
}

void mlp_free(MLP* model) {
    if (model) {
        if (model->layers) {
            for (uint32_t i = 0; i < model->num_layers; i++) {
                Layer* layer = &model->layers[i];
                if (layer) {
                    free(layer->weights);
                    free(layer->biases);
                    free(layer->activations);
                    free(layer->gradients);
                }
            }
            free(model->layers);
        }
        free(model);
    }
}

void mlp_forward(MLP* model, float* input) {
    float* current_input = input;

    pthread_t threads[NUM_THREADS];
    ModelArgs args[NUM_THREADS];

    for (uint32_t i = 0; i < model->num_layers; i++) {
        Layer* layer = &model->layers[i];

        for (uint32_t t = 0; t < NUM_THREADS; t++) {
            args[t] = (ModelArgs){
                .inputs = current_input,
                .targets = NULL,  // No targets in forward pass
                .weights = layer->weights,
                .biases = layer->biases,
                .outputs = layer->activations,
                .rows = layer->output_size,
                .cols = layer->input_size,
                .thread_id = t,
                .thread_count = NUM_THREADS,
                .learning_rate = 0.0f  // Not used in forward pass
            };
            pthread_create(&threads[t], NULL, parallel_forward_pass, &args[t]);
        }

        for (uint32_t t = 0; t < NUM_THREADS; t++) {
            pthread_join(threads[t], NULL);
        }

        current_input = layer->activations;
    }
}

void mlp_backward(MLP* model, float* input, float* target) {
    int32_t num_layers = (int32_t) model->num_layers - 1;

    pthread_t threads[NUM_THREADS];
    ModelArgs args[NUM_THREADS];

    // Applying chain-rule requires iterating in reverse order
    for (int32_t l = num_layers; l >= 0; l--) {
        Layer* layer = &model->layers[l];
        float* prev_activations = (l == 0) ? input : model->layers[l - 1].activations;

        for (uint32_t t = 0; t < NUM_THREADS; t++) {
            args[t] = (ModelArgs){
                .inputs = prev_activations,
                .targets = (l == num_layers) ? target : NULL,
                .weights = layer->weights,
                .biases = layer->biases,
                .outputs = layer->gradients,
                .rows = layer->output_size,
                .cols = layer->input_size,
                .thread_id = t,
                .thread_count = NUM_THREADS,
                .learning_rate = LEARNING_RATE
            };
            pthread_create(&threads[t], NULL, parallel_backward_pass, &args[t]);
        }

        for (uint32_t t = 0; t < NUM_THREADS; t++) {
            pthread_join(threads[t], NULL);
        }
    }
}

void mlp_train(MLP* model, MNISTDataset* dataset, uint32_t epochs, float error_threshold) {
    for (uint32_t epoch = 0; epoch < epochs; epoch++) {
        // Shuffle the dataset at the start of each epoch
        mnist_dataset_shuffle(dataset);

        float total_error = 0.0f;

        // Iterate over each sample in the dataset
        for (uint32_t i = 0; i < dataset->length; i++) {
            MNISTSample* sample = &dataset->samples[i];

            // Perform forward pass
            mlp_forward(model, sample->pixels);

            // Compute target vector (one-hot encoding)
            float target[10] = {0};
            target[sample->label] = 1.0f;

            // Perform backward pass
            mlp_backward(model, sample->pixels, target);

            // Accumulate error (mean squared error for simplicity)
            for (uint32_t j = 0; j < 10; j++) {
                float error = target[j] - model->layers[model->num_layers - 1].activations[j];
                total_error += error * error;
            }

            // Progress tracking
            float progress = (float) i / dataset->length;
            print_progress("Training", progress, 50, '#');
        }
        printf("\n");

        // Compute average error
        total_error /= dataset->length;

        // Report epoch metrics
        printf("Epoch %u, Error: %.6f\n", epoch + 1, (double) total_error);

        // Early stopping condition
        if (total_error < error_threshold) {
            printf("Training converged at epoch %u, Error: %.6f\n", epoch + 1, (double) total_error);
            break;
        }
    }
}

// Multi-threaded operations

void* parallel_forward_pass(void* args) {
    ModelArgs* fargs = (ModelArgs*) args;
    uint32_t start = fargs->thread_id * (fargs->rows / fargs->thread_count);
    uint32_t end = (fargs->thread_id + 1) * (fargs->rows / fargs->thread_count);

    // Handle remainder rows in the last thread
    if (fargs->thread_id == fargs->thread_count - 1) {
        end = fargs->rows;
    }

    for (uint32_t i = start; i < end; i++) {
        fargs->outputs[i] = fargs->biases[i]; // Start with bias
        // Apply the dot product
        for (uint32_t j = 0; j < fargs->cols; j++) {
            fargs->outputs[i] += fargs->weights[i * fargs->cols + j] * fargs->inputs[j];
        }
        // store the activations in the hidden output layer
        fargs->outputs[i] = activate_relu(fargs->outputs[i]);
    }

    return NULL;
}

void* parallel_backward_pass(void* args) {
    ModelArgs* bargs = (ModelArgs*) args;

    // Compute start and end indices for this thread
    uint32_t start = bargs->thread_id * (bargs->rows / bargs->thread_count);
    uint32_t end = (bargs->thread_id + 1) * (bargs->rows / bargs->thread_count);

    // Handle remainder rows in the last thread
    if (bargs->thread_id == bargs->thread_count - 1) {
        end = bargs->rows;
    }

    // Backpropagate error and update weights for assigned rows
    for (uint32_t i = start; i < end; i++) {
        float* weights = bargs->weights + i * bargs->cols;
        float error = (bargs->targets) 
                        ? bargs->targets[i] - bargs->outputs[i] // Output layer error
                        : 0.0f;

        if (!bargs->targets) {
            // Hidden layer error: sum of weighted gradients from the next layer
            for (uint32_t j = 0; j < bargs->cols; j++) {
                error += weights[j] * bargs->inputs[j];
            }
        }

        // Compute gradient
        float gradient = error * activate_relu_prime(bargs->outputs[i]);

        // Update weights and biases
        for (uint32_t j = 0; j < bargs->cols; j++) {
            weights[j] += bargs->learning_rate * gradient * bargs->inputs[j];
        }
        bargs->biases[i] += bargs->learning_rate * gradient;

        // Store gradient for next layer
        bargs->outputs[i] = gradient;
    }

    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc != 2 || !argv[1]) {
        fprintf(stderr, "Usage: %s <path_to_mnist>\n", argv[0]);
        return EXIT_FAILURE;
    }

    char* training_path = path_join(argv[1], "training");
    if (!path_exists(training_path)) {
        fprintf(stderr, "Training path does not exist!\n");
        path_free_string(training_path);
        return EXIT_FAILURE;
    }

    // Training has a max of 60000 samples
    MNISTDataset* dataset = mnist_dataset_create(60000);
    if (!dataset) {
        path_free_string(training_path);
        return EXIT_FAILURE;
    }
    mnist_dataset_load(training_path, dataset);

    uint32_t input_size = 784; // MNIST images flattened
    uint32_t hidden_size = 128; // Example hidden layer size
    uint32_t output_size = 10; // 10 output classes
    MLP* model = mlp_create(input_size, hidden_size, output_size);
    mlp_train(model, dataset, EPOCHS, ERROR_THRESHOLD);

    // Cleanup
    mlp_free(model);
    mnist_dataset_free(dataset);
    path_free_string(training_path);

    return EXIT_SUCCESS;
}
