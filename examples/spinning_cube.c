#include <SDL3/SDL.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_vulkan.h>
#include <stdbool.h>
#include <stdio.h>
#include <vulkano.h>
#include <vulkano_renderer.h>

#ifndef VK_EXT_DEBUG_REPORT_EXTENSION_NAME
#define VK_EXT_DEBUG_REPORT_EXTENSION_NAME "VK_EXT_debug_report"
#endif

typedef struct {
  VulkanoContext context;
  VulkanoRenderer renderer;
  SDL_Window *window;
  int width;
  int height;
  bool should_close;
} App;

static bool create_window(App *app) {
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    printf("SDL_Init Error: %s\n", SDL_GetError());
    return false;
  }

  app->window =
      SDL_CreateWindow("Vulkano Spinning Cube", app->width, app->height,
                       SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);

  if (!app->window) {
    printf("Failed to create window: %s\n", SDL_GetError());
    return false;
  }

  return true;
}

static bool create_surface(App *app) {

  // // Now we can make the Vulkan instance
  // VkInstanceCreateInfo create_info = {};
  // create_info.enabledExtensionCount = count_extensions;
  // create_info.ppEnabledExtensionNames = extensions;

  // VkInstance instance;
  // VkResult result = vkCreateInstance(&create_info, NULL, &instance);

  if (!SDL_Vulkan_CreateSurface(app->window, app->context.instance, NULL,
                                &app->context.surface)) {
    printf("Failed to create Vulkan surface: %s\n", SDL_GetError());
    return false;
  }
  return true;
}

static void handle_events(App *app) {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
    case SDL_EVENT_QUIT:
      app->should_close = true;
      break;

    case SDL_EVENT_KEY_DOWN:
      if (event.key.key == SDLK_ESCAPE) {
        app->should_close = true;
      }
      break;

    case SDL_EVENT_WINDOW_RESIZED:
      app->width = event.window.data1;
      app->height = event.window.data2;
      // Handle resize if needed
      break;
    }
  }
}

int main(int argc, char *argv[]) {
  App app = {.width = 800, .height = 600, .should_close = false};

  // Create window
  if (!create_window(&app)) {
    return 1;
  }

  Uint32 count_instance_extensions;
  const char *const *instance_extensions =
      SDL_Vulkan_GetInstanceExtensions(&count_instance_extensions);

  if (instance_extensions == NULL) {
    printf("instance_extensions is null: %s\n", SDL_GetError());
    return false;
  }

  int count_extensions = count_instance_extensions + 1;
  const char **extensions = SDL_malloc(count_extensions * sizeof(const char *));
  extensions[0] = VK_EXT_DEBUG_REPORT_EXTENSION_NAME;
  SDL_memcpy(&extensions[1], instance_extensions,
             count_instance_extensions * sizeof(const char *));

  // Initialize Vulkan
  if (vulkano_init(&app.context, count_extensions, extensions) !=
      VULKANO_SUCCESS) {
    printf("Failed to initialize Vulkan\n");
    SDL_DestroyWindow(app.window);
    SDL_Quit();
    return 1;
  }
  SDL_free(extensions);

  // Create surface
  if (!create_surface(&app)) {
    vulkano_cleanup(&app.context);
    SDL_DestroyWindow(app.window);
    SDL_Quit();
    return 1;
  }

  // Create swapchain and initialize renderer
  if (vulkano_renderer_init(&app.context, &app.renderer) != VULKANO_SUCCESS) {
    printf("Failed to initialize renderer\n");
    vkDestroySurfaceKHR(app.context.instance, app.context.surface, NULL);
    vulkano_cleanup(&app.context);
    SDL_DestroyWindow(app.window);
    SDL_Quit();
    return 1;
  }

  // Main loop
  while (!app.should_close) {
    handle_events(&app);

    VulkanoResult result = vulkano_renderer_draw_frame(&app.renderer);
    if (result != VULKANO_SUCCESS) {
      if (result == VULKANO_ERROR_SWAPCHAIN_OUTDATED) {
        // Handle resize
        continue;
      }
      break;
    }
  }

  // Cleanup
  vulkano_renderer_wait_idle(&app.renderer);
  vulkano_renderer_cleanup(&app.renderer);
  vkDestroySurfaceKHR(app.context.instance, app.context.surface, NULL);
  vulkano_cleanup(&app.context);
  SDL_DestroyWindow(app.window);
  SDL_Quit();

  return 0;
}
