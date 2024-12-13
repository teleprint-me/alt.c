/**
 * Copyright © 2024 Austin Berrio
 *
 * @file src/vk/shader.c
 */

#include "vk/shader.h"

#include <stdlib.h>

VkShaderModuleCreateInfo vulkan_create_shader_module_info(char* buffer, size_t codeSize) {
    VkShaderModuleCreateInfo shaderInfo = {0};
    shaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderInfo.codeSize = codeSize;
    shaderInfo.pCode = (uint32_t*) buffer;
    return shaderInfo;
}

FILE* vulkan_read_shader_module(const char* filepath) {
    FILE* file = fopen(filepath, "rb");
    if (NULL == file) {
        fprintf(stderr, "Failed to open SPIR-V file: %s\n", filepath);
        exit(EXIT_FAILURE);
    }
    return file;
}

size_t vulkan_calculate_shader_module_size(FILE* file) {
    // Get file size
    fseek(file, 0, SEEK_END);
    size_t fileSize = ftell(file);
    rewind(file);
    return fileSize;
}

char* vulkan_create_shader_module_buffer(FILE* file, size_t fileSize) {
    char* buffer = (char*) malloc(fileSize);
    if (NULL == buffer) {
        fprintf(stderr, "Failed to allocate shader module buffer: %zu\n", fileSize);
        exit(EXIT_FAILURE);
    }
    fread(buffer, 1, fileSize, file);
    return buffer;
}

vulkan_shader_t* vulkan_create_shader(VkDevice device, const char* filepath) {
    vulkan_shader_t* shader = (vulkan_shader_t*) malloc(sizeof(vulkan_shader_t));
    if (NULL == shader) {
        fprintf(stderr, "Failed to allocate memory for Vulkan shader structure\n");
        exit(EXIT_FAILURE);
    }

    FILE* file = vulkan_read_shader_module(filepath);
    shader->bufferSize = vulkan_calculate_shader_module_size(file);
    shader->buffer = vulkan_create_shader_module_buffer(file, shader->bufferSize);

    VkShaderModuleCreateInfo shaderInfo
        = vulkan_create_shader_module_info(shader->buffer, shader->bufferSize);

    VkResult result = vkCreateShaderModule(device, &shaderInfo, NULL, &shader->module);
    if (VK_SUCCESS != result) {
        fprintf(stderr, "Failed to create shader module!\n");
        exit(EXIT_FAILURE);
    }

    fclose(file); // Close the SPIR-V file as it's no longer needed

    return shader;
}

void vulkan_destroy_shader(VkDevice device, vulkan_shader_t* shader) {
    if (shader) {
        if (VK_NULL_HANDLE != shader->module) {
            vkDestroyShaderModule(device, shader->module, NULL);
        }
        if (shader->buffer) {
            free(shader->buffer); // Free the shader code buffer
        }
        free(shader); // Free the shader structure
    }
}
