/**
 * Copyright © 2024 Austin Berrio
 *
 * @file include/vk/instance.h
 *
 * @brief Create the VkInstance object with "sane" defaults.
 *
 * @note Apply zero-initialization strategy to maintain a "sane" default implementation.
 */

#ifndef VK_INSTANCE_H
#define VK_INSTANCE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <vulkan/vulkan.h>

uint32_t vulkan_get_api_version(void);

VkApplicationInfo vulkan_create_application_info(const char* pApplicationName, const char* pEngineName);

VkInstanceCreateInfo vulkan_create_instance_info(const VkApplicationInfo* pApplicationInfo);

void vulkan_print_application_info(const VkApplicationInfo* pApplicationInfo);

void vulkan_set_instance_info_extensions(
    VkInstanceCreateInfo* pInstanceInfo, const char* const* extensions, uint32_t extensionCount
);

VkResult vulkan_check_validation_layer_support(const char* const* layers, uint32_t layerCount);

VkResult vulkan_set_instance_info_validation_layers(
    VkInstanceCreateInfo* pInstanceInfo, const char* const* layers, uint32_t layerCount
);

#ifdef __cplusplus
}
#endif

#endif // VK_INSTANCE_H
