#include "vulkano_swapchain.h"
#include <stdlib.h>
#include <string.h>

static VkSurfaceFormatKHR choose_swap_surface_format(VkSurfaceFormatKHR* formats, uint32_t format_count) {
    for (uint32_t i = 0; i < format_count; i++) {
        if (formats[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
            formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return formats[i];
        }
    }
    return formats[0];
}

static VkPresentModeKHR choose_swap_present_mode(VkPresentModeKHR* present_modes, uint32_t present_mode_count) {
    for (uint32_t i = 0; i < present_mode_count; i++) {
        if (present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
            return present_modes[i];
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

static VkExtent2D choose_swap_extent(const VkSurfaceCapabilitiesKHR* capabilities, uint32_t width, uint32_t height) {
    if (capabilities->currentExtent.width != UINT32_MAX) {
        return capabilities->currentExtent;
    } else {
        VkExtent2D extent = {width, height};
        
        extent.width = (extent.width > capabilities->maxImageExtent.width) ? 
            capabilities->maxImageExtent.width : extent.width;
        extent.width = (extent.width < capabilities->minImageExtent.width) ? 
            capabilities->minImageExtent.width : extent.width;
            
        extent.height = (extent.height > capabilities->maxImageExtent.height) ? 
            capabilities->maxImageExtent.height : extent.height;
        extent.height = (extent.height < capabilities->minImageExtent.height) ? 
            capabilities->minImageExtent.height : extent.height;
            
        return extent;
    }
}

VulkanoResult vulkano_create_swapchain(VulkanoContext* context, VulkanoSwapchain* swapchain) {
    // Query surface capabilities
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(context->physical_device, context->surface, &capabilities);

    // Query supported surface formats
    uint32_t format_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(context->physical_device, context->surface, &format_count, NULL);
    VkSurfaceFormatKHR* formats = malloc(sizeof(VkSurfaceFormatKHR) * format_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(context->physical_device, context->surface, &format_count, formats);

    // Query supported presentation modes
    uint32_t present_mode_count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(context->physical_device, context->surface, &present_mode_count, NULL);
    VkPresentModeKHR* present_modes = malloc(sizeof(VkPresentModeKHR) * present_mode_count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(context->physical_device, context->surface, &present_mode_count, present_modes);

    // Choose swap chain settings
    VkSurfaceFormatKHR surface_format = choose_swap_surface_format(formats, format_count);
    VkPresentModeKHR present_mode = choose_swap_present_mode(present_modes, present_mode_count);
    VkExtent2D extent = choose_swap_extent(&capabilities, 800, 600); // TODO: Get actual window size

    // Choose image count
    uint32_t image_count = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && image_count > capabilities.maxImageCount) {
        image_count = capabilities.maxImageCount;
    }

    // Create swap chain
    VkSwapchainCreateInfoKHR create_info = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = context->surface,
        .minImageCount = image_count,
        .imageFormat = surface_format.format,
        .imageColorSpace = surface_format.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .preTransform = capabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = present_mode,
        .clipped = VK_TRUE,
        .oldSwapchain = VK_NULL_HANDLE
    };

    if (vkCreateSwapchainKHR(context->device, &create_info, NULL, &swapchain->swapchain) != VK_SUCCESS) {
        free(formats);
        free(present_modes);
        return VULKANO_ERROR_INIT_FAILED;
    }

    // Get swap chain images
    vkGetSwapchainImagesKHR(context->device, swapchain->swapchain, &image_count, NULL);
    swapchain->images = malloc(sizeof(VkImage) * image_count);
    vkGetSwapchainImagesKHR(context->device, swapchain->swapchain, &image_count, swapchain->images);

    swapchain->format = surface_format.format;
    swapchain->extent = extent;
    swapchain->image_count = image_count;

    // Create image views
    swapchain->image_views = malloc(sizeof(VkImageView) * image_count);
    swapchain->framebuffers = malloc(sizeof(VkFramebuffer) * image_count);

    for (uint32_t i = 0; i < image_count; i++) {
        VkImageViewCreateInfo view_info = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = swapchain->images[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = surface_format.format,
            .components.r = VK_COMPONENT_SWIZZLE_IDENTITY,
            .components.g = VK_COMPONENT_SWIZZLE_IDENTITY,
            .components.b = VK_COMPONENT_SWIZZLE_IDENTITY,
            .components.a = VK_COMPONENT_SWIZZLE_IDENTITY,
            .subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .subresourceRange.baseMipLevel = 0,
            .subresourceRange.levelCount = 1,
            .subresourceRange.baseArrayLayer = 0,
            .subresourceRange.layerCount = 1
        };

        if (vkCreateImageView(context->device, &view_info, NULL, &swapchain->image_views[i]) != VK_SUCCESS) {
            vulkano_destroy_swapchain(context, swapchain);
            free(formats);
            free(present_modes);
            return VULKANO_ERROR_INIT_FAILED;
        }
    }

    free(formats);
    free(present_modes);
    return VULKANO_SUCCESS;
}

void vulkano_destroy_swapchain(VulkanoContext* context, VulkanoSwapchain* swapchain) {
    if (!context || !swapchain) {
        return;
    }

    for (uint32_t i = 0; i < swapchain->image_count; i++) {
        if (swapchain->framebuffers && swapchain->framebuffers[i] != VK_NULL_HANDLE) {
            vkDestroyFramebuffer(context->device, swapchain->framebuffers[i], NULL);
        }
        if (swapchain->image_views && swapchain->image_views[i] != VK_NULL_HANDLE) {
            vkDestroyImageView(context->device, swapchain->image_views[i], NULL);
        }
    }

    free(swapchain->framebuffers);
    free(swapchain->image_views);
    free(swapchain->images);
    
    vkDestroySwapchainKHR(context->device, swapchain->swapchain, NULL);
    
    swapchain->swapchain = VK_NULL_HANDLE;
    swapchain->images = NULL;
    swapchain->image_views = NULL;
    swapchain->framebuffers = NULL;
    swapchain->image_count = 0;
}

VulkanoResult vulkano_recreate_swapchain(VulkanoContext* context, VulkanoSwapchain* swapchain) {
    vkDeviceWaitIdle(context->device);
    
    VulkanoSwapchain new_swapchain = {0};
    VulkanoResult result = vulkano_create_swapchain(context, &new_swapchain);
    
    if (result == VULKANO_SUCCESS) {
        vulkano_destroy_swapchain(context, swapchain);
        *swapchain = new_swapchain;
    }
    
    return result;
}