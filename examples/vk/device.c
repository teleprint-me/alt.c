/**
 * Copyright © 2024 Austin Berrio
 *
 * @file examples/device.c
 *
 * @brief A simple example showcasing how to create and destroy custom vulkan instance, device, and
 * queue objects.
 */

/// @todo device api is a work in progress
// #include "vk/device.h"

#include "logger.h"
#include "vk/instance.h"

#include <stdio.h>
#include <stdlib.h>

// Define validation layers
#define VALIDATION_LAYER_COUNT 1 /// @warning Cannot be a variable
const char* validationLayers[VALIDATION_LAYER_COUNT] = {"VK_LAYER_KHRONOS_validation"};

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
        LOG_ERROR("%s: No discrete GPU found. Selecting first available device.\n", __func__);
        physicalDevice = physicalDeviceList[0];
    }
    free(physicalDeviceList); // cleanup allocated device list
    return physicalDevice;
}

uint32_t vulkan_get_compute_queue_family_index(VkInstance instance, VkPhysicalDevice physicalDevice) {
        // Get the number of available queue families
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, NULL);
    if (0 == queueFamilyCount) {
        LOG_ERROR("%s: No queue families found on the physical device.\n", __func__);
        vkDestroyInstance(instance, NULL);
        return EXIT_FAILURE;
    }
    // Allocate memory for queueing device family properties
    VkQueueFamilyProperties* queueFamilies = (VkQueueFamilyProperties*) malloc(sizeof(VkQueueFamilyProperties) * queueFamilyCount);
    if (!queueFamilies) {
        LOG_ERROR("%s: Failed to allocate memory for queueing device family properties.\n", __func__);
        vkDestroyInstance(instance, NULL);
        return EXIT_FAILURE;
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
        return EXIT_FAILURE;
    }
    return computeQueueFamilyIndex;
}

int main(void) {
    const char* applicationName = "DeviceApp";
    const char* engineName = "DeviceEngine";

    VkResult result;
    VkApplicationInfo applicationInfo = vulkan_create_application_info(applicationName, engineName);
    VkInstanceCreateInfo instanceInfo = vulkan_create_instance_info(&applicationInfo);
    vulkan_print_application_info(&applicationInfo); // Output application information
    result = vulkan_set_instance_info_validation_layers(
        &instanceInfo, validationLayers, VALIDATION_LAYER_COUNT
    );
    if (VK_SUCCESS != result) {
        return result;
    }

    VkInstance instance;
    result = vkCreateInstance(&instanceInfo, NULL, &instance);
    if (VK_SUCCESS != result) {
        LOG_ERROR("%s: Failed to create Vulkan instance! (Error code: %d)\n", __func__, result);
        return result;
    }
    LOG_INFO("%s: instance=%p\n", __func__, instance); /// @note masked pointer.

    /**
     * Devices
     *
     * @brief Once Vulkan is initialized, devices and queues are the primary objects used to
     * interact with a Vulkan implementation.
     *
     * @ref https://docs.vulkan.org/spec/latest/chapters/devsandqueues.html
     */

    /** Create a physical device */
    VkPhysicalDevice physicalDevice = vulkan_create_physical_device(instance);
    LOG_INFO("%s: physicalDevice=%p\n", __func__, physicalDevice); /// @note masked pointer

    /** Create a logical device */

    /// @todo This is a work in progress...
    // vulkan_destroy_device(device);
    // printf("Successfully destroyed Vulkan device!\n");

    vkDestroyInstance(instance, NULL);
    printf("Successfully destroyed Vulkan instance!\n");

    return EXIT_SUCCESS;
}
