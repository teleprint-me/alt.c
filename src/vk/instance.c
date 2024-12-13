/**
 * Copyright © 2024 Austin Berrio
 *
 * @file src/vk/instance.c
 *
 * @brief Create the VkInstance object with "sane" defaults.
 *
 * @note Apply zero-initialization strategy to maintain a "sane" default implementation.
 */

#include "vk/instance.h"

#include <stdio.h>
#include <stdlib.h>

VkApplicationInfo
vulkan_create_application_info(const char* pApplicationName, const char* pEngineName) {
    VkApplicationInfo applicationInfo = {0}; // Zero-initialize all members
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.pApplicationName = pApplicationName;
    applicationInfo.applicationVersion = VK_API_VERSION_1_0;
    applicationInfo.pEngineName = pEngineName;
    applicationInfo.engineVersion = VK_API_VERSION_1_0;
    applicationInfo.apiVersion = VK_API_VERSION_1_2;
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
        fprintf(stderr, "vulkan_set_instance_info_extensions: pInstanceInfo is NULL\n");
        return;
    }
    pInstanceInfo->enabledExtensionCount = extensionCount;
    pInstanceInfo->ppEnabledExtensionNames = extensions;
}

void vulkan_set_instance_info_validation_layers(
    VkInstanceCreateInfo* pInstanceInfo, const char* const* layers, uint32_t layerCount
) {
    if (NULL == pInstanceInfo) {
        fprintf(stderr, "vulkan_set_instance_info_validation_layers: pInstanceInfo is NULL\n");
        return;
    }
    pInstanceInfo->enabledLayerCount = layerCount;
    pInstanceInfo->ppEnabledLayerNames = layers;
}

vulkan_instance_t* vulkan_create_instance(const char* pApplicationName, const char* pEngineName) {
    // Allocate memory with zero-initialization
    vulkan_instance_t* vkInstance = calloc(1, sizeof(vulkan_instance_t));
    if (NULL == vkInstance) {
        fprintf(stderr, "vulkan_create_instance: Failed to allocate memory for VulkanInstance\n");
        return NULL;
    }

    // Initialize applicationInfo and instanceCreateInfo
    vkInstance->applicationInfo = vulkan_create_application_info(pApplicationName, pEngineName);
    vkInstance->instanceCreateInfo = vulkan_create_instance_info(&vkInstance->applicationInfo);

    // Example: Setting extensions and validation layers
    // For headless compute, you might not need any extensions or layers initially
    // Uncomment and modify the following lines as needed

    /*
    const char* extensions[] = {
        // Add required extensions here, e.g., "VK_KHR_surface", "VK_EXT_debug_utils"
    };
    vulkan_set_instance_info_extensions(&vkInstance->instanceCreateInfo, extensions,
    sizeof(extensions)/sizeof(extensions[0]));

    const char* layers[] = {
        // Add validation layers here if needed, e.g., "VK_LAYER_KHRONOS_validation"
    };
    vulkan_set_instance_info_validation_layers(&vkInstance->instanceCreateInfo, layers,
    sizeof(layers)/sizeof(layers[0]));
    */

    // Create the Vulkan instance
    VkResult result = vkCreateInstance(
        &vkInstance->instanceCreateInfo,
        NULL, // Allocation callbacks (optional)
        &vkInstance->handle
    );

    if (VK_SUCCESS != result) {
        fprintf(
            stderr,
            "vulkan_create_instance: Failed to create Vulkan instance! (Error code: %d)\n",
            result
        );
        free(vkInstance);
        return NULL;
    }

    return vkInstance;
}

void vulkan_destroy_instance(vulkan_instance_t* vkInstance) {
    if (NULL == vkInstance) {
        fprintf(stderr, "vulkan_destroy_instance: vkInstance is NULL\n");
        return;
    }

    if (VK_NULL_HANDLE != vkInstance->handle) {
        vkDestroyInstance(vkInstance->handle, NULL); // Vulkan instance cleanup
    }

    free(vkInstance);
}
