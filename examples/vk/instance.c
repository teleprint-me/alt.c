/**
 * Copyright © 2024 Austin Berrio
 *
 * @file examples/vk/instance.c
 */

#include "logger.h"
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

    VkApplicationInfo applicationInfo = vulkan_create_application_info(applicationName, engineName);

    // Output application information
    printf("Application Name: %s\n", applicationInfo.pApplicationName);
    printf("Application Version: %u\n", applicationInfo.applicationVersion);
    printf("Engine Name: %s\n", applicationInfo.pEngineName);
    printf("Engine Version: %u\n", applicationInfo.engineVersion);
    printf(
        "API Version: %u.%u.%u\n",
        VK_API_VERSION_MAJOR(applicationInfo.apiVersion),
        VK_API_VERSION_MINOR(applicationInfo.apiVersion),
        VK_API_VERSION_PATCH(applicationInfo.apiVersion)
    );

    VkInstanceCreateInfo instanceInfo = vulkan_create_instance_info(&applicationInfo);

    result = vulkan_set_instance_info_validation_layers(&instanceInfo, validationLayers, VALIDATION_LAYER_COUNT);
    if (VK_SUCCESS != result) {
        return result;
    }

    VkInstance instance;
    result = vkCreateInstance(&instanceInfo, NULL, &instance);
    if (VK_SUCCESS != result) {
        LOG_ERROR("%s: Failed to create Vulkan instance! (Error code: %d)\n", __func__, result);
        return result;
    }

    printf("Successfully created Vulkan instance!\n");

    vkDestroyInstance(instance, NULL);
    printf("Successfully destroyed Vulkan instance!\n");

    return EXIT_SUCCESS;
}
