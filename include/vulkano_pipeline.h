#ifndef VULKANO_PIPELINE_H
#define VULKANO_PIPELINE_H

#include "vulkano.h"
#include "vulkano_swapchain.h"

// The main pipeline structure
typedef struct VulkanoPipeline {
  VkPipelineLayout layout;
  VkRenderPass render_pass;
  VkPipeline pipeline;
  VkDescriptorSetLayout descriptor_set_layout;
  VkDescriptorPool descriptor_pool;
  VkDescriptorSet *descriptor_sets;
} VulkanoPipeline;

// Main pipeline functions
VulkanoResult vulkano_create_pipeline(VulkanoContext *context,
                                      VulkanoSwapchain *swapchain,
                                      VulkanoPipeline *pipeline,
                                      const char *vertex_shader_path,
                                      const char *fragment_shader_path);

void vulkano_destroy_pipeline(VulkanoContext *context,
                              VulkanoPipeline *pipeline);

// Descriptor management
VulkanoResult vulkano_create_descriptor_pool(VulkanoContext *context,
                                             VulkanoPipeline *pipeline,
                                             uint32_t max_frames);

#endif // VULKANO_PIPELINE_H
