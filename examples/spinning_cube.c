#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <vulkano.h>
#include <vulkano_renderer.h>

typedef struct {
  VulkanoContext context;
  VulkanoRenderer renderer;
  SDL_Window *window;
  int width;
  int height;
  bool should_close;
} App;

static bool init_vulkan(App *app) {
  // Add required extensions for Wayland/XCB
  const char *extensions[] = {
      VK_KHR_SURFACE_EXTENSION_NAME,
#ifdef VK_USE_PLATFORM_XCB_KHR
      VK_KHR_XCB_SURFACE_EXTENSION_NAME,
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
      VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME,
#endif
      VK_EXT_DEBUG_REPORT_EXTENSION_NAME // Add debug extension
  };
  uint32_t extension_count = sizeof(extensions) / sizeof(extensions[0]);

  // Get SDL's required extensions count
  uint32_t sdl_extension_count = 0;
  if (!SDL_Vulkan_GetInstanceExtensions(&sdl_extension_count)) {
    printf("Failed to get SDL extension count: %s\n", SDL_GetError());
    return false;
  }

  printf("Found %u platform extensions and %u SDL extensions\n",
         extension_count, sdl_extension_count);

  // Initialize Vulkan with our extensions
  if (vulkano_init(&app->context, extensions, extension_count) !=
      VULKANO_SUCCESS) {
    printf("Failed to initialize Vulkan\n");
    return false;
  }

  return true;
}

static bool create_window(App *app) {
  // Set hints before creating window to minimize allocations
  SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, "0");
  SDL_SetHint(SDL_HINT_VIDEO_WAYLAND_ALLOW_LIBDECOR, "0");
  SDL_SetHint(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "0");

  // Only initialize video subsystem
  SDL_InitSubSystem(SDL_INIT_VIDEO);

  app->window = SDL_CreateWindow(
      "Vulkano Spinning Cube", app->width, app->height,
      SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_MINIMIZED);

  if (!app->window) {
    printf("Failed to create window: %s\n", SDL_GetError());
    return false;
  }

  return true;
}

static bool create_surface(App *app) {
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
      // TODO: Handle resize
      break;
    }
  }
}

static void cleanup_sdl(void) { SDL_Quit(); }

int main(int argc, char *argv[]) {
  (void)argc;
  (void)argv;

  atexit(cleanup_sdl);

  App app = {.width = 800,
             .height = 600,
             .should_close = false,
             .window = NULL,
             .context = {0}};

  // Initialize SDL with only video subsystem
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    printf("SDL_Init Error: %s\n", SDL_GetError());
    return 1;
  }

  // Disable unnecessary SDL subsystems
  // SDL_EventState(SDL_EVENT_TEXT_INPUT, SDL_DISABLE);
  // SDL_EventState(SDL_EVENT_TEXT_EDITING, SDL_DISABLE);
  // SDL_EventState(SDL_EVENT_TEXT_EDITING_EXT, SDL_DISABLE);

  // Create window
  if (!create_window(&app)) {
    SDL_Quit();
    return 1;
  }

  // Initialize Vulkan
  if (!init_vulkan(&app)) {
    SDL_DestroyWindow(app.window);
    SDL_Quit();
    return 1;
  }

  // Create surface
  if (!create_surface(&app)) {
    vulkano_cleanup(&app.context);
    SDL_DestroyWindow(app.window);
    SDL_Quit();
    return 1;
  }

  // Initialize renderer
  if (vulkano_renderer_init(&app.context, &app.renderer) != VULKANO_SUCCESS) {
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
        // TODO: Handle resize
        continue;
      }
      break;
    }
  }

  // Cleanup everything
  vulkano_renderer_wait_idle(&app.renderer);
  vulkano_renderer_cleanup(&app.renderer);
  vkDestroySurfaceKHR(app.context.instance, app.context.surface, NULL);
  vulkano_cleanup(&app.context);
  SDL_DestroyWindow(app.window);
  SDL_Quit();

  return 0;
}