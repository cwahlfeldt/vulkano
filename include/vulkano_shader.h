#ifndef VULKANO_SHADER_H
#define VULKANO_SHADER_H

#include "vulkano.h"

VkShaderModule vulkano_create_shader_module(VulkanoContext* context, const char* filename);
void vulkano_destroy_shader_module(VulkanoContext* context, VkShaderModule shader_module);

#endif // VULKANO_SHADER_H