#include "vulkano_commands.h"
#include "vulkano_vertex.h"
#include <stdlib.h>

VulkanoResult vulkano_create_command_buffers(VulkanoRenderer *renderer) {
  // Allocate space for command buffers (one per swapchain image)
  renderer->command_buffer_count = renderer->swapchain.image_count;
  renderer->command_buffers =
      malloc(sizeof(VkCommandBuffer) * renderer->command_buffer_count);

  if (!renderer->command_buffers) {
    return VULKANO_ERROR_INIT_FAILED;
  }

  // Command buffer allocation info
  VkCommandBufferAllocateInfo alloc_info = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .commandPool = renderer->context->command_pool,
      .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount = renderer->command_buffer_count};

  // Allocate command buffers
  VkResult result = vkAllocateCommandBuffers(
      renderer->context->device, &alloc_info, renderer->command_buffers);

  if (result != VK_SUCCESS) {
    free(renderer->command_buffers);
    renderer->command_buffers = NULL;
    return VULKANO_ERROR_INIT_FAILED;
  }

  return VULKANO_SUCCESS;
}

VulkanoResult vulkano_record_command_buffer(VulkanoRenderer *renderer,
                                            uint32_t image_index) {
  VkCommandBuffer command_buffer = renderer->command_buffers[image_index];

  // Begin command buffer recording
  VkCommandBufferBeginInfo begin_info = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .flags = 0,
      .pInheritanceInfo = NULL};

  if (vkBeginCommandBuffer(command_buffer, &begin_info) != VK_SUCCESS) {
    return VULKANO_ERROR_INIT_FAILED;
  }

  // Begin render pass
  VkRenderPassBeginInfo render_pass_info = {
      .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
      .renderPass = renderer->pipeline.render_pass,
      .framebuffer = renderer->swapchain.framebuffers[image_index],
      .renderArea.offset = {0, 0},
      .renderArea.extent = renderer->swapchain.extent};

  // Clear color (black background)
  VkClearValue clear_color = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
  render_pass_info.clearValueCount = 1;
  render_pass_info.pClearValues = &clear_color;

  // Begin the render pass
  vkCmdBeginRenderPass(command_buffer, &render_pass_info,
                       VK_SUBPASS_CONTENTS_INLINE);

  // Bind the graphics pipeline
  vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                    renderer->pipeline.pipeline);

  // Set viewport and scissor (if they're not dynamic)
  VkViewport viewport = {.x = 0.0f,
                         .y = 0.0f,
                         .width = (float)renderer->swapchain.extent.width,
                         .height = (float)renderer->swapchain.extent.height,
                         .minDepth = 0.0f,
                         .maxDepth = 1.0f};
  vkCmdSetViewport(command_buffer, 0, 1, &viewport);

  VkRect2D scissor = {.offset = {0, 0}, .extent = renderer->swapchain.extent};
  vkCmdSetScissor(command_buffer, 0, 1, &scissor);

  // Bind vertex and index buffers (we'll implement this later)
  VkBuffer vertex_buffers[] = {renderer->vertex_buffer};
  VkDeviceSize offsets[] = {0};
  vkCmdBindVertexBuffers(command_buffer, 0, 1, vertex_buffers, offsets);
  vkCmdBindIndexBuffer(command_buffer, renderer->index_buffer, 0,
                       VK_INDEX_TYPE_UINT16);

  // Bind descriptor sets for uniform buffers (we'll implement this later)
  vkCmdBindDescriptorSets(
      command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
      renderer->pipeline.layout, 0, 1,
      &renderer->pipeline.descriptor_sets[renderer->current_frame], 0, NULL);

  // Draw command
  vkCmdDrawIndexed(command_buffer, 36, 1, 0, 0, 0); // 36 indices for a cube

  // End render pass
  vkCmdEndRenderPass(command_buffer);

  // End command buffer recording
  if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS) {
    return VULKANO_ERROR_INIT_FAILED;
  }

  return VULKANO_SUCCESS;
}

void vulkano_destroy_command_buffers(VulkanoRenderer *renderer) {
  if (renderer->command_buffers) {
    vkFreeCommandBuffers(
        renderer->context->device, renderer->context->command_pool,
        renderer->command_buffer_count, renderer->command_buffers);
    free(renderer->command_buffers);
    renderer->command_buffers = NULL;
    renderer->command_buffer_count = 0;
  }
}
