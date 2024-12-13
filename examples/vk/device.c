/**
 * Copyright © 2024 Austin Berrio
 *
 * @file examples/device.c
 *
 * @brief A simple example showcasing how to create and destroy custom vulkan instance, device, and
 * queue objects.
 */

#include "vk/device.h"
#include "vk/instance.h"

#include <stdio.h>
#include <stdlib.h>

int main(void) {
    const char* appName = "DeviceApp";
    const char* engineName = "DeviceEngine";

    vulkan_instance_t* vkInstance = vulkan_create_instance(appName, engineName);
    if (NULL == vkInstance) {
        fprintf(stderr, "Failed to create Vulkan instance.\n");
        return EXIT_FAILURE;
    }
    printf("Successfully created Vulkan instance!\n");

    vulkan_device_t* vkDevice = vulkan_create_device(vkInstance->handle);
    if (NULL == vkDevice) {
        fprintf(stderr, "Failed to create Vulkan device.\n");
        return EXIT_FAILURE;
    }
    printf("Successfully created Vulkan device!\n");

    vulkan_destroy_device(vkDevice);
    printf("Successfully destroyed Vulkan device!\n");

    vulkan_destroy_instance(vkInstance);
    printf("Successfully destroyed Vulkan instance!\n");

    return EXIT_SUCCESS;
}
