/**
 * Copyright © 2024 Austin Berrio
 *
 * @file src/vk/device.c
 *
 * @brief Vulkan Device API for managing Vulkan instances and devices.
 *
 * @note This API handles physical and logical device management, queue setup,
 *       and other device-related operations.
 */

#include <stdio.h>
#include <stdlib.h>

#include "interface/logger.h"

#include "vk/device.h"

VkDeviceQueueCreateInfo
vulkan_create_device_queue_info(uint32_t queueFamilyIndex, uint32_t queueCount) {
    float queuePriority = 1.0f; /// @note Maybe use a parameter for this???
    VkDeviceQueueCreateInfo deviceQueueInfo = {0};
    deviceQueueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    deviceQueueInfo.queueFamilyIndex = queueFamilyIndex; // Use the discovered queue index
    deviceQueueInfo.queueCount = queueCount;
    deviceQueueInfo.pQueuePriorities = &queuePriority;
    return deviceQueueInfo;
}

VkDeviceCreateInfo vulkan_create_device_info(VkDeviceQueueCreateInfo deviceQueueInfo) {
    // Device the device info object
    VkDeviceCreateInfo deviceInfo = {0};
    deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceInfo.queueCreateInfoCount = 1;
    deviceInfo.pQueueCreateInfos = &deviceQueueInfo; // Pass the queue info array
    deviceInfo.enabledExtensionCount = 0; // Modify this if extensions are required
    deviceInfo.ppEnabledExtensionNames = NULL;
    /// @note pEnabledFeatures is required for half and quarter precision.
    /// Using single precision for simplicity for now.
    deviceInfo.pEnabledFeatures = NULL;
    return deviceInfo;
}

void vulkan_print_physical_device_properties(VkPhysicalDeviceProperties* properties) {
    LOG_INFO("%s: Device Name: %s\n", __func__, properties->deviceName);
    LOG_INFO("%s: Device Type: %d\n", __func__, properties->deviceType);
    LOG_INFO(
        "%s: API Version: %u.%u.%u\n",
        __func__,
        VK_API_VERSION_MAJOR(properties->apiVersion),
        VK_API_VERSION_MINOR(properties->apiVersion),
        VK_API_VERSION_PATCH(properties->apiVersion)
    );
}

/// @note Not sure how to decouple the logic in a sane way to get device properties.
/// This matters because it allows the user to select a device.
/// This is fine for now, but needs to be reasoned out and improved upon.
VkPhysicalDevice vulkan_create_physical_device(VkInstance instance) {
    // Get the physical device count
    uint32_t deviceCount = 0;
    VkResult result = vkEnumeratePhysicalDevices(instance, &deviceCount, NULL);
    if (VK_SUCCESS != result || 0 == deviceCount) {
        LOG_ERROR(
            "%s: Failed to enumerate physical devices or no Vulkan-supported GPU found!\n", __func__
        );
        vkDestroyInstance(instance, NULL);
        return NULL;
    }
    // Allocate memory for the device list
    VkPhysicalDevice* physicalDeviceList
        = (VkPhysicalDevice*) malloc(deviceCount * sizeof(VkPhysicalDevice));
    if (!physicalDeviceList) {
        LOG_ERROR("%s: Failed to allocate memory for physical device list!\n", __func__);
        vkDestroyInstance(instance, NULL);
        return NULL;
    }
    // Enumerate the physical device list
    result = vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDeviceList);
    if (VK_SUCCESS != result) {
        LOG_ERROR("%s: Failed to enumerate physical devices! (Error code: %d)\n", __func__, result);
        free(physicalDeviceList);
        vkDestroyInstance(instance, NULL);
        return NULL;
    }

    // Iterate through devices and select one
    VkPhysicalDeviceProperties properties = {0};
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    for (uint32_t i = 0; i < deviceCount; i++) {
        vkGetPhysicalDeviceProperties(physicalDeviceList[i], &properties);
        vulkan_print_physical_device_properties(&properties);
        // Prefer discrete GPU
        if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            physicalDevice = physicalDeviceList[i];
            break; // Stop at the first discrete GPU
        }
    }
    // Fallback to first device if no discrete GPU is found
    if (VK_NULL_HANDLE == physicalDevice) {
        LOG_WARN("%s: No discrete GPU found. Selecting first available device.\n", __func__);
        physicalDevice = physicalDeviceList[0];
    }
    free(physicalDeviceList); // cleanup allocated device list
    return physicalDevice;
}

uint32_t
vulkan_get_compute_queue_family_index(VkInstance instance, VkPhysicalDevice physicalDevice) {
    // Get the number of available queue families
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, NULL);
    if (0 == queueFamilyCount) {
        LOG_ERROR("%s: No queue families found on the physical device.\n", __func__);
        vkDestroyInstance(instance, NULL);
        return UINT32_MAX;
    }
    // Allocate memory for queueing device family properties
    VkQueueFamilyProperties* queueFamilies
        = (VkQueueFamilyProperties*) malloc(sizeof(VkQueueFamilyProperties) * queueFamilyCount);
    if (!queueFamilies) {
        LOG_ERROR(
            "%s: Failed to allocate memory for queueing device family properties.\n", __func__
        );
        vkDestroyInstance(instance, NULL);
        return UINT32_MAX;
    }
    // Queue device family properties
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies);
    // Discover the compute-capable queue
    uint32_t computeQueueFamilyIndex = UINT32_MAX; // Set to an invalid index by default
    for (uint32_t i = 0; i < queueFamilyCount; i++) {
        if (queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
            computeQueueFamilyIndex = i; // Pick the first compute-capable queue
            break;
        }
    }
    free(queueFamilies); // Clean up allocated memory
    if (computeQueueFamilyIndex == UINT32_MAX) {
        LOG_ERROR("%s: No compute-capable queue family found.\n", __func__);
        vkDestroyInstance(instance, NULL);
        return UINT32_MAX;
    }
    return computeQueueFamilyIndex;
}
