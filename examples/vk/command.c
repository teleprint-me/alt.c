/**
 * Copyright © 2024 Austin Berrio
 *
 * @file examples/vk/command.c
 *
 * @brief A simple example showcasing how to implement command buffers and pools
 */

#include <stdio.h>
#include <stdlib.h>

#include "logger.h"
#include "random.h"
#include "vk/device.h"
#include "vk/instance.h"

// Define validation layers
#define VALIDATION_LAYER_COUNT 1 /// @warning Cannot be a variable
const char* validationLayers[VALIDATION_LAYER_COUNT] = {"VK_LAYER_KHRONOS_validation"};

VkCommandPoolCreateInfo vulkan_create_command_pool_info(uint32_t queueFamilyIndex) {
    VkCommandPoolCreateInfo commandPoolInfo = {0};
    commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolInfo.queueFamilyIndex = queueFamilyIndex;
    commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    return commandPoolInfo;
}

VkCommandBufferAllocateInfo vulkan_create_command_buffer_alloc_info(VkCommandPool commandPool, uint32_t commandBufferCount) {
    VkCommandBufferAllocateInfo commandBufferAllocInfo = {0};
    commandBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocInfo.commandPool = commandPool;
    commandBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocInfo.commandBufferCount = commandBufferCount;
    return commandBufferAllocInfo;
}

VkBufferCreateInfo vulkan_create_buffer_info(VkDeviceSize bufferSize) {
    VkBufferCreateInfo bufferInfo = {0};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = bufferSize;
    bufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    return bufferInfo;
}

VkMemoryAllocateInfo vulkan_create_memory_alloc_info(VkMemoryRequirements memReqs, uint32_t memTypeIndex) {
    VkMemoryAllocateInfo memAllocInfo = {0};
    memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memAllocInfo.allocationSize = memReqs.size;
    memAllocInfo.memoryTypeIndex = memTypeIndex;
    return memAllocInfo;
}

uint32_t vulkan_get_device_mem_type_index(VkPhysicalDevice physicalDevice, VkMemoryRequirements memReqs, VkMemoryPropertyFlags flags) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
    uint32_t memoryTypeIndex = UINT32_MAX;
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        VkMemoryPropertyFlags props = memProperties.memoryTypes[i].propertyFlags & flags;
        if ((memReqs.memoryTypeBits & (1 << i)) && (props == flags)) {
            memoryTypeIndex = i;
            break;
        }
    }
    return memoryTypeIndex;
}

int main(void) {
    // Define some dummy data to emulate a forward pass workflow.
    const uint32_t width = 3;
    const uint32_t height = 2;

    float input[width] = {0.0f};
    random_linear_init(input, width);

    float weights[width * height] = {0.0f};
    random_linear_init(weights, width * height);

    float biases[width] = {0.0f};
    random_linear_init(biases, width);

    float output[height] = {0.0f}; // Activations should match height
    random_linear_init(output, height);

    /**
     * Initialization
     *
     * @brief Before using Vulkan, an application must initialize it by loading the Vulkan commands,
     * and creating a VkInstance object.
     *
     * @ref https://docs.vulkan.org/spec/latest/chapters/initialization.html
     */

    const char* applicationName = "PoolApp";
    const char* engineName = "PoolEngine";

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

    /**
     * Devices and Queues
     *
     * @brief Once Vulkan is initialized, devices and queues are the primary objects used to
     * interact with a Vulkan implementation.
     *
     * @ref https://docs.vulkan.org/spec/latest/chapters/devsandqueues.html
     */

    /** Create a physical device */
    VkPhysicalDevice physicalDevice = vulkan_create_physical_device(instance);
    uint32_t queueFamilyIndex = vulkan_get_compute_queue_family_index(instance, physicalDevice);
    VkDeviceQueueCreateInfo deviceQueueInfo = vulkan_create_device_queue_info(queueFamilyIndex, 1);
    VkDeviceCreateInfo deviceInfo = vulkan_create_device_info(deviceQueueInfo);

    // Create the logical device
    VkDevice logicalDevice;
    result = vkCreateDevice(physicalDevice, &deviceInfo, NULL, &logicalDevice);
    if (VK_SUCCESS != result) {
        LOG_ERROR("%s: Failed to create logical device! (Error code: %d)\n", __func__, result);
        vkDestroyInstance(instance, NULL);
        return result;
    }

    // For submitting commands to execute compute pipeline.
    VkQueue computeQueue;
    vkGetDeviceQueue(logicalDevice, queueFamilyIndex, 0, &computeQueue);

    /**
     * Command Buffers
     *
     * @brief Command buffers are objects used to record commands which can be subsequently
     * submitted to a device queue for execution.
     *
     * @ref https://docs.vulkan.org/spec/latest/chapters/cmdbuffers.html
     */

    VkCommandPoolCreateInfo commandPoolInfo = vulkan_create_command_pool_info(queueFamilyIndex);    

    VkCommandPool commandPool;
    result = vkCreateCommandPool(logicalDevice, &commandPoolInfo, NULL, &commandPool);
    if (VK_SUCCESS != result) {
        fprintf(stderr, "Failed to create command pool.\n");
        return EXIT_FAILURE;
    }
    VkCommandBufferAllocateInfo commandBufferAllocInfo = vulkan_create_command_buffer_alloc_info(commandPool, 1);

    VkCommandBuffer commandBuffer;
    result = vkAllocateCommandBuffers(logicalDevice, &commandBufferAllocInfo, &commandBuffer);
    if (VK_SUCCESS != result) {
        fprintf(stderr, "Failed to allocate command buffer.\n");
        return EXIT_FAILURE;
    }

    /// @note how do I know what the buffer size should be???
    VkBufferCreateInfo bufferInfo = vulkan_create_buffer_info(128); // 128 bytes until i understand this better

    VkBuffer buffer;
    result = vkCreateBuffer(logicalDevice, &bufferInfo, NULL, &buffer);
    if (VK_SUCCESS != result) {
        fprintf(stderr, "Failed to create buffer.\n");
        return EXIT_FAILURE;
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(logicalDevice, buffer, &memRequirements);
    uint32_t memTypeIndex = vulkan_get_device_mem_type_index(physicalDevice, memRequirements, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    if (UINT32_MAX == memTypeIndex) {
        return EXIT_FAILURE;
    }
    VkMemoryAllocateInfo memAllocInfo = vulkan_create_memory_alloc_info(memRequirements, memTypeIndex);

    VkDeviceMemory bufferDeviceMemory;
    result = vkAllocateMemory(logicalDevice, &memAllocInfo, NULL, &bufferDeviceMemory);
    if (VK_SUCCESS != result) {
        fprintf(stderr, "Failed to allocate buffer memory.\n");
        return EXIT_FAILURE;
    }

    vkBindBufferMemory(logicalDevice, buffer, bufferDeviceMemory, 0);

    // Cleanup
    vkDestroyDevice(logicalDevice, NULL);
    vkDestroyInstance(instance, NULL);

    return EXIT_SUCCESS;
}
