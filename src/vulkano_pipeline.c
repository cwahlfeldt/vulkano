#include "vulkano_pipeline.h"
#include "vulkano_shader.h"
#include "vulkano_vertex.h"
#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan_core.h>

static VkResult create_render_pass(VulkanoContext *context,
                                   VulkanoSwapchain *swapchain,
                                   VulkanoPipeline *pipeline) {
  // Color attachment description
  VkAttachmentDescription color_attachment = {
      .format = swapchain->format,
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
      .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
      .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
      .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR};

  // Attachment reference
  VkAttachmentReference color_attachment_ref = {
      .attachment = 0, .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

  // Subpass description
  VkSubpassDescription subpass = {.pipelineBindPoint =
                                      VK_PIPELINE_BIND_POINT_GRAPHICS,
                                  .colorAttachmentCount = 1,
                                  .pColorAttachments = &color_attachment_ref};

  // Subpass dependency
  VkSubpassDependency dependency = {
      .srcSubpass = VK_SUBPASS_EXTERNAL,
      .dstSubpass = 0,
      .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
      .srcAccessMask = 0,
      .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
      .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT};

  // Create render pass
  VkRenderPassCreateInfo render_pass_info = {
      .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
      .attachmentCount = 1,
      .pAttachments = &color_attachment,
      .subpassCount = 1,
      .pSubpasses = &subpass,
      .dependencyCount = 1,
      .pDependencies = &dependency};

  return vkCreateRenderPass(context->device, &render_pass_info, NULL,
                            &pipeline->render_pass);
}

static VkResult create_descriptor_set_layout(VulkanoContext *context,
                                             VulkanoPipeline *pipeline) {
  VkDescriptorSetLayoutBinding ubo_layout_binding = {
      .binding = 0,
      .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
      .descriptorCount = 1,
      .stageFlags = VK_SHADER_STAGE_VERTEX_BIT};

  VkDescriptorSetLayoutCreateInfo layout_info = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
      .bindingCount = 1,
      .pBindings = &ubo_layout_binding};

  return vkCreateDescriptorSetLayout(context->device, &layout_info, NULL,
                                     &pipeline->descriptor_set_layout);
}

static void
setup_shader_stages(VkShaderModule vert_shader, VkShaderModule frag_shader,
                    VkPipelineShaderStageCreateInfo shader_stages[2]) {
  shader_stages[0] = (VkPipelineShaderStageCreateInfo){
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .stage = VK_SHADER_STAGE_VERTEX_BIT,
      .module = vert_shader,
      .pName = "main"};

  shader_stages[1] = (VkPipelineShaderStageCreateInfo){
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
      .module = frag_shader,
      .pName = "main"};
}

static void setup_vertex_input_state(
    VkPipelineVertexInputStateCreateInfo *vertex_input_info,
    const VkVertexInputBindingDescription *binding_description,
    const VkVertexInputAttributeDescription *attribute_descriptions,
    uint32_t attribute_count) {
  *vertex_input_info = (VkPipelineVertexInputStateCreateInfo){
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
      .vertexBindingDescriptionCount = 1,
      .pVertexBindingDescriptions = binding_description,
      .vertexAttributeDescriptionCount = attribute_count,
      .pVertexAttributeDescriptions = attribute_descriptions};
}

static void
setup_input_assembly(VkPipelineInputAssemblyStateCreateInfo *input_assembly) {
  *input_assembly = (VkPipelineInputAssemblyStateCreateInfo){
      .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
      .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
      .primitiveRestartEnable = VK_FALSE};
}

static void
setup_viewport_state(VkPipelineViewportStateCreateInfo *viewport_state,
                     VkViewport *viewport, VkRect2D *scissor,
                     const VkExtent2D *extent) {
  *viewport = (VkViewport){.x = 0.0f,
                           .y = 0.0f,
                           .width = (float)extent->width,
                           .height = (float)extent->height,
                           .minDepth = 0.0f,
                           .maxDepth = 1.0f};

  *scissor = (VkRect2D){.offset = {0, 0}, .extent = *extent};

  *viewport_state = (VkPipelineViewportStateCreateInfo){
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
      .viewportCount = 1,
      .pViewports = viewport,
      .scissorCount = 1,
      .pScissors = scissor};
}

static void
setup_rasterization_state(VkPipelineRasterizationStateCreateInfo *rasterizer) {
  *rasterizer = (VkPipelineRasterizationStateCreateInfo){
      .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
      .depthClampEnable = VK_FALSE,
      .rasterizerDiscardEnable = VK_FALSE,
      .polygonMode = VK_POLYGON_MODE_FILL,
      .lineWidth = 1.0f,
      .cullMode = VK_CULL_MODE_BACK_BIT,
      .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
      .depthBiasEnable = VK_FALSE};
}

static void
setup_multisampling(VkPipelineMultisampleStateCreateInfo *multisampling) {
  *multisampling = (VkPipelineMultisampleStateCreateInfo){
      .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
      .sampleShadingEnable = VK_FALSE,
      .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT};
}

static void setup_color_blend_state(
    VkPipelineColorBlendStateCreateInfo *color_blending,
    VkPipelineColorBlendAttachmentState *color_blend_attachment) {
  *color_blend_attachment = (VkPipelineColorBlendAttachmentState){
      .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
      .blendEnable = VK_FALSE};

  *color_blending = (VkPipelineColorBlendStateCreateInfo){
      .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
      .logicOpEnable = VK_FALSE,
      .attachmentCount = 1,
      .pAttachments = color_blend_attachment};
}

VulkanoResult vulkano_create_pipeline(VulkanoContext *context,
                                      VulkanoSwapchain *swapchain,
                                      VulkanoPipeline *pipeline,
                                      const char *vertex_shader_path,
                                      const char *fragment_shader_path) {
  // Initialize the pipeline to a clean state
  memset(pipeline, 0, sizeof(VulkanoPipeline));

  // Create render pass
  if (create_render_pass(context, swapchain, pipeline) != VK_SUCCESS) {
    return VULKANO_ERROR_INIT_FAILED;
  }

  // Create descriptor set layout
  if (create_descriptor_set_layout(context, pipeline) != VK_SUCCESS) {
    vulkano_destroy_pipeline(context, pipeline);
    return VULKANO_ERROR_INIT_FAILED;
  }

  // Create shader modules
  VkShaderModule vert_shader_module =
      vulkano_create_shader_module(context, vertex_shader_path);
  VkShaderModule frag_shader_module =
      vulkano_create_shader_module(context, fragment_shader_path);

  if (vert_shader_module == VK_NULL_HANDLE ||
      frag_shader_module == VK_NULL_HANDLE) {
    vulkano_destroy_shader_module(context, vert_shader_module);
    vulkano_destroy_shader_module(context, frag_shader_module);
    vulkano_destroy_pipeline(context, pipeline);
    return VULKANO_ERROR_INIT_FAILED;
  }

  // Set up all pipeline states
  VkPipelineShaderStageCreateInfo shader_stages[2];
  setup_shader_stages(vert_shader_module, frag_shader_module, shader_stages);

  // Vertex input state
  VkVertexInputBindingDescription binding_description =
      vulkano_get_vertex_binding_description();
  uint32_t attribute_count;
  VkVertexInputAttributeDescription *attribute_descriptions =
      vulkano_get_vertex_attribute_descriptions(&attribute_count);

  VkPipelineVertexInputStateCreateInfo vertex_input_info;
  setup_vertex_input_state(&vertex_input_info, &binding_description,
                           attribute_descriptions, attribute_count);

  // Other pipeline states
  VkPipelineInputAssemblyStateCreateInfo input_assembly;
  setup_input_assembly(&input_assembly);

  VkViewport viewport;
  VkRect2D scissor;
  VkPipelineViewportStateCreateInfo viewport_state;
  setup_viewport_state(&viewport_state, &viewport, &scissor,
                       &swapchain->extent);

  VkPipelineRasterizationStateCreateInfo rasterizer;
  setup_rasterization_state(&rasterizer);

  VkPipelineMultisampleStateCreateInfo multisampling;
  setup_multisampling(&multisampling);

  VkPipelineColorBlendAttachmentState color_blend_attachment;
  VkPipelineColorBlendStateCreateInfo color_blending;
  setup_color_blend_state(&color_blending, &color_blend_attachment);

  // Create pipeline layout
  VkPipelineLayoutCreateInfo pipeline_layout_info = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .setLayoutCount = 1,
      .pSetLayouts = &pipeline->descriptor_set_layout,
      .pushConstantRangeCount = 0,
      .pPushConstantRanges = NULL};

  if (vkCreatePipelineLayout(context->device, &pipeline_layout_info, NULL,
                             &pipeline->layout) != VK_SUCCESS) {
    vulkano_destroy_shader_module(context, vert_shader_module);
    vulkano_destroy_shader_module(context, frag_shader_module);
    vulkano_destroy_pipeline(context, pipeline);
    return VULKANO_ERROR_INIT_FAILED;
  }

  // Create the graphics pipeline
  VkGraphicsPipelineCreateInfo pipeline_info = {
      .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
      .stageCount = 2,
      .pStages = shader_stages,
      .pVertexInputState = &vertex_input_info,
      .pInputAssemblyState = &input_assembly,
      .pViewportState = &viewport_state,
      .pRasterizationState = &rasterizer,
      .pMultisampleState = &multisampling,
      .pDepthStencilState = NULL,
      .pColorBlendState = &color_blending,
      .pDynamicState = NULL,
      .layout = pipeline->layout,
      .renderPass = pipeline->render_pass,
      .subpass = 0,
      .basePipelineHandle = VK_NULL_HANDLE,
      .basePipelineIndex = -1};

  VkResult result =
      vkCreateGraphicsPipelines(context->device, VK_NULL_HANDLE, 1,
                                &pipeline_info, NULL, &pipeline->pipeline);

  // Clean up shader modules
  vulkano_destroy_shader_module(context, vert_shader_module);
  vulkano_destroy_shader_module(context, frag_shader_module);

  if (result != VK_SUCCESS) {
    vulkano_destroy_pipeline(context, pipeline);
    return VULKANO_ERROR_INIT_FAILED;
  }

  return VULKANO_SUCCESS;
}

VulkanoResult vulkano_create_descriptor_pool(VulkanoContext *context,
                                             VulkanoPipeline *pipeline,
                                             uint32_t max_frames) {
  VkDescriptorPoolSize pool_size = {.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                    .descriptorCount = max_frames};

  VkDescriptorPoolCreateInfo pool_info = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
      .poolSizeCount = 1,
      .pPoolSizes = &pool_size,
      .maxSets = max_frames};

  if (vkCreateDescriptorPool(context->device, &pool_info, NULL,
                             &pipeline->descriptor_pool) != VK_SUCCESS) {
    return VULKANO_ERROR_INIT_FAILED;
  }

  // Allocate descriptor sets
  pipeline->descriptor_sets = malloc(sizeof(VkDescriptorSet) * max_frames);
  if (!pipeline->descriptor_sets) {
    vkDestroyDescriptorPool(context->device, pipeline->descriptor_pool, NULL);
    pipeline->descriptor_pool = VK_NULL_HANDLE;
    return VULKANO_ERROR_INIT_FAILED;
  }

  VkDescriptorSetLayout layouts[max_frames];
  for (uint32_t i = 0; i < max_frames; i++) {
    layouts[i] = pipeline->descriptor_set_layout;
  }

  VkDescriptorSetAllocateInfo alloc_info = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
      .descriptorPool = pipeline->descriptor_pool,
      .descriptorSetCount = max_frames,
      .pSetLayouts = layouts};

  if (vkAllocateDescriptorSets(context->device, &alloc_info,
                               pipeline->descriptor_sets) != VK_SUCCESS) {
    free(pipeline->descriptor_sets);
    pipeline->descriptor_sets = NULL;
    vkDestroyDescriptorPool(context->device, pipeline->descriptor_pool, NULL);
    pipeline->descriptor_pool = VK_NULL_HANDLE;
    return VULKANO_ERROR_INIT_FAILED;
  }

  return VULKANO_SUCCESS;
}

void vulkano_destroy_pipeline(VulkanoContext *context,
                              VulkanoPipeline *pipeline) {
  if (!context || !pipeline) {
    return;
  }

  if (pipeline->descriptor_pool != VK_NULL_HANDLE) {
    vkDestroyDescriptorPool(context->device, pipeline->descriptor_pool, NULL);
  }

  if (pipeline->descriptor_set_layout != VK_NULL_HANDLE) {
    vkDestroyDescriptorSetLayout(context->device,
                                 pipeline->descriptor_set_layout, NULL);
  }

  if (pipeline->pipeline != VK_NULL_HANDLE) {
    vkDestroyPipeline(context->device, pipeline->pipeline, NULL);
  }

  if (pipeline->layout != VK_NULL_HANDLE) {
    vkDestroyPipelineLayout(context->device, pipeline->layout, NULL);
  }

  if (pipeline->render_pass != VK_NULL_HANDLE) {
    vkDestroyRenderPass(context->device, pipeline->render_pass, NULL);
  }

  free(pipeline->descriptor_sets);

  memset(pipeline, 0, sizeof(VulkanoPipeline));
}
