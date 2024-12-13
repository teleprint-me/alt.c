/**
 * Copyright © 2024 Austin Berrio
 *
 * @file examples/instance.c
 */

#include "vk/instance.h"

#include <stdio.h>
#include <stdlib.h>

/**
 * @brief Simple example showcasing how to create and destroy a custom VulkanInstance object.
 */
int main(void) {
    const char* appName = "InstanceApp";
    const char* engineName = "InstanceEngine";

    vulkan_instance_t* vkInstance = vulkan_create_instance(appName, engineName);
    if (NULL == vkInstance) {
        fprintf(stderr, "Failed to create Vulkan instance.\n");
        return EXIT_FAILURE;
    }
    printf("Successfully created Vulkan instance!\n");

    vulkan_destroy_instance(vkInstance);
    printf("Successfully destroyed Vulkan instance!\n");

    return EXIT_SUCCESS;
}
