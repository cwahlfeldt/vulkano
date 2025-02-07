#include "vulkano_vertex.h"

VkVertexInputBindingDescription vulkano_get_vertex_binding_description(void) {
    VkVertexInputBindingDescription binding_description = {
        .binding = 0,
        .stride = sizeof(Vertex),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
    };
    
    return binding_description;
}

VkVertexInputAttributeDescription* vulkano_get_vertex_attribute_descriptions(uint32_t* count) {
    static VkVertexInputAttributeDescription attribute_descriptions[2] = {
        {
            .binding = 0,
            .location = 0,
            .format = VK_FORMAT_R32G32B32_SFLOAT,
            .offset = offsetof(Vertex, pos)
        },
        {
            .binding = 0,
            .location = 1,
            .format = VK_FORMAT_R32G32B32_SFLOAT,
            .offset = offsetof(Vertex, color)
        }
    };
    
    *count = 2;
    return attribute_descriptions;
}