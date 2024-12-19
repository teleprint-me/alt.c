/**
 * Copyright © 2024 Austin Berrio
 *
 * @file examples/vk/device.c
 *
 * @brief A simple example showcasing how to create and destroy custom vulkan instance, device, and
 * queue objects.
 */

#include <stdio.h>
#include <stdlib.h>

#include "logger.h"
#include "vk/instance.h"
#include "vk/device.h"

// Define validation layers
#define VALIDATION_LAYER_COUNT 1 /// @warning Cannot be a variable
const char* validationLayers[VALIDATION_LAYER_COUNT] = {"VK_LAYER_KHRONOS_validation"};

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

    uint32_t queueFamilyIndex = vulkan_get_compute_queue_family_index(instance, physicalDevice);
    VkDeviceQueueCreateInfo deviceQueueInfo = vulkan_create_device_queue_info(queueFamilyIndex, 1);
    VkDeviceCreateInfo deviceInfo = vulkan_create_device_info(deviceQueueInfo);

    // Create the logical device
    VkDevice logicalDevice;
    result = vkCreateDevice(physicalDevice, &deviceInfo, NULL, &logicalDevice);
    if (VK_SUCCESS != result) {
        fprintf(stderr, "Failed to create logical device! (Error code: %d)\n", result);
        vkDestroyInstance(instance, NULL);
        return EXIT_FAILURE;
    }

    vkDestroyDevice(logicalDevice, NULL);
    printf("Successfully destroyed Vulkan device!\n");
    vkDestroyInstance(instance, NULL);
    printf("Successfully destroyed Vulkan instance!\n");

    return EXIT_SUCCESS;
}
