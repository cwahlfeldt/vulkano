#include "vulkano_buffer.h"
#include <string.h>

static uint32_t find_memory_type(
    VulkanoContext* context,
    uint32_t type_filter,
    VkMemoryPropertyFlags properties
) {
    VkPhysicalDeviceMemoryProperties mem_properties;
    vkGetPhysicalDeviceMemoryProperties(context->physical_device, &mem_properties);

    for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++) {
        if ((type_filter & (1 << i)) && 
            (mem_properties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    return UINT32_MAX;
}

VulkanoResult vulkano_create_buffer(
    VulkanoContext* context,
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags properties,
    VkBuffer* buffer,
    VkDeviceMemory* buffer_memory
) {
    VkBufferCreateInfo buffer_info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };

    if (vkCreateBuffer(context->device, &buffer_info, NULL, buffer) != VK_SUCCESS) {
        return VULKANO_ERROR_INIT_FAILED;
    }

    VkMemoryRequirements mem_requirements;
    vkGetBufferMemoryRequirements(context->device, *buffer, &mem_requirements);

    uint32_t memory_type = find_memory_type(context, mem_requirements.memoryTypeBits, properties);
    if (memory_type == UINT32_MAX) {
        vkDestroyBuffer(context->device, *buffer, NULL);
        return VULKANO_ERROR_INIT_FAILED;
    }

    VkMemoryAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = mem_requirements.size,
        .memoryTypeIndex = memory_type
    };

    if (vkAllocateMemory(context->device, &alloc_info, NULL, buffer_memory) != VK_SUCCESS) {
        vkDestroyBuffer(context->device, *buffer, NULL);
        return VULKANO_ERROR_INIT_FAILED;
    }

    if (vkBindBufferMemory(context->device, *buffer, *buffer_memory, 0) != VK_SUCCESS) {
        vkDestroyBuffer(context->device, *buffer, NULL);
        vkFreeMemory(context->device, *buffer_memory, NULL);
        return VULKANO_ERROR_INIT_FAILED;
    }

    return VULKANO_SUCCESS;
}

VulkanoResult vulkano_create_vertex_buffer(
    VulkanoContext* context,
    const Vertex* vertices,
    size_t vertex_count,
    VkBuffer* buffer,
    VkDeviceMemory* buffer_memory
) {
    VkDeviceSize buffer_size = sizeof(Vertex) * vertex_count;

    // Create staging buffer
    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;
    
    if (vulkano_create_buffer(
            context,
            buffer_size,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            &staging_buffer,
            &staging_buffer_memory
        ) != VULKANO_SUCCESS) {
        return VULKANO_ERROR_INIT_FAILED;
    }

    // Copy vertices to staging buffer
    void* data;
    vkMapMemory(context->device, staging_buffer_memory, 0, buffer_size, 0, &data);
    memcpy(data, vertices, buffer_size);
    vkUnmapMemory(context->device, staging_buffer_memory);

    // // Create vertex buffer
    if (vulkano_create_buffer(
            context,
            buffer_size,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            buffer,
            buffer_memory
        ) != VULKANO_SUCCESS) {
        vkDestroyBuffer(context->device, staging_buffer, NULL);
        vkFreeMemory(context->device, staging_buffer_memory, NULL);
        return VULKANO_ERROR_INIT_FAILED;
    }

    // // Copy data from staging buffer to vertex buffer
    VkCommandBuffer command_buffer;
    VkCommandBufferAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandPool = context->command_pool,
        .commandBufferCount = 1
    };

    // BUSTED
    // ////////////////////////////////////////////////////////////////////////
    vkAllocateCommandBuffers(context->device, &alloc_info, &command_buffer);

    VkCommandBufferBeginInfo begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };

    vkBeginCommandBuffer(command_buffer, &begin_info);

    VkBufferCopy copy_region = {
        .srcOffset = 0,
        .dstOffset = 0,
        .size = buffer_size
    };

    vkCmdCopyBuffer(command_buffer, staging_buffer, *buffer, 1, &copy_region);
    vkEndCommandBuffer(command_buffer);

    VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &command_buffer
    };

    vkQueueSubmit(context->graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(context->graphics_queue);

    vkFreeCommandBuffers(context->device, context->command_pool, 1,
                         &command_buffer);
    vkDestroyBuffer(context->device, staging_buffer, NULL);
    vkFreeMemory(context->device, staging_buffer_memory, NULL);

    return VULKANO_SUCCESS;
}

VulkanoResult vulkano_create_index_buffer(
    VulkanoContext* context,
    const uint16_t* indices,
    size_t index_count,
    VkBuffer* buffer,
    VkDeviceMemory* buffer_memory
) {
    VkDeviceSize buffer_size = sizeof(uint16_t) * index_count;

    // Create staging buffer
    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;
    
    if (vulkano_create_buffer(
            context,
            buffer_size,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            &staging_buffer,
            &staging_buffer_memory
        ) != VULKANO_SUCCESS) {
        return VULKANO_ERROR_INIT_FAILED;
    }

    // Copy indices to staging buffer
    void* data;
    vkMapMemory(context->device, staging_buffer_memory, 0, buffer_size, 0, &data);
    memcpy(data, indices, buffer_size);
    vkUnmapMemory(context->device, staging_buffer_memory);

    // Create index buffer
    if (vulkano_create_buffer(
            context,
            buffer_size,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            buffer,
            buffer_memory
        ) != VULKANO_SUCCESS) {
        vkDestroyBuffer(context->device, staging_buffer, NULL);
        vkFreeMemory(context->device, staging_buffer_memory, NULL);
        return VULKANO_ERROR_INIT_FAILED;
    }

    // Copy data from staging buffer to index buffer
    VkCommandBuffer command_buffer;
    VkCommandBufferAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandPool = context->command_pool,
        .commandBufferCount = 1
    };

    vkAllocateCommandBuffers(context->device, &alloc_info, &command_buffer);

    VkCommandBufferBeginInfo begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };

    vkBeginCommandBuffer(command_buffer, &begin_info);

    VkBufferCopy copy_region = {
        .srcOffset = 0,
        .dstOffset = 0,
        .size = buffer_size
    };

    vkCmdCopyBuffer(command_buffer, staging_buffer, *buffer, 1, &copy_region);
    vkEndCommandBuffer(command_buffer);

    VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &command_buffer
    };

    vkQueueSubmit(context->graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(context->graphics_queue);

    vkFreeCommandBuffers(context->device, context->command_pool, 1, &command_buffer);
    vkDestroyBuffer(context->device, staging_buffer, NULL);
    vkFreeMemory(context->device, staging_buffer_memory, NULL);

    return VULKANO_SUCCESS;
}

VulkanoResult vulkano_create_uniform_buffer(
    VulkanoContext* context,
    size_t buffer_size,
    VkBuffer* buffer,
    VkDeviceMemory* buffer_memory,
    void** buffer_mapped
) {
    VulkanoResult result = vulkano_create_buffer(
        context,
        buffer_size,
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        buffer,
        buffer_memory
    );

    if (result == VULKANO_SUCCESS) {
        vkMapMemory(context->device, *buffer_memory, 0, buffer_size, 0, buffer_mapped);
    }

    return result;
}
