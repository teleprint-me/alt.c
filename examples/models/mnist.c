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

// MNIST image dimensions
#define IMAGE_SIZE 28 * 28 // Flattened size of MNIST images

// Model file parameters
#define UUID_STR_LEN 37 // 36 characters + 1 null character

// Align objects
#define OBJECT_ALIGNMENT 8

// Structures

/**
 * @brief Represents a single MNIST image and its label.
 */
typedef struct __attribute__((aligned(OBJECT_ALIGNMENT))) MNISTSample {
    int label;     /**< Label representing the digit (0-9). */
    float* pixels; /**< Flattened pixel data (grayscale values). */
} MNISTSample;

/**
 * @brief Represents a dataset of MNIST samples.
 */
typedef struct __attribute__((aligned(OBJECT_ALIGNMENT))) MNISTDataset {
    uint32_t length;      /**< Number of loaded samples. */
    MNISTSample* samples; /**< Array of MNIST samples. */
} MNISTDataset;

/**
 * @brief Represents a single layer in the MLP model.
 */
typedef struct __attribute__((aligned(OBJECT_ALIGNMENT))) Layer {
    uint32_t input_size; /**< Number of inputs to this layer (width -> cols). */
    uint32_t output_size;/**< Number of outputs (neurons) in this layer (height -> rows). */
    float* weights;      /**< Flattened weight matrix. */
    float* biases;       /**< Bias vector for the layer. */
    float* activations;  /**< Activations (outputs) of this layer. */
    float* gradients;    /**< Gradients (outputs) for backpropagation. */
} Layer;

/**
 * @brief Represents hyperparameters for the MLP model.
 */
typedef struct __attribute__((aligned(OBJECT_ALIGNMENT))) Parameters {
    float error_threshold; /**< Threshold for early stopping. */
    float learning_rate;   /**< Learning rate for gradient descent. */
    uint32_t n_threads;    /**< Number of CPU threads to use. */
    uint32_t n_epochs;     /**< Number of training epochs. */
    uint32_t n_layers;     /**< Number of layers (connections). */
    uint32_t* layer_sizes; /**< Sizes of each layer (input, hidden, output). */
} Parameters;

/**
 * @brief Represents a multi-layer perceptron (MLP) model.
 */
typedef struct __attribute__((aligned(OBJECT_ALIGNMENT))) MLP {
    Parameters* params;  /**< Hyperparameters of the model. */
    Layer* layers;       /**< Array of layers in the model. */
} MLP;

/**
 * @brief Represents arguments for multi-threaded operations in the MLP model.
 */
typedef struct __attribute__((aligned(OBJECT_ALIGNMENT))) ModelArgs {
    uint32_t rows;      /**< Number of rows in the matrix (neurons in the layer). */
    uint32_t cols;      /**< Number of columns in the matrix (input size to the layer). */
    uint32_t thread_id; /**< Thread ID for this thread. */
    uint32_t thread_count; /**< Total number of threads. */
    float learning_rate; /**< Learning rate for gradient descent (backward pass). */
    float* inputs;      /**< Input vector (e.g., MNIST pixels). */
    float* targets;     /**< Target vector (NULL for forward pass). */
    float* weights;     /**< Flattened weight matrix. */
    float* biases;      /**< Bias vector. */
    float* outputs;     /**< Output activations or gradients vector. */
} ModelArgs;

// Prototypes

// Utilities
void* aligned_malloc(size_t alignment, size_t size); // uses posix_memalign()
void print_progress(char* title, float percentage, uint32_t width, char ch);

// MNIST dataset management
MNISTDataset* mnist_dataset_create(uint32_t max_samples);
void mnist_dataset_free(MNISTDataset* dataset);

// load and shuffle return the number of samples
uint32_t mnist_dataset_load(const char* path, MNISTDataset* dataset);
uint32_t mnist_dataset_shuffle(MNISTDataset* dataset);

// MLP model
Parameters* mlp_create_params(
    float error_threshold,
    float learning_rate,
    uint32_t n_threads,
    uint32_t n_epochs,
    uint32_t n_layers,
    uint32_t* layer_sizes
);
void mlp_free_params(Parameters* params);

MLP* mlp_create_model(Parameters* params);
void mlp_free_model(MLP* model);

void mlp_forward(MLP* model, float* input);
void* mlp_forward_parallel(void* args);

void mlp_backward(MLP* model, float* input, float* target);
void* mlp_backward_parallel(void* args);

void mlp_train(MLP* model, MNISTDataset* dataset);

// File management
MagicState mlp_save(MLP* model, const char* filepath);
MagicState mlp_load(MLP* model, const char* filepath);

// Memory utility (align memory)

void* aligned_malloc(size_t alignment, size_t size) {
    // Ensure alignment is at least sizeof(void*) and a power of 2
    if (alignment < sizeof(void*)) {
        alignment = sizeof(void*);
    }
    if ((alignment & (alignment - 1)) != 0) { // Check if alignment is a power of 2
        LOG_ERROR("%s: Alignment %zu is not a power of 2.\n", __func__, alignment);
        return NULL;
    }

    void* ptr = NULL;
    if (posix_memalign(&ptr, alignment, size) != 0) {
        LOG_ERROR("%s: posix_memalign failed (alignment=%zu, size=%zu).\n", __func__, alignment, size);
        return NULL;
    }

    // LOG_DEBUG("%s: Allocated memory at %p (alignment=%zu, size=%zu).\n", __func__, ptr, alignment, size);
    return ptr;
}

// Progress utility (display live progress)

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
    MNISTDataset* dataset = aligned_malloc(alignof(MNISTDataset), sizeof(MNISTDataset));
    if (!dataset) {
        LOG_ERROR("%s: Failed to allocate MNISTDataset.\n", __func__);
        return NULL;
    }

    dataset->samples = aligned_malloc(alignof(MNISTSample), sizeof(MNISTSample) * max_samples);
    if (!dataset->samples) {
        LOG_ERROR("%s: Failed to allocate MNIST samples.\n", __func__);
        free(dataset);
        return NULL;
    }

    dataset->length = max_samples;
    for (uint32_t i = 0; i < max_samples; i++) {
        dataset->samples[i].pixels = aligned_malloc(alignof(float), sizeof(float) * IMAGE_SIZE);
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

Parameters* mlp_create_params(
    float error_threshold,
    float learning_rate,
    uint32_t n_threads,
    uint32_t n_epochs,
    uint32_t n_layers,
    uint32_t* layer_sizes
) {
    // Allocate memory for hyperparameters
    Parameters* params = (Parameters*) aligned_malloc(alignof(Parameters), sizeof(Parameters));
    if (!params) {
        LOG_ERROR("%s: Failed to allocate memory for Parameters.\n", __func__);
        return NULL;
    }

    // Set hyperparameter values
    params->error_threshold = error_threshold;
    params->learning_rate = learning_rate;
    params->n_threads = n_threads;
    params->n_epochs = n_epochs;
    params->n_layers = n_layers;
    
    // Allocate memory for the layer sizes
    params->layer_sizes = (uint32_t*) aligned_malloc(alignof(uint32_t), sizeof(uint32_t) * n_layers);
    if (!params->layer_sizes) {
        LOG_ERROR("%s: Failed to allocate memory for layer sizes.\n", __func__);
        free(params);
        return NULL;
    }

    // Copy the layer sizes to parameters
    for (uint32_t i = 0; i < n_layers; i++) {
        params->layer_sizes[i] = layer_sizes[i];
    }

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

MLP* mlp_create_model(Parameters* params) {
    if (params->n_layers < 2) {
        LOG_ERROR("%s: MLP must have at least two layers (input and output).\n", __func__);
        return NULL;
    }

    MLP* model = (MLP*) aligned_malloc(alignof(MLP), sizeof(MLP));
    if (!model) {
        LOG_ERROR("%s: Failed to allocate memory for MLP.\n", __func__);
        return NULL;
    }

    model->params = params; // MLP model takes ownership of Parameters
    model->layers = (Layer*) aligned_malloc(alignof(Layer), sizeof(Layer) * params->n_layers);
    if (!model->layers) {
        LOG_ERROR("%s: Failed to allocate memory for layers.\n", __func__);
        free(model);
        return NULL;
    }

    for (uint32_t i = 0; i < params->n_layers - 1; i++) {
        Layer* layer = &model->layers[i];
        layer->input_size = params->layer_sizes[i];
        layer->output_size = params->layer_sizes[i + 1];
        layer->weights = (float*) aligned_malloc(alignof(float), sizeof(float) * layer->input_size * layer->output_size);
        layer->biases = (float*) aligned_malloc(alignof(float), sizeof(float) * layer->output_size);
        layer->activations = (float*) aligned_malloc(alignof(float), sizeof(float) * layer->output_size);
        layer->gradients = (float*) aligned_malloc(alignof(float), sizeof(float) * layer->output_size);

        if (!layer->weights || !layer->biases || !layer->activations || !layer->gradients) {
            LOG_ERROR("%s: Failed to allocate memory for layer %d.\n", __func__, i);
            mlp_free_model(model);
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

void mlp_free_model(MLP* model) {
    if (model) {
        if (model->layers) {
            for (uint32_t i = 0; i < model->params->n_layers - 1; i++) {
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
        if (model->params) {
            mlp_free_params(model->params);
        }
        free(model);
    }
}

void mlp_forward(MLP* model, float* input) {
    float* current_input = input;

    uint32_t n_threads = model->params->n_threads;
    pthread_t* threads = (pthread_t*) aligned_malloc(alignof(pthread_t), sizeof(pthread_t) * n_threads);
    ModelArgs* args = (ModelArgs*) aligned_malloc(alignof(ModelArgs), sizeof(ModelArgs) * n_threads);

    for (uint32_t l = 0; l < model->params->n_layers - 1; l++) { // -1 for connections
        Layer* layer = &model->layers[l];

        for (uint32_t t = 0; t < n_threads; t++) {
            args[t] = (ModelArgs) {
                .inputs = current_input,
                .targets = NULL, // No targets in forward pass
                .weights = layer->weights,
                .biases = layer->biases,
                .outputs = layer->activations,
                .rows = layer->output_size,
                .cols = layer->input_size,
                .thread_id = t,
                .thread_count = n_threads,
                .learning_rate = 0.0f // Not used in forward pass
            };
            pthread_create(&threads[t], NULL, mlp_forward_parallel, &args[t]);
        }

        for (uint32_t t = 0; t < n_threads; t++) {
            pthread_join(threads[t], NULL);
        }

        current_input = layer->activations;
    }

    free(threads);
    free(args);
}

void* mlp_forward_parallel(void* args) {
    ModelArgs* fargs = (ModelArgs*) args;
    uint32_t start = fargs->thread_id * (fargs->rows / fargs->thread_count);
    uint32_t end = (fargs->thread_id + 1) * (fargs->rows / fargs->thread_count);

    // Handle remainder rows in the last thread
    if (fargs->thread_id == fargs->thread_count - 1) {
        end = fargs->rows;
    }

    for (uint32_t row = start; row < end; row++) {
        fargs->outputs[row] = fargs->biases[row]; // Start with bias
        // Apply the dot product
        for (uint32_t col = 0; col < fargs->cols; col++) {
            fargs->outputs[row] += fargs->weights[row * fargs->cols + col] * fargs->inputs[col];
        }
        // store the activations in the hidden output layer
        fargs->outputs[row] = activate_relu(fargs->outputs[row]);
    }

    return NULL;
}

void mlp_backward(MLP* model, float* input, float* target) {
    int32_t n_layers = (int32_t) model->params->n_layers - 1;

    // Dynamically allocate threads and arguments
    uint32_t n_threads = model->params->n_threads - 1;
    pthread_t* threads = (pthread_t*) aligned_malloc(alignof(pthread_t), sizeof(pthread_t) * n_threads);
    ModelArgs* args = (ModelArgs*) aligned_malloc(alignof(ModelArgs), sizeof(ModelArgs) * n_threads);
    if (!threads || !args) {
        LOG_ERROR("%s: Memory allocation for threads or arguments failed.\n", __func__);
        return;
    }

    // Applying chain-rule requires iterating in reverse order
    for (int32_t l = n_layers; l-- > 0;) {
        Layer* layer = &model->layers[l];
        float* prev_activations = (l == 0) ? input : model->layers[l - 1].activations;

        for (uint32_t t = 0; t < n_threads; t++) {
            args[t] = (ModelArgs) {
                .inputs = prev_activations,
                .targets = (l == n_layers) ? target : NULL,
                .weights = layer->weights,
                .biases = layer->biases,
                .outputs = layer->gradients,
                .rows = layer->output_size,
                .cols = layer->input_size,
                .thread_id = t,
                .thread_count = n_threads,
                .learning_rate = model->params->learning_rate
            };
            pthread_create(&threads[t], NULL, mlp_backward_parallel, &args[t]);
        }

        for (uint32_t t = 0; t < n_threads; t++) {
            pthread_join(threads[t], NULL);
        }
    }

    free(threads);
    free(args);
}

void* mlp_backward_parallel(void* args) {
    ModelArgs* bargs = (ModelArgs*) args;

    // Compute start and end indices for this thread
    uint32_t start = bargs->thread_id * (bargs->rows / bargs->thread_count); // First row
    uint32_t end = (bargs->thread_id + 1) * (bargs->rows / bargs->thread_count); // Last row
    // Handle remainder rows in the last thread
    if (bargs->thread_id == bargs->thread_count - 1) {
        end = bargs->rows;
    }

    // Backpropagate error and update weights for assigned rows
    for (uint32_t row = start; row < end; row++) {
        // Inputs are naturally aligned and no transposition is needed
        float* weights = bargs->weights + row * bargs->cols; // Flat matrix manually transposed by index
        // Output layer error
        float error = 0.0f;
        if (bargs->targets) {
            // Output layer: compute error from target
            error = bargs->targets[row] - bargs->outputs[row];
        } else {
            // Hidden layer: propagate error backward
            for (uint32_t col = 0; col < bargs->cols; col++) {
                error += weights[col] * bargs->inputs[col];
            }
        }

        // Compute gradient
        float gradient = error * activate_relu_prime(bargs->outputs[row]);
        // Update weights and biases
        for (uint32_t col = 0; col < bargs->cols; col++) {
            weights[col] += bargs->learning_rate * gradient * bargs->inputs[col];
        }
        bargs->biases[row] += bargs->learning_rate * gradient;
        // Store gradient for next layer
        bargs->outputs[row] = gradient;
    }

    return NULL;
}

void mlp_train(MLP* model, MNISTDataset* dataset) {
    for (uint32_t epoch = 0; epoch < model->params->n_epochs; epoch++) {
        mnist_dataset_shuffle(dataset);
        float total_error = 0.0f;

        for (uint32_t k = 0; k < dataset->length; k++) {
            MNISTSample* sample = &dataset->samples[k];

            // Perform forward pass
            mlp_forward(model, sample->pixels);

            // Allocate aligned memory for target
            uint32_t target_len = 10;
            float* target = aligned_malloc(alignof(float), sizeof(float) * target_len);
            if (!target) {
                LOG_ERROR("%s: Aligned memory allocation for target failed.\n", __func__);
                return;
            }
            memset(target, 0, sizeof(float) * target_len);
            target[sample->label] = 1.0f; // one-hot encoding

            // Perform backward pass
            mlp_backward(model, sample->pixels, target);

            // Accumulate error
            uint32_t n_layers = model->params->n_layers - 1;
            for (int32_t l = n_layers; l-- > 0;) { /// @patch this fixes the alignment crash
                for (uint32_t col = 0; col < target_len; col++) {
                    float error = target[col] - model->layers[l].activations[col];
                    total_error += error * error; // mse for simplicity
                }
            }

            free(target); // Free target after backward pass

            // Progress tracking
            float progress = (float) k / dataset->length;
            print_progress("Training", progress, 50, '#');
        }
        printf("\n");

        // Compute average error
        total_error /= dataset->length;

        // Report epoch metrics
        printf("Epoch %u, Error: %.6f\n", epoch + 1, (double) total_error);

        // Early stopping condition
        if (total_error < model->params->error_threshold) {
            printf(
                "Training converged at epoch %u, Error: %.6f\n", epoch + 1, (double) total_error
            );
            break;
        }
    }
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
    int32_t uuid_len = UUID_STR_LEN;
    char* uuid = malloc(uuid_len);
    if (uuid == NULL) {
        LOG_ERROR("%s: Failed to allocate memory for UUID.\n", __func__);
        return MAGIC_ERROR;
    }
    uuid_unparse_lower(binuuid, uuid);

    // Calculate General Section size
    uint64_t general_size = data_type_size + sizeof(model_name_len) + model_name_len
                            + sizeof(author_len) + author_len + sizeof(uuid_len) + uuid_len;

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

MagicState save_parameters_section(MagicFile* magic_file, Parameters* params) {
    // Calculate the size of the Parameters Section
    int64_t param_size = sizeof(params->error_threshold) +
                         sizeof(params->learning_rate) +
                         sizeof(params->n_epochs) +
                         sizeof(params->n_layers) +
                         (sizeof(uint32_t) * params->n_layers); // Layer sizes array

    // Write section marker
    if (magic_write_section_marker(magic_file, MAGIC_PARAMETERS, param_size) != MAGIC_SUCCESS) {
        LOG_ERROR("%s: Failed to write parameters section marker.\n", __func__);
        return MAGIC_ERROR;
    }

    // Write error threshold
    if (fwrite(&params->error_threshold, sizeof(float), 1, magic_file->model) != 1) {
        LOG_ERROR("%s: Failed to write error threshold.\n", __func__);
        return MAGIC_ERROR;
    }

    // Write learning rate
    if (fwrite(&params->learning_rate, sizeof(float), 1, magic_file->model) != 1) {
        LOG_ERROR("%s: Failed to write learning rate.\n", __func__);
        return MAGIC_ERROR;
    }

    // Write number of epochs
    if (fwrite(&params->n_epochs, sizeof(uint32_t), 1, magic_file->model) != 1) {
        LOG_ERROR("%s: Failed to write number of epochs.\n", __func__);
        return MAGIC_ERROR;
    }

    // Write number of layers
    if (fwrite(&params->n_layers, sizeof(uint32_t), 1, magic_file->model) != 1) {
        LOG_ERROR("%s: Failed to write number of layers.\n", __func__);
        return MAGIC_ERROR;
    }

    // Write layer sizes array
    if (fwrite(params->layer_sizes, sizeof(uint32_t), params->n_layers, magic_file->model) != params->n_layers) {
        LOG_ERROR("%s: Failed to write layer sizes array.\n", __func__);
        return MAGIC_ERROR;
    }

    return MAGIC_SUCCESS;
}

MagicState load_parameters_section(MagicFile* magic_file, Parameters* params) {
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

    // Read error threshold
    if (fread(&params->error_threshold, sizeof(float), 1, magic_file->model) != 1) {
        LOG_ERROR("%s: Failed to read error threshold.\n", __func__);
        return MAGIC_ERROR;
    }

    // Read learning rate
    if (fread(&params->learning_rate, sizeof(float), 1, magic_file->model) != 1) {
        LOG_ERROR("%s: Failed to read learning rate.\n", __func__);
        return MAGIC_ERROR;
    }

    // Read epochs
    if (fread(&params->n_epochs, sizeof(uint32_t), 1, magic_file->model) != 1) {
        LOG_ERROR("%s: Failed to read epochs.\n", __func__);
        return MAGIC_ERROR;
    }

    // Read number of layers
    if (fread(&params->n_layers, sizeof(uint32_t), 1, magic_file->model) != 1) {
        LOG_ERROR("%s: Failed to read number of layers.\n", __func__);
        return MAGIC_ERROR;
    }

    // Free pre-existing allocated memory
    if (params->layer_sizes) {
        free(params->layer_sizes);
    }

    // Allocate new memory according to the number of layers
    params->layer_sizes = (uint32_t*) malloc(sizeof(uint32_t) * params->n_layers);
    if (!params->layer_sizes) {
        LOG_ERROR("%s: Failed to allocate memory for layer sizes.\n", __func__);
        return MAGIC_ERROR;
    }

    // Read layer sizes array
    if (fread(params->layer_sizes, sizeof(uint32_t), params->n_layers, magic_file->model) != params->n_layers) {
        LOG_ERROR("%s: Failed to read layer sizes array.\n", __func__);
        free(params->layer_sizes);
        return MAGIC_ERROR;
    }

    return MAGIC_SUCCESS;
}

MagicState save_tensors_section(MagicFile* magic_file, MLP* model) {
    if (model->params->n_layers < 2) {
        LOG_ERROR("Invalid number of layers: %u\n", model->params->n_layers);
        return MAGIC_ERROR;
    }

    // Calculate the size of the Tensors Section
    uint64_t tensors_size = 0;
    for (uint32_t i = 0; i < model->params->n_layers - 1; i++) {
        Layer* layer = &model->layers[i];
        tensors_size += sizeof(uint32_t) * 2; // input_size, output_size
        tensors_size += sizeof(float) * (layer->input_size * layer->output_size); // weights
        tensors_size += sizeof(float) * layer->output_size; // biases
    }

    // Write section marker
    if (magic_write_section_marker(magic_file, MAGIC_TENSORS, tensors_size) != MAGIC_SUCCESS) {
        LOG_ERROR("%s: Failed to write tensors section marker.\n", __func__);
        return MAGIC_ERROR;
    }

    // Write each layer's tensors
    for (uint32_t i = 0; i < model->params->n_layers - 1; i++) {
        Layer* layer = &model->layers[i];
        // Guard against null pointers
        if (!layer->weights || !layer->biases) {
            LOG_ERROR("Layer %d has uninitialized weights or biases\n", i);
            return MAGIC_ERROR;
        }
        // Write input and output sizes
        if (fwrite(&layer->input_size, sizeof(uint32_t), 1, magic_file->model) != 1 ||
            fwrite(&layer->output_size, sizeof(uint32_t), 1, magic_file->model) != 1) {
            LOG_ERROR("%s: Failed to write layer dimensions.\n", __func__);
            return MAGIC_ERROR;
        }
        // Write weights
        uint32_t weights_length = layer->input_size * layer->output_size;
        if (fwrite(layer->weights, sizeof(float), weights_length, magic_file->model) != weights_length) {
            LOG_ERROR("%s: Failed to write layer weights.\n", __func__);
            return MAGIC_ERROR;
        }
        // Write biases
        if (fwrite(layer->biases, sizeof(float), layer->output_size, magic_file->model) != layer->output_size) {
            LOG_ERROR("%s: Failed to write layer biases.\n", __func__);
            return MAGIC_ERROR;
        }
    }

    return MAGIC_SUCCESS;
}

MagicState load_tensors_section(MagicFile* magic_file, MLP* model) {
    // Read and validate section marker
    int64_t section_marker = 0, section_size = 0;
    if (magic_read_section_marker(magic_file, &section_marker, &section_size) != MAGIC_SUCCESS) {
        LOG_ERROR("%s: Failed to read tensors section marker.\n", __func__);
        return MAGIC_ERROR;
    }
    if (section_marker != MAGIC_TENSORS) {
        LOG_ERROR("%s: Invalid section marker for tensors section.\n", __func__);
        return MAGIC_INVALID_MARKER;
    }

    // Simplify access to parameters
    Parameters* params = model->params;

    // Populate each layer's tensors
    for (uint32_t i = 0; i < params->n_layers - 1; i++) { // -1 for connections
        Layer* layer = &model->layers[i];
        uint32_t expected_input_size = params->layer_sizes[i];
        uint32_t expected_output_size = params->layer_sizes[i + 1];

        // Read input and output sizes
        uint32_t input_size = 0, output_size = 0;
        if (fread(&input_size, sizeof(uint32_t), 1, magic_file->model) != 1 ||
            fread(&output_size, sizeof(uint32_t), 1, magic_file->model) != 1) {
            LOG_ERROR("%s: Failed to read layer dimensions.\n", __func__);
            return MAGIC_ERROR;
        }

        // Validate dimensions match expected sizes
        if (input_size != expected_input_size || output_size != expected_output_size) {
            LOG_ERROR(
                "%s: Dimension mismatch for layer %u (expected %u->%u, got %u->%u).\n",
                __func__, i, expected_input_size, expected_output_size, input_size, output_size
            );
            return MAGIC_ERROR;
        }

        // Read weights
        if (fread(
                layer->weights,
                sizeof(float),
                input_size * output_size,
                magic_file->model
            ) != input_size * output_size) {
            LOG_ERROR("%s: Failed to read layer weights.\n", __func__);
            return MAGIC_ERROR;
        }

        // Read biases
        if (fread(layer->biases, sizeof(float), output_size, magic_file->model)
            != output_size) {
            LOG_ERROR("%s: Failed to read layer biases.\n", __func__);
            return MAGIC_ERROR;
        }

        // Assign dimensions to layer
        layer->input_size = input_size;
        layer->output_size = output_size;
    }

    return MAGIC_SUCCESS;
}

MagicState mlp_save(MLP* model, const char* filepath) {
    MagicFile magic_file = magic_file_create(filepath, "wb");
    if (magic_file.open(&magic_file) != MAGIC_SUCCESS) {
        LOG_ERROR("%s: Failed to open file %s for writing.\n", __func__, filepath);
        return MAGIC_ERROR;
    }

    #define CLEANUP_AND_RETURN(state) \
        do { \
            magic_file.close(&magic_file); \
            return state; \
        } while (0)

    // Write Start Marker
    if (magic_write_start_marker(&magic_file, MAGIC_VERSION, MAGIC_ALIGNMENT) != MAGIC_SUCCESS) {
        LOG_ERROR("%s: Failed to write start marker to file %s.\n", __func__, filepath);
        CLEANUP_AND_RETURN(MAGIC_ERROR);
    }

    // General Section
    if (save_general_section(&magic_file, "MNIST MLP", "Austin Berrio") != MAGIC_SUCCESS) {
        LOG_ERROR("%s: Failed to save general section to file %s.\n", __func__, filepath);
        CLEANUP_AND_RETURN(MAGIC_ERROR);
    }

    // Parameters Section
    if (save_parameters_section(&magic_file, model->params) != MAGIC_SUCCESS) {
        LOG_ERROR("%s: Failed to save parameters section to file %s.\n", __func__, filepath);
        CLEANUP_AND_RETURN(MAGIC_ERROR);
    }

    // Tensors Section
    if (save_tensors_section(&magic_file, model) != MAGIC_SUCCESS) {
        LOG_ERROR("%s: Failed to save tensors section to file %s.\n", __func__, filepath);
        CLEANUP_AND_RETURN(MAGIC_ERROR);
    }

    // End Marker
    if (magic_write_end_marker(&magic_file) != MAGIC_SUCCESS) {
        LOG_ERROR("%s: Failed to write end marker to file %s.\n", __func__, filepath);
        CLEANUP_AND_RETURN(MAGIC_ERROR);
    }

    CLEANUP_AND_RETURN(MAGIC_SUCCESS);
}

MagicState mlp_load(MLP* model, const char* filepath) {
    MagicFile magic_file = magic_file_create(filepath, "rb");
    if (magic_file.open(&magic_file) != MAGIC_SUCCESS) {
        LOG_ERROR("%s: Failed to open file %s for reading.\n", __func__, filepath);
        return MAGIC_ERROR;
    }

    // Ensure parameters are pre-allocated
    if (!model->params) {
        LOG_ERROR("%s: Model parameters are not allocated.\n", __func__);
        magic_file.close(&magic_file);
        return MAGIC_ERROR;
    }

    // Validate version and alignment
    if (magic_file.validate(&magic_file) != MAGIC_SUCCESS) {
        LOG_ERROR("%s: Failed to validate magic file %s.\n", __func__, filepath);
        magic_file.close(&magic_file);
        return MAGIC_ERROR;
    }

    // Read Start Marker
    int32_t version, alignment;
    if (magic_read_start_marker(&magic_file, &version, &alignment) != MAGIC_SUCCESS) {
        LOG_ERROR("%s: Failed to read start marker from file %s.\n", __func__, filepath);
        magic_file.close(&magic_file);
        return MAGIC_ERROR;
    }
    LOG_INFO("%s: ALT model file format version %i\n", __func__, version);
    LOG_INFO("%s: ALT model file format alignment %i\n", __func__, alignment);

    // General Section
    char *model_name = NULL, *author = NULL, *uuid = NULL;
    if (load_general_section(&magic_file, &model_name, &author, &uuid) != MAGIC_SUCCESS) {
        LOG_ERROR("%s: Failed to load general section from file %s.\n", __func__, filepath);
        magic_file.close(&magic_file);
        return MAGIC_ERROR;
    }
    if (!model_name || !author || !uuid) {
        LOG_ERROR("%s: General section contains invalid data in file %s.\n", __func__, filepath);
        free(model_name);
        free(author);
        free(uuid);
        magic_file.close(&magic_file);
        return MAGIC_ERROR;
    }
    LOG_INFO("%s: Loaded model '%s' by %s (UUID: %s).\n", __func__, model_name, author, uuid);
    free(model_name);
    free(author);
    free(uuid);

    // Parameters Section
    if (load_parameters_section(&magic_file, model->params) != MAGIC_SUCCESS) {
        LOG_ERROR("%s: Failed to load parameters section from file %s.\n", __func__, filepath);
        magic_file.close(&magic_file);
        return MAGIC_ERROR;
    }
    LOG_INFO(
        "%s: Loaded parameters - Epochs: %u, Learning Rate: %.4f, Error Threshold: %.4f\n",
        __func__,
        model->params->n_epochs,
        (double) model->params->learning_rate,
        (double) model->params->error_threshold
    );

    // Tensors Section
    if (load_tensors_section(&magic_file, model) != MAGIC_SUCCESS) {
        LOG_ERROR("%s: Failed to load tensors section from file %s.\n", __func__, filepath);
        magic_file.close(&magic_file);
        return MAGIC_ERROR;
    }
    LOG_INFO("%s: Loaded tensors for %u layers.\n", __func__, model->params->n_layers);

    // End Marker
    if (magic_read_end_marker(&magic_file) != MAGIC_SUCCESS) {
        LOG_ERROR("%s: Failed to validate end marker in file %s.\n", __func__, filepath);
        magic_file.close(&magic_file);
        return MAGIC_ERROR;
    }

    // Close the file
    magic_file.close(&magic_file);
    return MAGIC_SUCCESS;
}

uint32_t* parse_layer_sizes(Parameters* params, const char* sizes) {
    if (!params) {
        LOG_ERROR("%s: Parameters is NULL.\n", __func__);
        return NULL;
    }

    // Free the default allocated sizes
    if (params->layer_sizes) {
        free(params->layer_sizes);
    }

    uint32_t* layer_sizes = NULL;
    uint32_t count = 0;

    // Count the number of layers (commas + 1)
    for (const char* p = sizes; *p; p++) {
        if (*p == ',') { count++; }
    }
    count++;

    // Allocate memory for layer sizes
    layer_sizes = aligned_malloc(alignof(uint32_t), sizeof(uint32_t) * count);
    if (!layer_sizes) {
        LOG_ERROR("%s: Failed to allocate memory for layer sizes.\n", __func__);
        return NULL;
    }

    // Parse the layer sizes
    const char* start = sizes;
    for (uint32_t i = 0; i < count; i++) {
        layer_sizes[i] = (uint32_t) strtol(start, (char**)&start, 10);
        if (*start == ',') { start++; }
    }

    params->n_layers = count; // Update the number of layers
    return layer_sizes;
}

void print_usage(const char* program_name) {
    fprintf(stderr, "Usage: %s <path_to_mnist> [options]\n", program_name);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "\t--layers <num> Number of layers to connect (default: 3)\n");
    fprintf(stderr, "\t--layer-sizes <num> Number of neurons in each layer (default: 784,128,10)\n");
    fprintf(stderr, "\t--threads <int> Number of CPU threads (default: auto)\n");
    fprintf(stderr, "\t--epochs <int> Number of epochs to train (default: 1)\n");
    fprintf(stderr, "\t--learning-rate <float> Learning rate (default: 0.1)\n");
    fprintf(stderr, "\t--error-threshold <float> Early stopping threshold (default: 0.01)\n");
    fprintf(stderr, "\t--model <path> Path to save/load the model (default: models/mnist/model.alt)\n");
}

int main(int argc, char* argv[]) {
    global_logger.log_level = LOG_LEVEL_DEBUG;

    if (argc < 2 || !argv[1]) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    // Default model file path
    char* model_file_path = "models/mnist/model.alt";

    // Create default hyperparameters instance for mlp configuration
    Parameters* params = mlp_create_params(
        /* error_threshold */ 0.05f,
        /* learning_rate */ 0.1f,
        /* n_threads */ sysconf(_SC_NPROCESSORS_ONLN), // Default available CPU thread count
        /* n_epochs */ 1, // Number of training cycles
        /* n_layers */ 3, // Number of connected layers
        /* layer_sizes */ (uint32_t[]){784, 128, 10} // input, hidden, output
    );

    // Parse CLI arguments
    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "--epochs") == 0 && i + 1 < argc) {
            params->n_epochs = (uint32_t)atoi(argv[++i]);
        } else if (strcmp(argv[i], "--learning-rate") == 0 && i + 1 < argc) {
            params->learning_rate = atof(argv[++i]);
        } else if (strcmp(argv[i], "--error-threshold") == 0 && i + 1 < argc) {
            params->error_threshold = atof(argv[++i]);
        } else if (strcmp(argv[i], "--layer-sizes") == 0 && i + 1 < argc) {
            // Parse layer sizes from a comma-separated string
            char* sizes = argv[++i];
            params->layer_sizes = parse_layer_sizes(params, sizes);
            if (!params->layer_sizes) {
                LOG_ERROR("%s: Failed to parse layer sizes.\n", __func__);
                return EXIT_FAILURE;
            }
        } else if (strcmp(argv[i], "--threads") == 0 && i + 1 < argc) {
            params->n_threads = (uint32_t)atoi(argv[++i]);
        } else if (strcmp(argv[i], "--model") == 0 && i + 1 < argc) {
            model_file_path = argv[++i];
        } else {
            print_usage(argv[0]);
            return EXIT_FAILURE;
        }
    }

    for (uint32_t i = 0; i < params->n_layers; i++) {
        LOG_DEBUG("%s: Layer Sizes[%d] = %d\n", __func__, i, params->layer_sizes[i]);
    }

    // Prepare paths
    char* training_path = path_join(argv[1], "training");
    if (!path_exists(training_path)) {
        LOG_ERROR("%s: Training path does not exist: %s\n", __func__, training_path);
        path_free_string(training_path);
        return EXIT_FAILURE;
    }

    // Timer start
    clock_t start_time = clock();

    // Load dataset
    MNISTDataset* dataset = mnist_dataset_create(60000);
    if (!dataset) {
        path_free_string(training_path);
        return EXIT_FAILURE;
    }

    uint32_t sample_count = mnist_dataset_load(training_path, dataset);
    if (sample_count == 0) {
        LOG_ERROR("%s: No samples loaded from the dataset.\n", __func__);
        mnist_dataset_free(dataset);
        path_free_string(training_path);
        return EXIT_FAILURE;
    }
    LOG_INFO("%s: Loaded %d samples.\n", __func__, sample_count);

    // Timer stop for loading and shuffling
    clock_t load_time = clock();
    LOG_INFO("%s: Loading and shuffling time: %.2f seconds\n",
             __func__, (double)(load_time - start_time) / CLOCKS_PER_SEC);

    // Create or load model
    MLP* model = mlp_create_model(params); // model owns params
    if (path_exists(model_file_path)) {
        LOG_INFO("%s: Loading model from %s\n", __func__, model_file_path);
        if (mlp_load(model, model_file_path) != MAGIC_SUCCESS) {
            LOG_ERROR("%s: Failed to load model from %s\n", __func__, model_file_path);
            mlp_free_model(model); // model frees params
            mnist_dataset_free(dataset);
            path_free_string(training_path);
            return EXIT_FAILURE;
        }
    } else {
        LOG_INFO("%s: Training model from scratch.\n", __func__);
        if (!model) {
            LOG_ERROR("%s: Failed to create the MLP model.\n", __func__);
            mnist_dataset_free(dataset);
            path_free_string(training_path);
            return EXIT_FAILURE;
        }
    }

    // Train the model
    mlp_train(model, dataset);

    // Timer stop for training
    clock_t train_time = clock();
    LOG_INFO("%s: Training time: %.2f seconds\n",
             __func__, (double)(train_time - load_time) / CLOCKS_PER_SEC);

    // Save the model
    char* model_base_path = path_dirname(model_file_path);
    if (!path_exists(model_base_path)) {
        mkdir(model_base_path, 0755);
        // path_create_directories(model_base_path, 0755); // @todo This function does not exist.
    }
    if (mlp_save(model, model_file_path) != MAGIC_SUCCESS) {
        LOG_ERROR("%s: Failed to save the model to %s.\n", __func__, model_file_path);
    } else {
        LOG_INFO("%s: Model saved to %s.\n", __func__, model_file_path);
    }

    // Timer stop for saving
    clock_t end_time = clock();
    LOG_INFO("%s: Total time: %.2f seconds\n",
             __func__, (double)(end_time - start_time) / CLOCKS_PER_SEC);

    // Cleanup
    mlp_free_model(model);
    mnist_dataset_free(dataset);
    path_free_string(training_path);
    path_free_string(model_base_path);

    return EXIT_SUCCESS;
}
