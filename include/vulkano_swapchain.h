#ifndef VULKANO_SWAPCHAIN_H
#define VULKANO_SWAPCHAIN_H

#include "vulkano.h"

typedef struct VulkanoSwapchain {
    VkSwapchainKHR swapchain;
    VkFormat format;
    VkExtent2D extent;
    uint32_t image_count;
    VkImage* images;
    VkImageView* image_views;
    VkFramebuffer* framebuffers;
} VulkanoSwapchain;

VulkanoResult vulkano_create_swapchain(VulkanoContext* context, VulkanoSwapchain* swapchain);
void vulkano_destroy_swapchain(VulkanoContext* context, VulkanoSwapchain* swapchain);
VulkanoResult vulkano_recreate_swapchain(VulkanoContext* context, VulkanoSwapchain* swapchain);

#endif // VULKANO_SWAPCHAIN_H