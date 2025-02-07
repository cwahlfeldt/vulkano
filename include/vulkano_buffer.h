#ifndef VULKANO_BUFFER_H
#define VULKANO_BUFFER_H

#include "vulkano.h"
#include "vulkano_vertex.h"

// Buffer creation functions
VulkanoResult vulkano_create_buffer(
    VulkanoContext* context,
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags properties,
    VkBuffer* buffer,
    VkDeviceMemory* buffer_memory
);

// Helper functions for specific buffer types
VulkanoResult vulkano_create_vertex_buffer(
    VulkanoContext* context,
    const Vertex* vertices,
    size_t vertex_count,
    VkBuffer* buffer,
    VkDeviceMemory* buffer_memory
);

VulkanoResult vulkano_create_index_buffer(
    VulkanoContext* context,
    const uint16_t* indices,
    size_t index_count,
    VkBuffer* buffer,
    VkDeviceMemory* buffer_memory
);

VulkanoResult vulkano_create_uniform_buffer(
    VulkanoContext* context,
    size_t buffer_size,
    VkBuffer* buffer,
    VkDeviceMemory* buffer_memory,
    void** buffer_mapped
);

#endif // VULKANO_BUFFER_H