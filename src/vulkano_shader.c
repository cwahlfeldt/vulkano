#include "vulkano_shader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned char* read_file(const char* filename, size_t* size) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    *size = ftell(file);
    fseek(file, 0, SEEK_SET);

    unsigned char* buffer = malloc(*size);
    if (!buffer) {
        fclose(file);
        return NULL;
    }

    size_t bytes_read = fread(buffer, 1, *size, file);
    fclose(file);

    if (bytes_read != *size) {
        free(buffer);
        return NULL;
    }

    return buffer;
}

VkShaderModule vulkano_create_shader_module(VulkanoContext* context, const char* filename) {
    size_t size;
    unsigned char* code = read_file(filename, &size);
    if (!code) {
        return VK_NULL_HANDLE;
    }

    VkShaderModuleCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = size,
        .pCode = (const uint32_t*)code
    };

    VkShaderModule shader_module;
    VkResult result = vkCreateShaderModule(context->device, &create_info, NULL, &shader_module);
    
    free(code);
    
    if (result != VK_SUCCESS) {
        return VK_NULL_HANDLE;
    }

    return shader_module;
}

void vulkano_destroy_shader_module(VulkanoContext* context, VkShaderModule shader_module) {
    if (shader_module != VK_NULL_HANDLE) {
        vkDestroyShaderModule(context->device, shader_module, NULL);
    }
}