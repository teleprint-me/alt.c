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

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <vulkan/vulkan.h>

VkDeviceQueueCreateInfo
vulkan_create_device_queue_info(uint32_t queueFamilyIndex, uint32_t queueCount);

VkDeviceCreateInfo vulkan_create_device_info(VkDeviceQueueCreateInfo deviceQueueInfo);

void vulkan_print_physical_device_properties(VkPhysicalDeviceProperties* properties);

/// @note Not sure how to decouple the logic in a sane way to get device properties.
/// This matters because it allows the user to select a device.
/// This is fine for now, but needs to be reasoned out and improved upon.
VkPhysicalDevice vulkan_create_physical_device(VkInstance instance);

uint32_t
vulkan_get_compute_queue_family_index(VkInstance instance, VkPhysicalDevice physicalDevice);

#ifdef __cplusplus
}
#endif

#endif // VK_DEVICE_H
