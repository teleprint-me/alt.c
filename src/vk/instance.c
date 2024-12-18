/**
 * Copyright © 2024 Austin Berrio
 *
 * @file src/vk/instance.c
 *
 * @brief Simplify the creation pipeline for the VkInstance object.
 * 
 * The general idea is to keep the initialization process as simple, modular, and dry as possible.
 *
 * Create the VkInstance object with "sane" defaults.
 *  
 * @note Apply zero-initialization strategy to maintain a "sane" default implementation.
 * @note Keep the implementation as reasonably simple for now.
 * @note VK_VERSION is deprecated and superseded by VK_API_VERSION
 * 
 * Example: Setting extensions and validation layers
 * For headless compute, you might not need any extensions or layers initially
 * Modify the following lines as needed
 *
 * const char* extensions[] = {
 *     // Add required extensions here, e.g., "VK_KHR_surface", "VK_EXT_debug_utils"
 * };
 * vulkan_set_instance_info_extensions(&instanceCreateInfo, extensions, extensionCount);
 *
 * const char* layers[] = {
 *     // Add validation layers here if needed, e.g., "VK_LAYER_KHRONOS_validation"
 * };
 * vulkan_set_instance_info_validation_layers(&instanceCreateInfo, layers, layerCount);
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "logger.h"
#include "vk/instance.h"

uint32_t vulkan_get_api_version(void) {
    uint32_t apiVersion;
    if (vkEnumerateInstanceVersion(&apiVersion) != VK_SUCCESS) {
        fprintf(stderr, "Failed to enumerate Vulkan instance version.\n");
        return 0;
    }
    return apiVersion;
}

VkApplicationInfo vulkan_create_application_info(const char* pApplicationName, const char* pEngineName) {
    VkApplicationInfo applicationInfo = {0}; // Zero-initialize all members
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO; // Structure type
    applicationInfo.pApplicationName = pApplicationName; // Application name
    applicationInfo.applicationVersion = VK_API_VERSION_1_0; // Application version
    applicationInfo.pEngineName = pEngineName; // Engine name
    applicationInfo.engineVersion = VK_API_VERSION_1_0; // Engine version
    applicationInfo.apiVersion = vulkan_get_api_version(); // API version (e.g. Vulkan 1.2)
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

void vulkan_print_application_info(const VkApplicationInfo* pApplicationInfo) {
    // Output application information
    LOG_INFO("%s: Application Name: %s\n", __func__, pApplicationInfo->pApplicationName);
    LOG_INFO("%s: Application Version: %u\n", __func__, pApplicationInfo->applicationVersion);
    LOG_INFO("%s: Engine Name: %s\n", __func__, pApplicationInfo->pEngineName);
    LOG_INFO("%s: Engine Version: %u\n", __func__, pApplicationInfo->engineVersion);
    LOG_INFO(
        "%s: API Version: %u.%u.%u\n",
        __func__,
        VK_API_VERSION_MAJOR(pApplicationInfo->apiVersion),
        VK_API_VERSION_MINOR(pApplicationInfo->apiVersion),
        VK_API_VERSION_PATCH(pApplicationInfo->apiVersion)
    );
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

VkResult vulkan_check_validation_layer_support(const char* const* layers, uint32_t layerCount) {
    if (!layers || !(*layers) || 0 == layerCount) {
        LOG_ERROR("%s: Invalid arguments (layers=%p, layerCount=%u)\n", __func__, layers, layerCount);
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    VkResult result;

    uint32_t layerPropertyCount = 0;
    result = vkEnumerateInstanceLayerProperties(&layerPropertyCount, NULL);
    if (VK_SUCCESS != result) {
        LOG_ERROR("%s: Failed to enumerate layer property count (error code: %u)\n", __func__, result);
        return result;
    }
    if (0 == layerPropertyCount) {
        LOG_ERROR("%s: No validation layers available\n", __func__);
        return VK_ERROR_LAYER_NOT_PRESENT;
    }

    VkLayerProperties* availableLayers = (VkLayerProperties*) malloc(layerPropertyCount * sizeof(VkLayerProperties));
    if (!availableLayers) {
        LOG_ERROR("%s: Memory allocation failed for layer properties\n", __func__);
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }

    result = vkEnumerateInstanceLayerProperties(&layerPropertyCount, availableLayers);
    if (VK_SUCCESS != result) {
        LOG_ERROR("%s: Failed to enumerate layer properties (error code: %u)\n", __func__, result);
        free(availableLayers);
        return result;
    }

    // Check to see if the available layers match the requested layers.
    for (uint32_t i = 0; i < layerCount; ++i) {
        bool layerFound = false;
        for (uint32_t j = 0; j < layerPropertyCount; ++j) {
            if (strcmp(layers[i], availableLayers[j].layerName) == 0) {
                layerFound = true;
                break;
            }
        }
        if (!layerFound) {
            LOG_ERROR("%s: Validation layer not found: %s\n", __func__, layers[i]);
            free(availableLayers);
            return VK_ERROR_LAYER_NOT_PRESENT;
        }
    }

    free(availableLayers);
    return VK_SUCCESS;
}

VkResult vulkan_set_instance_info_validation_layers(
    VkInstanceCreateInfo* pInstanceInfo, const char* const* layers, uint32_t layerCount
) {
    if (!pInstanceInfo) {
        LOG_ERROR("%s: pInstanceInfo is NULL\n", __func__);
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    VkResult result = vulkan_check_validation_layer_support(layers, layerCount);
    if (VK_SUCCESS != result) {
        return result;
    }

    pInstanceInfo->enabledLayerCount = layerCount;
    pInstanceInfo->ppEnabledLayerNames = layers;

    return VK_SUCCESS;
}
