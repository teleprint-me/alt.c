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
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <uuid/uuid.h>

// stb
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h> // For dataset management

// alt
#include "activation.h" // For layer activations
#include "data_types.h" // Math, constants, data types, etc.
#include "logger.h"
#include "magic.h" // Alt model file format
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

MLP* mlp_create(uint32_t num_layers, uint32_t* layer_sizes);
void mlp_free(MLP* model);
void mlp_forward(MLP* model, float* input);
void mlp_backward(MLP* model, float* input, float* target);
void mlp_train(MLP* model, MNISTDataset* dataset, uint32_t epochs, float error_threshold);
MagicState mlp_save(MLP* model, const char* filepath);
MagicState mlp_load(MLP* model, const char* filepath);

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
        LOG_ERROR("%s: Failed to allocate MNISTDataset.\n", __func__);
        return NULL;
    }

    dataset->samples = malloc(sizeof(MNISTSample) * max_samples);
    if (!dataset->samples) {
        LOG_ERROR("%s: Failed to allocate MNIST samples.\n", __func__);
        free(dataset);
        return NULL;
    }

    dataset->length = max_samples;
    for (uint32_t i = 0; i < max_samples; i++) {
        dataset->samples[i].pixels = malloc(sizeof(float) * IMAGE_SIZE);
        dataset->samples[i].label = -1;

        if (!dataset->samples[i].pixels) {
            LOG_ERROR("%s: Failed to allocate MNIST sample pixels.\n", __func__);
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
        LOG_ERROR("%s: Invalid MNIST dataset.\n", __func__);
        return 0;
    }

    PathEntry* entry = path_create_entry(path, 0, 1);
    if (!entry) {
        LOG_ERROR("%s: Failed to traverse path '%s'.\n", path, __func__);
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
            LOG_ERROR("%s: Failed to load image '%s'.\n", __func__, info->path);
            continue;
        }

        // Ensure image dimensions match MNIST (28x28)
        if (width != 28 || height != 28) {
            LOG_ERROR("%s: Invalid dimensions for '%s'.\n", __func__, info->path);
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

MLP* mlp_create(uint32_t num_layers, uint32_t* layer_sizes) {
    if (num_layers < 2) {
        LOG_ERROR("%s: An MLP must have at least two layers (input and output).\n", __func__);
        return NULL;
    }

    // Allocate memory for the MLP structure
    MLP* model = malloc(sizeof(MLP));
    if (!model) {
        LOG_ERROR("%s: Failed to allocate memory for MLP.\n", __func__);
        return NULL;
    }

    // Define the number of layers
    model->num_layers = num_layers - 1; // Number of connections (layers - 1)
    model->layers = malloc(sizeof(Layer) * model->num_layers);
    if (!model->layers) {
        LOG_ERROR("%s: Failed to allocate memory for %u layers.\n", __func__, num_layers);
        free(model);
        return NULL;
    }

    // Initialize each layer
    for (uint32_t i = 0; i < model->num_layers; i++) {
        Layer* layer = &model->layers[i];
        layer->input_size = layer_sizes[i];
        layer->output_size = layer_sizes[i + 1];
        layer->weights = malloc(sizeof(float) * layer->input_size * layer->output_size);
        layer->biases = malloc(sizeof(float) * layer->output_size);
        layer->activations = malloc(sizeof(float) * layer->output_size);
        layer->gradients = malloc(sizeof(float) * layer->output_size);

        if (!layer->weights || !layer->biases || !layer->activations || !layer->gradients) {
            LOG_ERROR("%s: Failed to allocate memory for layer %d.\n", __func__, i);
            mlp_free(model); // Ensure memory cleanup
            return NULL;
        }

        // Initialize weights and biases
        for (uint32_t j = 0; j < layer->input_size * layer->output_size; j++) {
            layer->weights[j] = random_xavier_glorot(layer->input_size, layer->output_size);
        }
        for (uint32_t j = 0; j < layer->output_size; j++) {
            layer->biases[j] = random_linear();
        }
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
                .targets = NULL, // No targets in forward pass
                .weights = layer->weights,
                .biases = layer->biases,
                .outputs = layer->activations,
                .rows = layer->output_size,
                .cols = layer->input_size,
                .thread_id = t,
                .thread_count = NUM_THREADS,
                .learning_rate = 0.0f // Not used in forward pass
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
        float error
            = (bargs->targets) ? bargs->targets[i] - bargs->outputs[i] : 0.0f; // Output layer error

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

// MLP model file operations

// General section

MagicState save_general_section(MagicFile* magic_file, const char* model_name, const char* author) {
    // General configuration
    const int32_t data_type = TYPE_FLOAT32;
    const int32_t data_type_size = sizeof(data_type);

    // General model name
    const int32_t model_name_len = strlen(model_name) + 1;

    // General author name
    const int32_t author_len = strlen(author) + 1;

    // General UUID
    uuid_t binuuid;
    uuid_generate_random(binuuid);
#define UUID_STR_LEN 37 // 36 characters + 1 null character
    int32_t uuid_len = UUID_STR_LEN;
    char* uuid = malloc(uuid_len);
    if (uuid == NULL) {
        LOG_ERROR("%s: Failed to allocate memory for UUID.\n", __func__);
        return MAGIC_ERROR;
    }
    uuid_unparse_lower(binuuid, uuid);

    // Calculate General Section size
    uint64_t general_size = data_type_size + 
                            sizeof(model_name_len) + model_name_len +
                            sizeof(author_len) + author_len +
                            sizeof(uuid_len) + uuid_len;

    // Write section marker
    if (magic_write_section_marker(magic_file, MAGIC_GENERAL, general_size) != MAGIC_SUCCESS) {
        LOG_ERROR("%s: Failed to write general section marker.\n", __func__);
        free(uuid);
        return MAGIC_ERROR;
    }

    // Write the data type
    if (fwrite(&data_type, data_type_size, 1, magic_file->model) != 1) {
        LOG_ERROR("%s: Failed to write data type.\n", __func__);
        free(uuid);
        return MAGIC_ERROR;
    }
    // Write the model name and length
    if (fwrite(&model_name_len, sizeof(int32_t), 1, magic_file->model) != 1
        || fwrite(model_name, model_name_len, 1, magic_file->model) != 1) {
        LOG_ERROR("%s: Failed to write model name.\n", __func__);
        free(uuid);
        return MAGIC_ERROR;
    }
    // Write the author name and length
    if (fwrite(&author_len, sizeof(int32_t), 1, magic_file->model) != 1
        || fwrite(author, author_len, 1, magic_file->model) != 1) {
        LOG_ERROR("%s: Failed to write author name.\n", __func__);
        free(uuid);
        return MAGIC_ERROR;
    }
    // Write the UUID and length
    if (fwrite(&uuid_len, sizeof(int32_t), 1, magic_file->model) != 1
        || fwrite(uuid, uuid_len, 1, magic_file->model) != 1) {
        LOG_ERROR("%s: Failed to write UUID.\n", __func__);
        free(uuid);
        return MAGIC_ERROR;
    }
    free(uuid); // Cleanup

    return MAGIC_SUCCESS;
}

MagicState load_general_section(MagicFile* magic_file, char** model_name, char** author, char** uuid) {
    // Read and validate section marker
    int64_t section_marker, section_size;
    if (magic_read_section_marker(magic_file, &section_marker, &section_size) != MAGIC_SUCCESS) {
        LOG_ERROR("%s: Failed to read general section marker.\n", __func__);
        return MAGIC_ERROR;
    }
    if (section_marker != MAGIC_GENERAL) {
        LOG_ERROR("%s: Invalid section marker for general section.\n", __func__);
        return MAGIC_INVALID_MARKER;
    }

    // Read the data type (not used in this example, but read for consistency)
    int32_t data_type;
    if (fread(&data_type, sizeof(int32_t), 1, magic_file->model) != 1) {
        LOG_ERROR("%s: Failed to read data type.\n", __func__);
        return MAGIC_ERROR;
    }

    // Read model name
    int32_t model_name_len;
    if (fread(&model_name_len, sizeof(int32_t), 1, magic_file->model) != 1) {
        LOG_ERROR("%s: Failed to read model name length.\n", __func__);
        return MAGIC_ERROR;
    }
    *model_name = malloc(model_name_len);
    if (*model_name == NULL) {
        LOG_ERROR("%s: Failed to allocate memory for model name.\n", __func__);
        return MAGIC_ERROR;
    }
    if (fread(*model_name, model_name_len, 1, magic_file->model) != 1) {
        LOG_ERROR("%s: Failed to read model name.\n", __func__);
        free(*model_name);
        return MAGIC_ERROR;
    }

    // Read author name
    int32_t author_len;
    if (fread(&author_len, sizeof(int32_t), 1, magic_file->model) != 1) {
        LOG_ERROR("%s: Failed to read author name length.\n", __func__);
        free(*model_name);
        return MAGIC_ERROR;
    }
    *author = malloc(author_len);
    if (*author == NULL) {
        LOG_ERROR("%s: Failed to allocate memory for author name.\n", __func__);
        free(*model_name);
        return MAGIC_ERROR;
    }
    if (fread(*author, author_len, 1, magic_file->model) != 1) {
        LOG_ERROR("%s: Failed to read author name.\n", __func__);
        free(*model_name);
        free(*author);
        return MAGIC_ERROR;
    }

    // Read UUID
    int32_t uuid_len;
    if (fread(&uuid_len, sizeof(int32_t), 1, magic_file->model) != 1) {
        LOG_ERROR("%s: Failed to read UUID length.\n", __func__);
        free(*model_name);
        free(*author);
        return MAGIC_ERROR;
    }
    *uuid = malloc(uuid_len);
    if (*uuid == NULL) {
        LOG_ERROR("%s: Failed to allocate memory for UUID.\n", __func__);
        free(*model_name);
        free(*author);
        return MAGIC_ERROR;
    }
    if (fread(*uuid, uuid_len, 1, magic_file->model) != 1) {
        LOG_ERROR("%s: Failed to read UUID.\n", __func__);
        free(*model_name);
        free(*author);
        free(*uuid);
        return MAGIC_ERROR;
    }

    return MAGIC_SUCCESS;
}

// Parameters section

/// @todo Input, Output, and Hidden sizes are also hyperparameters.

MagicState save_parameters_section(MagicFile* magic_file, uint32_t epochs, float learning_rate, float error_threshold) {
    // Calculate the size of the Parameters Section
    uint64_t param_size = sizeof(epochs) + sizeof(learning_rate) + sizeof(error_threshold);

    // Write section marker
    if (magic_write_section_marker(magic_file, MAGIC_PARAMETERS, param_size) != MAGIC_SUCCESS) {
        LOG_ERROR("%s: Failed to write parameters section marker.\n", __func__);
        return MAGIC_ERROR;
    }

    // Write epochs
    if (fwrite(&epochs, sizeof(uint32_t), 1, magic_file->model) != 1) {
        LOG_ERROR("%s: Failed to write epochs.\n", __func__);
        return MAGIC_ERROR;
    }

    // Write learning rate
    if (fwrite(&learning_rate, sizeof(float), 1, magic_file->model) != 1) {
        LOG_ERROR("%s: Failed to write learning rate.\n", __func__);
        return MAGIC_ERROR;
    }

    // Write error threshold
    if (fwrite(&error_threshold, sizeof(float), 1, magic_file->model) != 1) {
        LOG_ERROR("%s: Failed to write error threshold.\n", __func__);
        return MAGIC_ERROR;
    }

    return MAGIC_SUCCESS;
}

MagicState load_parameters_section(MagicFile* magic_file, uint32_t* epochs, float* learning_rate, float* error_threshold) {
    // Read and validate section marker
    int64_t section_marker, section_size;
    if (magic_read_section_marker(magic_file, &section_marker, &section_size) != MAGIC_SUCCESS) {
        LOG_ERROR("%s: Failed to read parameters section marker.\n", __func__);
        return MAGIC_ERROR;
    }
    if (section_marker != MAGIC_PARAMETERS) {
        LOG_ERROR("%s: Invalid section marker for parameters section.\n", __func__);
        return MAGIC_INVALID_MARKER;
    }

    // Read epochs
    if (fread(epochs, sizeof(uint32_t), 1, magic_file->model) != 1) {
        LOG_ERROR("%s: Failed to read epochs.\n", __func__);
        return MAGIC_ERROR;
    }

    // Read learning rate
    if (fread(learning_rate, sizeof(float), 1, magic_file->model) != 1) {
        LOG_ERROR("%s: Failed to read learning rate.\n", __func__);
        return MAGIC_ERROR;
    }

    // Read error threshold
    if (fread(error_threshold, sizeof(float), 1, magic_file->model) != 1) {
        LOG_ERROR("%s: Failed to read error threshold.\n", __func__);
        return MAGIC_ERROR;
    }

    return MAGIC_SUCCESS;
}

MagicState mlp_save(MLP* model, const char* filepath) {
    MagicFile magic_file = magic_file_create(filepath, "wb");
    if (magic_file.open(&magic_file) != MAGIC_SUCCESS) {
        LOG_ERROR("%s: Failed to open file %s for writing.\n", __func__, filepath);
        return MAGIC_ERROR;
    }

    // Write Start Marker
    if (magic_write_start_marker(&magic_file, MAGIC_VERSION, MAGIC_ALIGNMENT) != MAGIC_SUCCESS) {
        LOG_ERROR("%s: Failed to write start marker.\n", __func__);
        magic_file.close(&magic_file);
        return MAGIC_ERROR;
    }

    // General Section
    save_general_section(&magic_file, "MNIST MLP", "Austin Berrio");

    // Parameters Section
    save_parameters_section(&magic_file, EPOCHS, LEARNING_RATE, ERROR_THRESHOLD);

    // Tensors Section
    uint64_t tensors_size = sizeof(uint32_t); // For number of layers
    for (uint32_t i = 0; i < model->num_layers; i++) {
        Layer* layer = &model->layers[i];
        tensors_size += sizeof(uint32_t) * 2; // input_size, output_size
        tensors_size += sizeof(float) * (layer->input_size * layer->output_size); // weights
        tensors_size += sizeof(float) * layer->output_size; // biases
    }
    if (magic_write_section_marker(&magic_file, MAGIC_TENSORS, tensors_size) != MAGIC_SUCCESS) {
        LOG_ERROR("%s: Failed to write tensors section marker.\n", __func__);
        magic_file.close(&magic_file);
        return MAGIC_ERROR;
    }
    fwrite(&model->num_layers, sizeof(uint32_t), 1, magic_file.model);
    for (uint32_t i = 0; i < model->num_layers; i++) {
        Layer* layer = &model->layers[i];
        fwrite(&layer->input_size, sizeof(uint32_t), 1, magic_file.model);
        fwrite(&layer->output_size, sizeof(uint32_t), 1, magic_file.model);
        fwrite(
            layer->weights, sizeof(float), layer->input_size * layer->output_size, magic_file.model
        );
        fwrite(layer->biases, sizeof(float), layer->output_size, magic_file.model);
    }

    // End Marker
    if (magic_write_end_marker(&magic_file) != MAGIC_SUCCESS) {
        LOG_ERROR("%s: Failed to write end marker.\n", __func__);
        magic_file.close(&magic_file);
        return MAGIC_ERROR;
    }

    // Close the file
    magic_file.close(&magic_file);
    return MAGIC_SUCCESS;
}

int main(int argc, char* argv[]) {
    global_logger.log_level = LOG_LEVEL_DEBUG; // Set the log level

    if (argc != 2 || !argv[1]) {
        LOG_ERROR("%s: Usage: %s <path_to_mnist>\n", __func__, argv[0]);
        return EXIT_FAILURE;
    }

    char* training_path = path_join(argv[1], "training");
    if (!path_exists(training_path)) {
        LOG_ERROR("%s: Training path does not exist!\n", __func__);
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

    uint32_t layer_sizes[] = {784, 128, 10}; // Input: 784, Hidden: 128, Output: 10
    MLP* model = mlp_create(3, layer_sizes);
    if (!model) {
        LOG_ERROR("%s: Failed to create the MLP model.\n", __func__);
        mnist_dataset_free(dataset);
        path_free_string(training_path);
        return EXIT_FAILURE;
    }
    mlp_train(model, dataset, EPOCHS, ERROR_THRESHOLD);
    mlp_save(model, "models/mnist/mlp.alt");

    // Cleanup
    mlp_free(model);
    mnist_dataset_free(dataset);
    path_free_string(training_path);

    return EXIT_SUCCESS;
}
