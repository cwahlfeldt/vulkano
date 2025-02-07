#ifndef VULKANO_VERTEX_H
#define VULKANO_VERTEX_H

#include "vulkano.h"

typedef struct Vertex {
    float pos[3];
    float color[3];
} Vertex;

typedef struct UniformBufferObject {
    float model[16];
    float view[16];
    float proj[16];
} UniformBufferObject;

VkVertexInputBindingDescription vulkano_get_vertex_binding_description(void);
VkVertexInputAttributeDescription* vulkano_get_vertex_attribute_descriptions(uint32_t* count);

#endif // VULKANO_VERTEX_H