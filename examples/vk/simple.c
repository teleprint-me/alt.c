/**
 * @file examples/vk/simple.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan.h>

#include "path.h"

typedef struct ShaderCode {
    size_t size;       // File size in bytes
    size_t count;      // Number of 32-bit elements
    char* path;        // Shader file path
    uint32_t* data;    // Shader binary data
} ShaderCode;

size_t shader_size(FILE* fp) {
    fseek(fp, 0, SEEK_END);
    size_t size = ftell(fp);
    rewind(fp);
    return size;
}

/**
 * @brief Loads a SPIR-V binary shader file into a ShaderCode struct.
 * 
 * @param filepath Path to the SPIR-V file.
 * @return Pointer to the ShaderCode struct or NULL on failure.
 * Caller is responsible for freeing the returned structure using shader_free.
 */
ShaderCode* shader_create(const char* filepath) {
    FILE* fp = fopen(filepath, "rb");
    if (!fp) {
        fprintf(stderr, "Failed to open SPIR-V file: %s\n", filepath);
        return NULL;
    }

    ShaderCode* code = (ShaderCode*) malloc(sizeof(ShaderCode));
    if (!code) {
        fprintf(stderr, "Failed to allocate memory for ShaderCode\n");
        fclose(fp);
        return NULL;
    }

    code->path = strdup(filepath); // Duplicate the file path for safe reference
    if (!code->path) {
        fprintf(stderr, "Failed to allocate memory for file path\n");
        free(code);
        fclose(fp);
        return NULL;
    }

    code->size = shader_size(fp);
    if (code->size % sizeof(uint32_t) != 0) {
        fprintf(stderr, "Invalid SPIR-V file size: %zu\n", code->size);
        free(code->path);
        free(code);
        fclose(fp);
        return NULL;
    }
    code->count = code->size / sizeof(uint32_t);

    code->data = (uint32_t*) malloc(code->size);
    if (!code->data) {
        fprintf(stderr, "Failed to allocate memory for shader data\n");
        free(code->path);
        free(code);
        fclose(fp);
        return NULL;
    }

    if (fread(code->data, sizeof(uint32_t), code->count, fp) != code->count) {
        fprintf(stderr, "Failed to read SPIR-V file completely\n");
        free(code->data);
        free(code->path);
        free(code);
        fclose(fp);
        return NULL;
    }

    fclose(fp);
    return code;
}

/**
 * @brief Frees the resources associated with a ShaderCode struct.
 * 
 * @param code Pointer to the ShaderCode struct to free.
 */
void shader_free(ShaderCode* code) {
    if (code){
        if (code->data) {
            free(code->data);
        }
        if (code->path) {
            free(code->path);
        }
        free(code);
    }
}

int main() {
    // Load the compute shader
    char* cwd = getenv("PWD"); // get the current working directory
    char* filepath = path_join(cwd, "/shaders/test.spv"); // malloc, must use free()!
    printf("Current working directory: %s\n", cwd);
    printf("Shader path: %s\n", filepath);
    ShaderCode* shader = shader_create(filepath);
    free(filepath); // free the malloced filepath
    if (!shader) {
        fprintf(stderr, "Failed to load shader\n");
        return EXIT_FAILURE;
    }
    printf("Loaded shader: %s\n", shader->path);
    printf("File size: %zu bytes\n", shader->size);
    printf("Data count: %zu\n", shader->count);

    VkResult result;

    // Create vulkan instance
    VkInstance instance;

    // Create application information
    VkApplicationInfo applicationInfo = {0}; // Zero-initialize all members
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;  // Structure type
    applicationInfo.pApplicationName = "Vulkan Compute Example"; // Application name (optional)
    applicationInfo.applicationVersion = VK_API_VERSION_1_0;     // Application version
    applicationInfo.pEngineName = "No Engine";                   // Engine name (optional)
    applicationInfo.engineVersion = VK_API_VERSION_1_0;          // Engine version
    applicationInfo.apiVersion = VK_API_VERSION_1_2;             // API version (Vulkan 1.2)

    // Output application information
    printf("Application Name: %s\n", applicationInfo.pApplicationName);
    printf("Application Version: %u\n", applicationInfo.applicationVersion);
    printf("Engine Name: %s\n", applicationInfo.pEngineName);
    printf("Engine Version: %u\n", applicationInfo.engineVersion);
    printf("API Version: %u.%u.%u\n",
        VK_API_VERSION_MAJOR(applicationInfo.apiVersion),
        VK_API_VERSION_MINOR(applicationInfo.apiVersion),
        VK_API_VERSION_PATCH(applicationInfo.apiVersion));

    // Create device info
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

    // Enumerate physical device count
    uint32_t deviceCount = 0;
    result = vkEnumeratePhysicalDevices(instance, &deviceCount, NULL);
    if (VK_SUCCESS != result) {
        fprintf(stderr, "Failed to enumerate physical devices! (Error code: %d)\n", result);
        exit(EXIT_FAILURE);
    }
    if (0 == deviceCount) {
        fprintf(stderr, "No GPUs with Vulkan support found!\n");
        exit(EXIT_FAILURE);
    }

    // Enumerate physical device list
    VkPhysicalDevice* physicalDeviceList
        = (VkPhysicalDevice*) malloc(deviceCount * sizeof(VkPhysicalDevice));
    if (NULL == physicalDeviceList) {
        fprintf(stderr, "Failed to allocate memory for physical device list!\n");
        return NULL;
    }
    result = vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDeviceList);
    if (VK_SUCCESS != result) {
        fprintf(stderr, "Failed to enumerate physical devices! (Error code: %d)\n", result);
        free(physicalDeviceList);
        return NULL;
    }

    // Physical device
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
    shader_free(shader);

    return 0;
}
