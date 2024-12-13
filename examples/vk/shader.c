/**
 * Copyright © 2024 Austin Berrio
 *
 * @file examples/shader.c
 *
 * @brief A simple example showcasing how to create and destroy custom
 * vulkan instance, device, and shader objects.
 */

#include "vk/device.h"
#include "vk/instance.h"
#include "vk/shader.h"

#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

void print_usage(const char* program_name) {
    fprintf(stderr, "Usage: %s [--help] path/to/shader.spv\n", program_name);
}

// Define command line option structure
static struct option long_options[] = {
    {"help", no_argument, NULL, 'h'},
    {0,      0,           0,    0  }
};

int main(int argc, char* argv[]) {
    const char* appName = "ShaderApp";
    const char* engineName = "ShaderEngine";
    char* shaderPath = NULL;

    // Parse command line options
    int option_index = 0;
    int opt;
    while ((opt = getopt_long(argc, argv, "h", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'h':
                print_usage(argv[0]);
                return EXIT_SUCCESS;
            default:
                print_usage(argv[0]);
                return EXIT_FAILURE;
        }
    }

    // Ensure a shader file path is provided
    if (optind < argc) {
        shaderPath = argv[optind]; // Get the positional argument (shader path)
    } else {
        fprintf(stderr, "Error: Missing required shader file path.\n");
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    // Create the Vulkan instance
    vulkan_instance_t* vkInstance = vulkan_create_instance(appName, engineName);
    if (NULL == vkInstance) {
        fprintf(stderr, "Failed to create Vulkan instance.\n");
        return EXIT_FAILURE;
    }
    printf("Successfully created Vulkan instance!\n");

    // Create the Vulkan device
    vulkan_device_t* vkDevice = vulkan_create_device(vkInstance->handle);
    if (NULL == vkDevice) {
        fprintf(stderr, "Failed to create Vulkan device.\n");
        return EXIT_FAILURE;
    }
    printf("Successfully created Vulkan device!\n");

    // Create the shader module
    vulkan_shader_t* vkShader = vulkan_create_shader(vkDevice->logicalDevice, shaderPath);
    if (NULL == vkShader) {
        fprintf(stderr, "Failed to create Vulkan shader.\n");
        return EXIT_FAILURE;
    }
    printf("Successfully created Vulkan shader!\n");

    // Destroy the shader module
    vulkan_destroy_shader(vkDevice->logicalDevice, vkShader);
    printf("Successfully destroyed Vulkan shader!\n");

    // Destroy the Vulkan device
    vulkan_destroy_device(vkDevice);
    printf("Successfully destroyed Vulkan device!\n");

    // Destroy the Vulkan instance
    vulkan_destroy_instance(vkInstance);
    printf("Successfully destroyed Vulkan instance!\n");

    return EXIT_SUCCESS;
}
