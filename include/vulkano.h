#ifndef VULKANO_H
#define VULKANO_H

#include <stdbool.h>
#include <stdint.h>
#include <vulkan/vulkan.h>

// Platform-specific includes
#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#include <windows.h>
#elif defined(__APPLE__)
#define VK_USE_PLATFORM_METAL_EXT
#include <Metal/Metal.h>
#elif defined(__ANDROID__)
#define VK_USE_PLATFORM_ANDROID_KHR
#include <android/native_window.h>
#else // Linux
#include <xcb/xcb.h>
#endif

// Error codes
typedef enum VulkanoResult {
  VULKANO_SUCCESS = 0,
  VULKANO_ERROR_INIT_FAILED = -1,
  VULKANO_ERROR_VULKAN_UNAVAILABLE = -2,
  VULKANO_ERROR_DEVICE_LOST = -3,
  VULKANO_ERROR_SWAPCHAIN_OUTDATED = -4,
} VulkanoResult;

// Core renderer context
typedef struct VulkanoContext {
  VkInstance instance;
  VkPhysicalDevice physical_device;
  VkDevice device;
  VkQueue graphics_queue;
  VkCommandPool command_pool;
  VkSurfaceKHR surface;
  bool initialized;
} VulkanoContext;

// Initialization
VulkanoResult vulkano_init(VulkanoContext *context, int count_extensions,
                           const char **extensions);

// Cleanup
void vulkano_cleanup(VulkanoContext *context);

// Device management
VulkanoResult vulkano_select_physical_device(VulkanoContext *context);
VulkanoResult vulkano_create_logical_device(VulkanoContext *context);

// Surface management
VulkanoResult vulkano_create_surface(VulkanoContext *context,
                                     void *native_window);

#endif // VULKANO_H
