/**
 * Copyright © 2024 Austin Berrio
 *
 * @file include/vk/device.h
 *
 * @brief Vulkan Device API for managing Vulkan devices and queues.
 *
 * @note This API handles physical and logical device management, queue setup,
 *       and other device-related operations.
 */

#ifndef VK_DEVICE_H
#define VK_DEVICE_H

#include "vk/instance.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Vulkan Device API that handles physical and logical device management.
 */

// @brief Structure representing a Vulkan device and related objects.
typedef struct VulkanDevice {
    uint32_t physicalDeviceCount; ///< Number of physical devices available
    VkPhysicalDevice physicalDevice; ///< Handle of the selected physical device
    VkPhysicalDeviceProperties
        physicalDeviceProperties; ///< Properties of the selected physical device
    VkDeviceCreateInfo deviceInfo; ///< Device creation info
    VkDevice logicalDevice; ///< Logical device handle
    VkQueue logicalQueue; ///< Handle to the Vulkan queue
    uint32_t queueFamilyIndex; ///< Index of the queue family
} vulkan_device_t;

// @brief Creates and initializes VkDeviceCreateInfo structure.
VkDeviceCreateInfo vulkan_create_device_info(
    VkDeviceQueueCreateInfo* pQueueCreateInfos, uint32_t queueCreateInfoCount
);

// @brief Enumerates the number of physical devices available in the instance.
uint32_t vulkan_enumerate_physical_device_count(VkInstance vkInstance);

// @brief Enumerates the physical devices available in the instance.
// @note The caller needs to free it using vulkan_free_device_list()
VkPhysicalDevice*
vulkan_enumerate_physical_device_list(VkInstance vkInstance, uint32_t deviceCount);

// @brief Frees the allocated physical device list.
void vulkan_free_device_list(VkPhysicalDevice* physicalDeviceList);

// @brief Retrieves properties of the selected physical device.
VkPhysicalDeviceProperties
vulkan_get_physical_device_properties(VkPhysicalDevice selectedPhysicalDevice);

// @brief Selects an appropriate physical device from the available devices.
VkPhysicalDevice
vulkan_select_physical_device(VkPhysicalDevice* physicalDeviceList, uint32_t deviceCount);

VkDevice vulkan_create_logical_device(
    VkPhysicalDevice selectedPhysicalDevice, VkDeviceCreateInfo deviceInfo
);

/**
 * @brief Vulkan Queue API that handles setup
 */

// @brief Creates and initializes VkDeviceQueueCreateInfo structure.
VkDeviceQueueCreateInfo vulkan_create_device_queue_info(uint32_t queueFamilyIndex);

// @brief Retrieves the count of queue family properties for the physical device.
uint32_t vulkan_get_queue_family_property_count(VkPhysicalDevice selectedPhysicalDevice);

// @brief Retrieves the queue family properties for the physical device.
// @note The caller needs to free it using vulkan_free_queue_family_properties()
void vulkan_get_queue_family_properties(
    VkPhysicalDevice selectedPhysicalDevice, VkQueueFamilyProperties* queueFamilyProperties
);

// @brief Finds the index of a queue family that supports compute operations.
uint32_t vulkan_get_compute_queue_family_index(
    VkQueueFamilyProperties* queueFamilyProperties, uint32_t queueFamilyPropertyCount
);

/**
 * @brief Create and destroy vulkan device types
 */

// @brief Initializes the VulkanDevice structure.
vulkan_device_t* vulkan_create_device(VkInstance vkInstance);

// @brief Cleans up and destroys the VulkanDevice structure.
void vulkan_destroy_device(vulkan_device_t* vkDevice);

#ifdef __cplusplus
}
#endif

#endif // VK_DEVICE_H
