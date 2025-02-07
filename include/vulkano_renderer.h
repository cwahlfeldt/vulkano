// include/vulkano_renderer.h
#ifndef VULKANO_RENDERER_H
#define VULKANO_RENDERER_H

#include "vulkano.h"
#include "vulkano_pipeline.h"
#include "vulkano_swapchain.h"

#define MAX_FRAMES_IN_FLIGHT 2

typedef struct VulkanoRenderer {
  VulkanoContext *context;
  VulkanoSwapchain swapchain;
  VulkanoPipeline pipeline;

  // Command buffers
  VkCommandBuffer *command_buffers;
  uint32_t command_buffer_count;

  // Sync objects
  VkSemaphore *image_available_semaphores;
  VkSemaphore *render_finished_semaphores;
  VkFence *in_flight_fences;
  uint32_t current_frame;

  // Cube geometry
  VkBuffer vertex_buffer;
  VkDeviceMemory vertex_buffer_memory;
  VkBuffer index_buffer;
  VkDeviceMemory index_buffer_memory;

  // Uniform buffers for transformation matrices
  VkBuffer *uniform_buffers;
  VkDeviceMemory *uniform_buffers_memory;
  void **uniform_buffers_mapped;

  // For animation
  float rotation;
} VulkanoRenderer;

VulkanoResult vulkano_renderer_init(VulkanoContext *context,
                                    VulkanoRenderer *renderer);
void vulkano_renderer_cleanup(VulkanoRenderer *renderer);
VulkanoResult vulkano_renderer_draw_frame(VulkanoRenderer *renderer);
VulkanoResult vulkano_renderer_wait_idle(VulkanoRenderer *renderer);

// Helper function declarations
void create_sync_objects(VulkanoRenderer *renderer);
void update_uniform_buffer(VulkanoRenderer *renderer, uint32_t current_image);
void cleanup_sync_objects(VulkanoRenderer *renderer);

#endif // VULKANO_RENDERER_H
