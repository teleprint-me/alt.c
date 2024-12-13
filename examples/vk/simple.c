/**
 * @file examples/vk/simple.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan.h>

#include "path.h"

// Helper to load SPIR-V
uint32_t* load_shader(const char* filename, size_t* size) {
    FILE* fp = fopen(filename, "rb");
    if (!fp) {
        perror("Failed to open SPIR-V file");
        return NULL;
    }
    fseek(fp, 0, SEEK_END);
    *size = ftell(fp);
    rewind(fp);

    uint32_t* code = malloc(*size);
    fread(code, sizeof(char), *size, fp);
    fclose(fp);
    return code;
}

int main() {
    char* cwd = getenv("PWD");
    char* shaderPath = path_join(cwd, "/shaders/test.spv"); // malloc, must use free()!
    printf("Current working directory: %s\n", cwd);
    printf("Shader path: %s\n", shaderPath);

    VkResult result;

    // Vulkan instance
    VkInstance instance;

    // Vulkan application information
    VkApplicationInfo applicationInfo = {0}; // Zero-initialize all members
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;  // Structure type
    applicationInfo.pApplicationName = "Vulkan Compute Example"; // Application name (optional)
    applicationInfo.applicationVersion = VK_API_VERSION_1_0;     // Application version
    applicationInfo.pEngineName = "No Engine";                   // Engine name (optional)
    applicationInfo.engineVersion = VK_API_VERSION_1_0;          // Engine version
    applicationInfo.apiVersion = VK_API_VERSION_1_2;             // API version (Vulkan 1.2)

    printf("Application Name: %s\n", applicationInfo.pApplicationName);
    printf("Application Version: %u\n", applicationInfo.applicationVersion);
    printf("Engine Name: %s\n", applicationInfo.pEngineName);
    printf("Engine Version: %u\n", applicationInfo.engineVersion);
    printf("API Version: %u.%u.%u\n",
        VK_API_VERSION_MAJOR(applicationInfo.apiVersion),
        VK_API_VERSION_MINOR(applicationInfo.apiVersion),
        VK_API_VERSION_PATCH(applicationInfo.apiVersion));

    VkInstanceCreateInfo instanceInfo = {0}; // Zero-initialize all members
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pApplicationInfo = &applicationInfo;
    instanceInfo.enabledExtensionCount = 0;
    instanceInfo.ppEnabledExtensionNames = NULL;
    instanceInfo.enabledLayerCount = 0;
    instanceInfo.ppEnabledLayerNames = NULL;
    result = vkCreateInstance(&instanceInfo, NULL, &instance);
    if (result != VK_SUCCESS) {
        fprintf(stderr, "Failed to create VkInstance.\n");
        return EXIT_FAILURE;
    }

    // Physical device
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, NULL);
    VkPhysicalDevice physicalDevice;
    result = vkEnumeratePhysicalDevices(instance, &deviceCount, &physicalDevice);
    if (result != VK_SUCCESS) {
        fprintf(stderr, "Failed to create enumerate physical devices.\n");
        return EXIT_FAILURE;
    }

    // Logical device
    float queuePriority = 1.0f;
    VkDeviceQueueCreateInfo queueCreateInfo
        = {.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
           .queueFamilyIndex = 0,
           .queueCount = 1,
           .pQueuePriorities = &queuePriority};
    VkDeviceCreateInfo deviceCreateInfo
        = {.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
           .queueCreateInfoCount = 1,
           .pQueueCreateInfos = &queueCreateInfo};
    VkDevice device;
    result = vkCreateDevice(physicalDevice, &deviceCreateInfo, NULL, &device);
    if (result != VK_SUCCESS) {
        fprintf(stderr, "Failed to create virtual device.\n");
        return EXIT_FAILURE;
    }

    // Compute shader module
    size_t shaderSize;
    uint32_t* shaderCode = load_shader(shaderPath, &shaderSize);
    if (!shaderCode) {
        fprintf(stderr, "Failed to read shader code.\n");
        return EXIT_FAILURE;
    }
    free(shaderPath); // free the string

    VkShaderModuleCreateInfo shaderModuleCreateInfo
        = {.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
           .codeSize = shaderSize,
           .pCode = shaderCode};
    VkShaderModule shaderModule;
    result = vkCreateShaderModule(device, &shaderModuleCreateInfo, NULL, &shaderModule);
    if (result != VK_SUCCESS) {
        fprintf(stderr, "Failed to read shader module.\n");
        free(shaderCode);
        return EXIT_FAILURE;
    }

    // Compute pipeline
    VkPipelineShaderStageCreateInfo stageCreateInfo
        = {.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
           .stage = VK_SHADER_STAGE_COMPUTE_BIT,
           .module = shaderModule,
           .pName = "main"};
    VkPipelineLayout pipelineLayout;
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo
        = {.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
           .setLayoutCount = 0,
           .pSetLayouts = NULL,
           .pushConstantRangeCount = 0,
           .pPushConstantRanges = NULL};
    result = vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, NULL, &pipelineLayout);
    if (result != VK_SUCCESS) {
        fprintf(stderr, "Failed to create VkPipelineLayout.\n");
        return EXIT_FAILURE;
    }

    VkComputePipelineCreateInfo pipelineCreateInfo
        = {.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
           .stage = stageCreateInfo,
           .layout = pipelineLayout};
    VkPipeline pipeline;
    result
        = vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, NULL, &pipeline);
    if (result != VK_SUCCESS) {
        fprintf(stderr, "Failed to create VkPipeline.\n");
        return EXIT_FAILURE;
    }

    printf("Compute pipeline created successfully.\n");

    // Cleanup
    vkDestroyPipeline(device, pipeline, NULL);
    vkDestroyPipelineLayout(device, pipelineLayout, NULL);
    vkDestroyShaderModule(device, shaderModule, NULL);
    vkDestroyDevice(device, NULL);
    vkDestroyInstance(instance, NULL);
    free(shaderCode);

    return 0;
}
