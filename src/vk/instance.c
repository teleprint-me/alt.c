/**
 * Copyright © 2024 Austin Berrio
 *
 * @file src/vk/instance.c
 *
 * @brief Create the VkInstance object with "sane" defaults.
 *
 * @note Apply zero-initialization strategy to maintain a "sane" default implementation.
 * @note Keep the implementation as reasonably simple for now.
 * @note VK_VERSION is deprecated and superseded by VK_API_VERSION
 * 
 * Example: Setting extensions and validation layers
 * For headless compute, you might not need any extensions or layers initially
 * Uncomment and modify the following lines as needed
 *
 * const char* extensions[] = {
 *     // Add required extensions here, e.g., "VK_KHR_surface", "VK_EXT_debug_utils"
 * };
 * vulkan_set_instance_info_extensions(&vkInstance->instanceCreateInfo, extensions,
 * sizeof(extensions)/sizeof(extensions[0]));
 *
 * const char* layers[] = {
 *     // Add validation layers here if needed, e.g., "VK_LAYER_KHRONOS_validation"
 * };
 * vulkan_set_instance_info_validation_layers(&vkInstance->instanceCreateInfo, layers,
 * sizeof(layers)/sizeof(layers[0]));
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "logger.h"
#include "vk/instance.h"

VkApplicationInfo vulkan_create_application_info(const char* pApplicationName, const char* pEngineName) {
    VkApplicationInfo applicationInfo = {0}; // Zero-initialize all members
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO; // Structure type
    applicationInfo.pApplicationName = pApplicationName; // Application name
    applicationInfo.applicationVersion = VK_API_VERSION_1_0; // Application version
    applicationInfo.pEngineName = pEngineName; // Engine name
    applicationInfo.engineVersion = VK_API_VERSION_1_0; // Engine version
    applicationInfo.apiVersion = VK_API_VERSION_1_2; // API version (Vulkan 1.2)
    return applicationInfo;
}

VkInstanceCreateInfo vulkan_create_instance_info(const VkApplicationInfo* pApplicationInfo) {
    VkInstanceCreateInfo instanceInfo = {0}; // Zero-initialize all members
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pApplicationInfo = pApplicationInfo;
    instanceInfo.enabledExtensionCount = 0;
    instanceInfo.ppEnabledExtensionNames = NULL;
    instanceInfo.enabledLayerCount = 0;
    instanceInfo.ppEnabledLayerNames = NULL;
    return instanceInfo;
}

void vulkan_set_instance_info_extensions(
    VkInstanceCreateInfo* pInstanceInfo, const char* const* extensions, uint32_t extensionCount
) {
    if (NULL == pInstanceInfo) {
        LOG_ERROR("%s: pInstanceInfo is NULL\n", __func__);
        return;
    }
    pInstanceInfo->enabledExtensionCount = extensionCount;
    pInstanceInfo->ppEnabledExtensionNames = extensions;
}

VkResult vulkan_set_instance_info_validation_layers(
    VkInstanceCreateInfo* pInstanceInfo, const char* const* layers, uint32_t layerCount
) {
    if (!pInstanceInfo) {
        LOG_ERROR("%s: pInstanceInfo is NULL\n", __func__);
        return EXIT_FAILURE;
    }

    VkResult result;

    // Get the number of validation layers
    uint32_t validationLayerCount = 0;
    result = vkEnumerateInstanceLayerProperties(&validationLayerCount, NULL);
    // add logger here

    // Allocate memory for validation layer properties
    VkLayerProperties* availableLayers = malloc(validationLayerCount * sizeof(VkLayerProperties));
    result = vkEnumerateInstanceLayerProperties(&validationLayerCount, availableLayers);
    // add logger here

    // Discover enumerated validation layers
    for (size_t i = 0; i < layerCount; i++) {
        bool found = false;
        for (uint32_t j = 0; j < validationLayerCount; j++) {
            if (strcmp(layers[i], availableLayers[j].layerName) == 0) {
                found = true;
                break;
            }
        }
        if (!found) {
            free(availableLayers);
            return EXIT_FAILURE;
        }
    }
    free(availableLayers);

    pInstanceInfo->enabledLayerCount = layerCount;
    pInstanceInfo->ppEnabledLayerNames = layers;

    return validationLayerCount;
}

VkInstance vulkan_create_instance(const char* pApplicationName, const char* pEngineName) {
    VkResult result;
    VkInstance instance;

    // Initialize applicationInfo and instanceCreateInfo
    VkApplicationInfo applicationInfo = vulkan_create_application_info(pApplicationName, pEngineName);
    VkInstanceCreateInfo instanceInfo = vulkan_create_instance_info(&applicationInfo);

    // Create simple validation layers for catching issues early
    vulkan_set_instance_info_validation_layers(
        &instanceInfo, (char[]){"VK_LAYER_KHRONOS_validation"}, 1
    );

    // Create the Vulkan instance
    VkResult result = vkCreateInstance(&instanceInfo, NULL, &instance);
    if (VK_SUCCESS != result) {
        LOG_ERROR("%s: Failed to create Vulkan instance! (Error code: %d)\n", __func__, result);
        return NULL;
    }

    return instance;
}

void vulkan_destroy_instance(VkInstance instance) {
    if (instance) {
        vkDestroyInstance(instance, NULL);
    }
}
