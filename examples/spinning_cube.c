#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <vulkano.h>
#include <vulkano_renderer.h>

#ifdef VK_USE_PLATFORM_XCB_KHR
#include <string.h>
#include <xcb/xcb.h>
#endif

typedef struct {
  VulkanoContext context;
  VulkanoRenderer renderer;

#ifdef VK_USE_PLATFORM_XCB_KHR
  xcb_connection_t *connection;
  xcb_window_t window;
  xcb_screen_t *screen;
#endif

  int width;
  int height;
  bool should_close;
} App;

static void create_window(App *app) {
#ifdef VK_USE_PLATFORM_XCB_KHR
  app->connection = xcb_connect(NULL, NULL);
  if (xcb_connection_has_error(app->connection)) {
    printf("Failed to connect to X server\n");
    exit(1);
  }

  // Get the first screen
  app->screen = xcb_setup_roots_iterator(xcb_get_setup(app->connection)).data;
  app->window = xcb_generate_id(app->connection);

  uint32_t value_mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
  uint32_t value_list[2];
  value_list[0] = app->screen->black_pixel;
  value_list[1] = XCB_EVENT_MASK_KEY_RELEASE | XCB_EVENT_MASK_EXPOSURE |
                  XCB_EVENT_MASK_STRUCTURE_NOTIFY;

  xcb_create_window(app->connection, XCB_COPY_FROM_PARENT, app->window,
                    app->screen->root, 0, 0, app->width, app->height, 0,
                    XCB_WINDOW_CLASS_INPUT_OUTPUT, app->screen->root_visual,
                    value_mask, value_list);

  // Set window title
  xcb_change_property(app->connection, XCB_PROP_MODE_REPLACE, app->window,
                      XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8,
                      strlen("Vulkano Spinning Cube"), "Vulkano Spinning Cube");

  xcb_map_window(app->connection, app->window);
  xcb_flush(app->connection);
#endif
}

static void create_surface(App *app) {
#ifdef VK_USE_PLATFORM_XCB_KHR
  VkXcbSurfaceCreateInfoKHR create_info = {
      .sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR,
      .connection = app->connection,
      .window = app->window};

  if (vkCreateXcbSurfaceKHR(app->context.instance, &create_info, NULL,
                            &app->context.surface) != VK_SUCCESS) {
    printf("Failed to create window surface\n");
    exit(1);
  }
#endif
}

static void handle_events(App *app) {
#ifdef VK_USE_PLATFORM_XCB_KHR
  xcb_generic_event_t *event;
  while ((event = xcb_poll_for_event(app->connection))) {
    switch (event->response_type & 0x7f) {
    case XCB_CLIENT_MESSAGE:
      app->should_close = true;
      break;
    case XCB_KEY_RELEASE:
      app->should_close = true;
      break;
    case XCB_DESTROY_NOTIFY:
      app->should_close = true;
      break;
    case XCB_CONFIGURE_NOTIFY: {
      xcb_configure_notify_event_t *cfg = (xcb_configure_notify_event_t *)event;
      if (cfg->width != app->width || cfg->height != app->height) {
        app->width = cfg->width;
        app->height = cfg->height;
        // Handle resize if needed
      }
      break;
    }
    }
    free(event);
  }
#endif
}

int main() {
  App app = {.width = 800, .height = 600, .should_close = false};

  // Create window
  create_window(&app);

  // Initialize Vulkan
  if (vulkano_init(&app.context) != VULKANO_SUCCESS) {
    printf("Failed to initialize Vulkan\n");
    return 1;
  }

  // Create surface
  create_surface(&app);

  // Create swapchain and initialize renderer
  if (vulkano_renderer_init(&app.context, &app.renderer) != VULKANO_SUCCESS) {
    printf("Failed to initialize renderer\n");
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

#ifdef VK_USE_PLATFORM_XCB_KHR
  xcb_destroy_window(app.connection, app.window);
  xcb_disconnect(app.connection);
#endif

  return 0;
}
