/**
 * @file examples/vk/simple.c
 * 
 * @warning All objects must be zero-initialized.
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
    if (!fp) {
        return 0;
    }
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

ShaderCode* shader_load(const char* cwd, const char* relative_path) {
    char* filepath = path_join(cwd, relative_path);
    if (!filepath) {
        fprintf(stderr, "Error: Failed to construct path\n");
        return NULL;
    }
    ShaderCode* shader = shader_create(filepath);
    free(filepath);
    return shader;
}

int main() {
    /** Create a vulkan instance object */

    // Create vulkan instance
    VkInstance instance;
    VkResult result;

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

    // Define validation layers
    #define VALIDATION_LAYER_LIMIT 1 /// @warning Cannot be a variable
    const char* validationLayers[VALIDATION_LAYER_LIMIT] = {"VK_LAYER_KHRONOS_validation"};

    // Get the number of validation layers
    uint32_t validationLayerCount = 0;
    vkEnumerateInstanceLayerProperties(&validationLayerCount, NULL);
    // Allocate memory for validation layer properties
    VkLayerProperties* availableLayers = malloc(validationLayerCount * sizeof(VkLayerProperties));
    vkEnumerateInstanceLayerProperties(&validationLayerCount, availableLayers);
    // Discover enumerated validation layers
    for (size_t i = 0; i < VALIDATION_LAYER_LIMIT; i++) {
        bool found = false;
        for (uint32_t j = 0; j < validationLayerCount; j++) {
            if (strcmp(validationLayers[i], availableLayers[j].layerName) == 0) {
                found = true;
                break;
            }
        }
        if (!found) {
            free(availableLayers);
            return EXIT_FAILURE;
        }
    }
    free(availableLayers);

    // Create device info
    VkInstanceCreateInfo instanceInfo = {0}; // Zero-initialize all members
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pApplicationInfo = &applicationInfo;
    instanceInfo.enabledExtensionCount = 0;
    instanceInfo.ppEnabledExtensionNames = NULL;
    instanceInfo.enabledLayerCount = VALIDATION_LAYER_LIMIT;
    instanceInfo.ppEnabledLayerNames = validationLayers;
    result = vkCreateInstance(&instanceInfo, NULL, &instance);
    if (VK_SUCCESS != result) {
        fprintf(stderr, "Failed to create VkInstance.\n");
        return EXIT_FAILURE;
    }

    /** Create a physical device object */

    // Get the physical device count
    uint32_t deviceCount = 0;
    result = vkEnumeratePhysicalDevices(instance, &deviceCount, NULL);
    if (VK_SUCCESS != result || 0 == deviceCount) {
        fprintf(stderr, "Failed to enumerate physical devices or no Vulkan-supported GPU found!\n");
        vkDestroyInstance(instance, NULL);
        return EXIT_FAILURE;
    }
    // Allocate memory for the device list
    VkPhysicalDevice* physicalDeviceList = (VkPhysicalDevice*) malloc(deviceCount * sizeof(VkPhysicalDevice));
    if (!physicalDeviceList) {
        fprintf(stderr, "Failed to allocate memory for physical device list!\n");
        vkDestroyInstance(instance, NULL);
        return EXIT_FAILURE;
    }
    // Enumerate the physical device list
    result = vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDeviceList);
    if (VK_SUCCESS != result) {
        fprintf(stderr, "Failed to enumerate physical devices! (Error code: %d)\n", result);
        free(physicalDeviceList);
        vkDestroyInstance(instance, NULL);
        return EXIT_FAILURE;
    }

    // Iterate through devices and select one
    VkPhysicalDeviceProperties properties = {0};
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    for (uint32_t i = 0; i < deviceCount; i++) {
        vkGetPhysicalDeviceProperties(physicalDeviceList[i], &properties);

        printf("Device Name: %s\n", properties.deviceName);
        printf("Device Type: %d\n", properties.deviceType);
        printf("API Version: %u.%u.%u\n",
               VK_API_VERSION_MAJOR(properties.apiVersion),
               VK_API_VERSION_MINOR(properties.apiVersion),
               VK_API_VERSION_PATCH(properties.apiVersion));
        
        // Prefer discrete GPU
        if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            physicalDevice = physicalDeviceList[i];
            break; // Stop at the first discrete GPU
        }
    }
    // Fallback to first device if no discrete GPU is found
    if (VK_NULL_HANDLE == physicalDevice) {
        fprintf(stderr, "No discrete GPU found. Selecting first available device.\n");
        physicalDevice = physicalDeviceList[0];
    }
    free(physicalDeviceList); // cleanup allocated device list

    /** Create a logical device object */

    // Get the number of available queue families
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, NULL);
    if (0 == queueFamilyCount) {
        fprintf(stderr, "No queue families found on the physical device.\n");
        vkDestroyInstance(instance, NULL);
        return EXIT_FAILURE;
    }
    // Allocate memory for queueing device family properties
    VkQueueFamilyProperties* queueFamilies = (VkQueueFamilyProperties*) malloc(sizeof(VkQueueFamilyProperties) * queueFamilyCount);
    if (!queueFamilies) {
        fprintf(stderr, "Failed to allocate memory for queueing device family properties.\n");
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
        fprintf(stderr, "No compute-capable queue family found.\n");
        vkDestroyInstance(instance, NULL);
        return EXIT_FAILURE;
    }

    // Define the device queue info object
    float queuePriority = 1.0f;
    VkDeviceQueueCreateInfo deviceQueueInfo = {0};
    deviceQueueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    deviceQueueInfo.queueFamilyIndex = computeQueueFamilyIndex; // Use the discovered queue index
    deviceQueueInfo.queueCount = 1;
    deviceQueueInfo.pQueuePriorities = &queuePriority;

    // Specify enabled device features
    VkPhysicalDeviceFeatures deviceFeatures = {0};
    // Enable specific features if required here, e.g., deviceFeatures.samplerAnisotropy = VK_TRUE;

    // Device the device info object
    VkDeviceCreateInfo deviceInfo = {0};
    deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceInfo.queueCreateInfoCount = 1;
    deviceInfo.pQueueCreateInfos = &deviceQueueInfo; // Pass the queue info array
    deviceInfo.enabledExtensionCount = 0; // Modify this if extensions are required
    deviceInfo.ppEnabledExtensionNames = NULL;
    deviceInfo.pEnabledFeatures = &deviceFeatures;

    // Create the logical device
    VkDevice logicalDevice;
    result = vkCreateDevice(physicalDevice, &deviceInfo, NULL, &logicalDevice);
    if (result != VK_SUCCESS) {
        fprintf(stderr, "Failed to create logical device! (Error code: %d)\n", result);
        vkDestroyInstance(instance, NULL);
        return EXIT_FAILURE;
    }

    // Retrieve the compute queue from the logical device
    VkQueue computeQueue;
    vkGetDeviceQueue(logicalDevice, computeQueueFamilyIndex, 0, &computeQueue);
    printf("Logical device and compute queue created successfully.\n");

    /** Read shader into memory */

    // Get current working directory
    char* cwd = getenv("PWD");
    if (!cwd) {
        fprintf(stderr, "Error: Failed to get current working directory\n");
        return EXIT_FAILURE;
    }
    // Load the compute shader
    ShaderCode* shader = shader_load(cwd, "/shaders/test.spv");
    if (!shader) {
        fprintf(stderr, "Failed to load compute shader\n");
        return EXIT_FAILURE;
    }
    // Display shader details
    printf("Loaded shader: %s\n", shader->path);
    printf("File size: %zu bytes\n", shader->size);
    printf("Data count: %zu\n", shader->count);

    // Create the shader module info object
    VkShaderModuleCreateInfo shaderModuleCreateInfo = {0};
    shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderModuleCreateInfo.codeSize = shader->size;
    shaderModuleCreateInfo.pCode = shader->data;

    // Create the shader module object
    VkShaderModule shaderModule;
    result = vkCreateShaderModule(logicalDevice, &shaderModuleCreateInfo, NULL, &shaderModule);
    if (result != VK_SUCCESS) {
        fprintf(stderr, "Failed to read shader module.\n");
        shader_free(shader);
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
    result = vkCreatePipelineLayout(logicalDevice, &pipelineLayoutCreateInfo, NULL, &pipelineLayout);
    if (result != VK_SUCCESS) {
        fprintf(stderr, "Failed to create VkPipelineLayout.\n");
        return EXIT_FAILURE;
    }

    VkComputePipelineCreateInfo pipelineCreateInfo
        = {.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
           .stage = stageCreateInfo,
           .layout = pipelineLayout};
    VkPipeline pipeline;
    result = vkCreateComputePipelines(logicalDevice, VK_NULL_HANDLE, 1, &pipelineCreateInfo, NULL, &pipeline);
    if (result != VK_SUCCESS) {
        fprintf(stderr, "Failed to create VkPipeline.\n");
        return EXIT_FAILURE;
    }

    printf("Compute pipeline created successfully.\n");

    // Cleanup
    vkDestroyPipeline(logicalDevice, pipeline, NULL);
    vkDestroyPipelineLayout(logicalDevice, pipelineLayout, NULL);
    vkDestroyShaderModule(logicalDevice, shaderModule, NULL);
    vkDestroyDevice(logicalDevice, NULL);
    vkDestroyInstance(instance, NULL);
    shader_free(shader);

    return 0;
}
