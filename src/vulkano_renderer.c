// src/vulkano_renderer.c
#include "vulkano_renderer.h"
#include "vulkano_buffer.h"
#include "vulkano_commands.h"
#include "vulkano_cube.h"
#include "vulkano_math.h"
#include "vulkano_vertex.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

VulkanoResult vulkano_renderer_init(VulkanoContext *context,
                                    VulkanoRenderer *renderer) {
  renderer->context = context;
  renderer->current_frame = 0;
  renderer->rotation = 0.0f;

  printf("fuck");

  // Create vertex buffer
  if (vulkano_create_vertex_buffer(
          context, cube_vertices, CUBE_VERTICES_COUNT, &renderer->vertex_buffer,
          &renderer->vertex_buffer_memory) != VULKANO_SUCCESS) {
    return VULKANO_ERROR_INIT_FAILED;
  }

  // Create index buffer
  if (vulkano_create_index_buffer(
          context, cube_indices, CUBE_INDICES_COUNT, &renderer->index_buffer,
          &renderer->index_buffer_memory) != VULKANO_SUCCESS) {
    vulkano_renderer_cleanup(renderer);
    return VULKANO_ERROR_INIT_FAILED;
  }

  // Create uniform buffers
  renderer->uniform_buffers = malloc(sizeof(VkBuffer) * MAX_FRAMES_IN_FLIGHT);
  renderer->uniform_buffers_memory =
      malloc(sizeof(VkDeviceMemory) * MAX_FRAMES_IN_FLIGHT);
  renderer->uniform_buffers_mapped =
      malloc(sizeof(void *) * MAX_FRAMES_IN_FLIGHT);

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    if (vulkano_create_uniform_buffer(
            context, sizeof(UniformBufferObject), &renderer->uniform_buffers[i],
            &renderer->uniform_buffers_memory[i],
            &renderer->uniform_buffers_mapped[i]) != VULKANO_SUCCESS) {
      vulkano_renderer_cleanup(renderer);
      return VULKANO_ERROR_INIT_FAILED;
    }
  }

  // Create sync objects
  create_sync_objects(renderer);

  return VULKANO_SUCCESS;
}

void create_sync_objects(VulkanoRenderer *renderer) {
  renderer->image_available_semaphores =
      malloc(sizeof(VkSemaphore) * MAX_FRAMES_IN_FLIGHT);
  renderer->render_finished_semaphores =
      malloc(sizeof(VkSemaphore) * MAX_FRAMES_IN_FLIGHT);
  renderer->in_flight_fences = malloc(sizeof(VkFence) * MAX_FRAMES_IN_FLIGHT);

  VkSemaphoreCreateInfo semaphore_info = {
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};

  VkFenceCreateInfo fence_info = {.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
                                  .flags = VK_FENCE_CREATE_SIGNALED_BIT};

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    vkCreateSemaphore(renderer->context->device, &semaphore_info, NULL,
                      &renderer->image_available_semaphores[i]);
    vkCreateSemaphore(renderer->context->device, &semaphore_info, NULL,
                      &renderer->render_finished_semaphores[i]);
    vkCreateFence(renderer->context->device, &fence_info, NULL,
                  &renderer->in_flight_fences[i]);
  }
}

void update_uniform_buffer(VulkanoRenderer *renderer, uint32_t current_image) {
  UniformBufferObject ubo = {0};

  // Update model matrix (rotation)
  matrix_rotation_y(ubo.model, renderer->rotation);

  // View matrix (simple camera looking at origin)
  matrix_identity(ubo.view);
  ubo.view[14] = -3.0f; // Move camera back 3 units

  // Projection matrix
  float aspect = (float)renderer->swapchain.extent.width /
                 (float)renderer->swapchain.extent.height;
  matrix_perspective(ubo.proj, (float)(M_PI / 4.0), aspect, 0.1f, 10.0f);

  // Copy to uniform buffer
  memcpy(renderer->uniform_buffers_mapped[current_image], &ubo, sizeof(ubo));

  // Update rotation for next frame
  renderer->rotation += 0.01f;
  if (renderer->rotation > 2 * M_PI) {
    renderer->rotation = 0.0f;
  }
}

VulkanoResult vulkano_renderer_draw_frame(VulkanoRenderer *renderer) {
  vkWaitForFences(renderer->context->device, 1,
                  &renderer->in_flight_fences[renderer->current_frame], VK_TRUE,
                  UINT64_MAX);

  uint32_t image_index;
  VkResult result = vkAcquireNextImageKHR(
      renderer->context->device, renderer->swapchain.swapchain, UINT64_MAX,
      renderer->image_available_semaphores[renderer->current_frame],
      VK_NULL_HANDLE, &image_index);

  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    return VULKANO_ERROR_SWAPCHAIN_OUTDATED;
  } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    return VULKANO_ERROR_INIT_FAILED;
  }

  update_uniform_buffer(renderer, renderer->current_frame);

  vkResetFences(renderer->context->device, 1,
                &renderer->in_flight_fences[renderer->current_frame]);

  vkResetCommandBuffer(renderer->command_buffers[image_index], 0);
  vulkano_record_command_buffer(renderer, image_index);

  VkSubmitInfo submit_info = {
      .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
      .waitSemaphoreCount = 1,
      .pWaitSemaphores =
          &renderer->image_available_semaphores[renderer->current_frame],
      .pWaitDstStageMask =
          (VkPipelineStageFlags[]){
              VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT},
      .commandBufferCount = 1,
      .pCommandBuffers = &renderer->command_buffers[image_index],
      .signalSemaphoreCount = 1,
      .pSignalSemaphores =
          &renderer->render_finished_semaphores[renderer->current_frame]};

  if (vkQueueSubmit(renderer->context->graphics_queue, 1, &submit_info,
                    renderer->in_flight_fences[renderer->current_frame]) !=
      VK_SUCCESS) {
    return VULKANO_ERROR_INIT_FAILED;
  }

  VkPresentInfoKHR present_info = {
      .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
      .waitSemaphoreCount = 1,
      .pWaitSemaphores =
          &renderer->render_finished_semaphores[renderer->current_frame],
      .swapchainCount = 1,
      .pSwapchains = &renderer->swapchain.swapchain,
      .pImageIndices = &image_index};

  result = vkQueuePresentKHR(renderer->context->graphics_queue, &present_info);

  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
    return VULKANO_ERROR_SWAPCHAIN_OUTDATED;
  } else if (result != VK_SUCCESS) {
    return VULKANO_ERROR_INIT_FAILED;
  }

  renderer->current_frame =
      (renderer->current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
  return VULKANO_SUCCESS;
}

void cleanup_sync_objects(VulkanoRenderer *renderer) {
  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    vkDestroySemaphore(renderer->context->device,
                       renderer->image_available_semaphores[i], NULL);
    vkDestroySemaphore(renderer->context->device,
                       renderer->render_finished_semaphores[i], NULL);
    vkDestroyFence(renderer->context->device, renderer->in_flight_fences[i],
                   NULL);
  }

  free(renderer->image_available_semaphores);
  free(renderer->render_finished_semaphores);
  free(renderer->in_flight_fences);
}

void vulkano_renderer_cleanup(VulkanoRenderer *renderer) {
  vkDeviceWaitIdle(renderer->context->device);

  // Clean up uniform buffers
  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    vkDestroyBuffer(renderer->context->device, renderer->uniform_buffers[i],
                    NULL);
    vkFreeMemory(renderer->context->device, renderer->uniform_buffers_memory[i],
                 NULL);
  }
  free(renderer->uniform_buffers);
  free(renderer->uniform_buffers_memory);
  free(renderer->uniform_buffers_mapped);

  // Clean up vertex and index buffers
  vkDestroyBuffer(renderer->context->device, renderer->vertex_buffer, NULL);
  vkFreeMemory(renderer->context->device, renderer->vertex_buffer_memory, NULL);
  vkDestroyBuffer(renderer->context->device, renderer->index_buffer, NULL);
  vkFreeMemory(renderer->context->device, renderer->index_buffer_memory, NULL);

  cleanup_sync_objects(renderer);
}

VulkanoResult vulkano_renderer_wait_idle(VulkanoRenderer *renderer) {
  return vkDeviceWaitIdle(renderer->context->device) == VK_SUCCESS
             ? VULKANO_SUCCESS
             : VULKANO_ERROR_DEVICE_LOST;
}
