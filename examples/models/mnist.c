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

#define IMAGE_SIZE 28 * 28 // Flattened size of MNIST images

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

// @ref https://stackoverflow.com/a/36315819/20035933
void print_progress(float percentage, uint32_t width, char ch) {
    char bar[width + 1];
    for (uint32_t i = 0; i < width; i++) {
        bar[i] = ch;
    }
    bar[width] = '\0'; // Null-terminate the bar for safety

    uint32_t progress = (uint32_t) (percentage * 100 + 0.5f); // Round percentage
    uint32_t left = (uint32_t) (percentage * width + 0.5f); // Round bar width
    uint32_t right = width - left;

    printf("\rLoading: %3u%% [%.*s%*s]", progress, left, bar, right, "");
    fflush(stdout);
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
        print_progress(progress, 50, '#');

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

void mnist_dataset_shuffle(MNISTDataset* dataset) {
    if (dataset && dataset->samples) {
        srand((unsigned int) time(NULL)); // Seed for randomness
        for (uint32_t i = dataset->length - 1; i > 0; i--) {
            uint32_t j = rand() % (i + 1); // Pick a random index
            // Swap samples[i] and samples[j]
            MNISTSample sample = dataset->samples[i];
            dataset->samples[i] = dataset->samples[j];
            dataset->samples[j] = sample;
        }
    }
}

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
    if (!input_hidden->weights || !input_hidden->biases || 
        !input_hidden->activations || !input_hidden->gradients) {
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
    if (!hidden_output->weights || !hidden_output->biases || 
        !hidden_output->activations || !hidden_output->gradients) {
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

void mlp_forward(MLP* model, float* input);
void mlp_backward(MLP* model, float* target);

void* parallel_matrix_multiply(void* args);

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

    MNISTDataset* dataset = mnist_dataset_create(60000); // Training has a max of 60000 samples
    if (!dataset) {
        path_free_string(training_path);
        return EXIT_FAILURE;
    }

    printf("Loading MNIST training data from '%s'...\n", training_path);
    uint32_t loaded_samples = mnist_dataset_load(training_path, dataset);
    printf("Loaded %u samples.\n", loaded_samples);

    // Cleanup
    mnist_dataset_free(dataset);
    path_free_string(training_path);

    return EXIT_SUCCESS;
}
