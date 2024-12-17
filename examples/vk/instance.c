/**
 * Copyright © 2024 Austin Berrio
 *
 * @file examples/vk/instance.c
 */

/// @todo Need to substitute logger or integrate into vk pipeline upon instance creation
#include "logger.h" /// @note Logger is thread safe and has a mutex lock
#include "vk/instance.h"

#include <stdio.h>
#include <stdlib.h>

// Define validation layers
#define VALIDATION_LAYER_COUNT 1 /// @warning Cannot be a variable
const char* validationLayers[VALIDATION_LAYER_COUNT] = {"VK_LAYER_KHRONOS_validation"};

/**
 * @brief Simple example showcasing how to create and destroy a custom VulkanInstance object.
 */
int main(void) {
    VkResult result;

    const char* applicationName = "InstanceApp";
    const char* engineName = "InstanceEngine";

    // Create the app and instance info objects
    VkApplicationInfo applicationInfo = vulkan_create_application_info(applicationName, engineName);
    VkInstanceCreateInfo instanceInfo = vulkan_create_instance_info(&applicationInfo);
    vulkan_print_application_info(&applicationInfo); // Output application information

    // Enable validation layers for exposing vk related issues
    result = vulkan_set_instance_info_validation_layers(&instanceInfo, validationLayers, VALIDATION_LAYER_COUNT);
    if (VK_SUCCESS != result) {
        return result;
    }

    // Create the instance object
    VkInstance instance;
    result = vkCreateInstance(&instanceInfo, NULL, &instance);
    if (VK_SUCCESS != result) {
        LOG_ERROR("%s: Failed to create Vulkan instance! (Error code: %d)\n", __func__, result);
        return result;
    }
    printf("Successfully created Vulkan instance!\n");

    // Free the instance object
    vkDestroyInstance(instance, NULL);
    printf("Successfully destroyed Vulkan instance!\n");

    return EXIT_SUCCESS;
}
