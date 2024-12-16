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

#include <stdint.h>
#include <vulkan/vulkan.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Create a VkApplicationInfo structure with the provided application and engine names.
 *
 * @param pApplicationName Name of the application.
 * @param pEngineName Name of the engine.
 * @return VkApplicationInfo Initialized VkApplicationInfo structure.
 *
 * @note The returned structure should be used to populate VkInstanceCreateInfo.
 */
VkApplicationInfo vulkan_create_application_info(const char* pApplicationName, const char* pEngineName);

/**
 * @brief Create a VkInstanceCreateInfo structure using the provided VkApplicationInfo.
 *
 * @param pApplicationInfo Pointer to an initialized VkApplicationInfo structure.
 * @return VkInstanceCreateInfo Initialized VkInstanceCreateInfo structure.
 */
VkInstanceCreateInfo vulkan_create_instance_info(const VkApplicationInfo* pApplicationInfo);

/**
 * @brief Set extensions for an existing VkInstanceCreateInfo object.
 *
 * @param pInstanceInfo Pointer to an initialized VkInstanceCreateInfo structure.
 * @param extensions Array of extension names to enable.
 * @param extensionCount Number of extensions in the array.
 */
void vulkan_set_instance_info_extensions(
    VkInstanceCreateInfo* pInstanceInfo, const char* const* extensions, uint32_t extensionCount
);

/**
 * @brief Set validation layers for an existing VkInstanceCreateInfo object.
 *
 * @param pInstanceInfo Pointer to an initialized VkInstanceCreateInfo structure.
 * @param layers Array of validation layer names to enable.
 * @param layerCount Number of layers in the array.
 */
VkResult vulkan_set_instance_info_validation_layers(
    VkInstanceCreateInfo* pInstanceInfo, const char* const* layers, uint32_t layerCount
);


VkInstance vulkan_create_instance(const char* pApplicationName, const char* pEngineName);

/**
 * @brief Destroy a VulkanInstance and free associated memory.
 *
 * @param vkInstance Pointer to the VulkanInstance to destroy.
 */
void vulkan_destroy_instance(VkInstance instance);

#ifdef __cplusplus
}
#endif

#endif // VK_INSTANCE_H
