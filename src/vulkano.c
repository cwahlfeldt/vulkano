#include "vulkano.h"
#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan_core.h>

#define VULKANO_MAX_PHYSICAL_DEVICES 16

static VkResult create_vulkan_instance(VulkanoContext *context) {
  VkApplicationInfo app_info = {.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                                .pNext = NULL,
                                .pApplicationName = "Vulkano Renderer",
                                .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
                                .pEngineName = "Vulkano",
                                .engineVersion = VK_MAKE_VERSION(1, 0, 0),
                                .apiVersion = VK_API_VERSION_1_1};

  VkInstanceCreateInfo create_info = {
      .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      .pNext = NULL,
      .flags = 0,
      .pApplicationInfo = &app_info,
      .enabledLayerCount = 0,
      .ppEnabledLayerNames = NULL,
      .enabledExtensionCount = 0,
      .ppEnabledExtensionNames = NULL};

  return vkCreateInstance(&create_info, NULL, &context->instance);
}

VulkanoResult vulkano_init(VulkanoContext *context) {
  if (!context) {
    return VULKANO_ERROR_INIT_FAILED;
  }

  // Clear context
  memset(context, 0, sizeof(VulkanoContext));

  // Create Vulkan instance
  VkResult result = create_vulkan_instance(context);
  if (result != VK_SUCCESS) {
    return VULKANO_ERROR_VULKAN_UNAVAILABLE;
  }

  // Select physical device
  result = vulkano_select_physical_device(context);
  if (result != VULKANO_SUCCESS) {
    vulkano_cleanup(context);
    return result;
  }

  // Create logical device
  result = vulkano_create_logical_device(context);
  if (result != VULKANO_SUCCESS) {
    vulkano_cleanup(context);
    return result;
  }

  context->initialized = true;
  return VULKANO_SUCCESS;
}

VulkanoResult vulkano_select_physical_device(VulkanoContext *context) {
  uint32_t device_count = 0;
  VkPhysicalDevice devices[VULKANO_MAX_PHYSICAL_DEVICES];

  VkResult result =
      vkEnumeratePhysicalDevices(context->instance, &device_count, NULL);
  if (result != VK_SUCCESS || device_count == 0) {
    return VULKANO_ERROR_VULKAN_UNAVAILABLE;
  }

  if (device_count > VULKANO_MAX_PHYSICAL_DEVICES) {
    device_count = VULKANO_MAX_PHYSICAL_DEVICES;
  }

  result =
      vkEnumeratePhysicalDevices(context->instance, &device_count, devices);
  if (result != VK_SUCCESS) {
    return VULKANO_ERROR_VULKAN_UNAVAILABLE;
  }

  // For now, just select the first device
  // TODO: Implement proper device selection based on capabilities
  context->physical_device = devices[0];

  return VULKANO_SUCCESS;
}

VulkanoResult vulkano_create_logical_device(VulkanoContext *context) {
  // Find a graphics queue family
  uint32_t queue_family_count = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(context->physical_device,
                                           &queue_family_count, NULL);

  VkQueueFamilyProperties *queue_families =
      malloc(sizeof(VkQueueFamilyProperties) * queue_family_count);
  vkGetPhysicalDeviceQueueFamilyProperties(context->physical_device,
                                           &queue_family_count, queue_families);

  uint32_t graphics_queue_family = UINT32_MAX;
  for (uint32_t i = 0; i < queue_family_count; i++) {
    if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      graphics_queue_family = i;
      break;
    }
  }

  free(queue_families);

  if (graphics_queue_family == UINT32_MAX) {
    return VULKANO_ERROR_VULKAN_UNAVAILABLE;
  }

  // Create logical device
  float queue_priority = 1.0f;
  VkDeviceQueueCreateInfo queue_create_info = {
      .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
      .queueFamilyIndex = graphics_queue_family,
      .queueCount = 1,
      .pQueuePriorities = &queue_priority};

  VkDeviceCreateInfo device_create_info = {
      .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
      .queueCreateInfoCount = 1,
      .pQueueCreateInfos = &queue_create_info,
      .enabledExtensionCount = 0,
      .ppEnabledExtensionNames = NULL,
      .pEnabledFeatures = NULL};

  VkResult result = vkCreateDevice(context->physical_device,
                                   &device_create_info, NULL, &context->device);
  if (result != VK_SUCCESS) {
    return VULKANO_ERROR_VULKAN_UNAVAILABLE;
  }

  // Get graphics queue
  vkGetDeviceQueue(context->device, graphics_queue_family, 0,
                   &context->graphics_queue);

  return VULKANO_SUCCESS;
}

void vulkano_cleanup(VulkanoContext *context) {
  if (!context) {
    return;
  }

  if (context->device != VK_NULL_HANDLE) {
    vkDestroyDevice(context->device, NULL);
    context->device = VK_NULL_HANDLE;
  }

  if (context->surface != VK_NULL_HANDLE) {
    vkDestroySurfaceKHR(context->instance, context->surface, NULL);
    context->surface = VK_NULL_HANDLE;
  }

  if (context->instance != VK_NULL_HANDLE) {
    vkDestroyInstance(context->instance, NULL);
    context->instance = VK_NULL_HANDLE;
  }

  context->initialized = false;
}
