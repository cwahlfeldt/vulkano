CC := gcc
CFLAGS := -std=c11 -Wall -Wextra -I./include -Wno-unused-parameter -Wno-enum-conversion -Wno-enum-compare
LDFLAGS := -lvulkan -lm

# CFLAGS += -fsanitize=address -g
# LDFLAGS += -fsanitize=address

SDL_DIR := lib/SDL
SDL_INCLUDE := $(SDL_DIR)/include
SDL_LIB := $(SDL_DIR)/build/libSDL3.a

# VK_DIR := lib/Vulkan-ValidationLayers
# # _INCLUDE := $(SDL_DIR)/include
# VK_LIB := $(VK_DIR)/build/layers/libVkLayer_utils.a

CFLAGS += -I$(SDL_INCLUDE) -Ilib/VulkanMemoryAllocator/include
LDFLAGS += $(SDL_LIB) -pthread

GLSLC := glslc
SHADER_SRC := $(wildcard shaders/*.vert) $(wildcard shaders/*.frag)
SHADER_SPV := $(SHADER_SRC:%=%.spv)

SRC_DIR := src
BUILD_DIR := build
INCLUDE_DIR := include

SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

EXAMPLES_DIR := examples
EXAMPLES_BUILD_DIR := $(BUILD_DIR)/examples
EXAMPLE_SRCS := examples/spinning_cube.c
EXAMPLE_BINS := $(EXAMPLES_BUILD_DIR)/spinning_cube

# Platform-specific settings
ifeq ($(OS),Windows_NT)
    CFLAGS += -DVK_USE_PLATFORM_WIN32_KHR
    LDFLAGS += -lgdi32
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
		ifneq ($(shell pkg-config --exists wayland-client && echo yes),)
			CFLAGS += -DVK_USE_PLATFORM_WAYLAND_KHR
			LDFLAGS += -lwayland-client
		else
			CFLAGS += -DVK_USE_PLATFORM_XCB_KHR
			LDFLAGS += -lxcb -lX11 -lX11-xcb
    	endif
    endif
    ifeq ($(UNAME_S),Darwin)
        CFLAGS += -DVK_USE_PLATFORM_METAL_EXT
        LDFLAGS += -framework Metal -framework Foundation -framework QuartzCore
    endif
endif

.PHONY: all clean shaders examples

all: shaders $(BUILD_DIR)/libvulkano.a examples

examples: $(BUILD_DIR)/libvulkano.a
	@mkdir -p $(EXAMPLES_BUILD_DIR)
	$(CC) $(CFLAGS) examples/spinning_cube.c -o $(EXAMPLES_BUILD_DIR)/spinning_cube -L$(BUILD_DIR) -lvulkano $(LDFLAGS)
	@echo "EXAMPLES_DIR: $(EXAMPLES_DIR)"
	@echo "EXAMPLE_SRCS: $(EXAMPLE_SRCS)"
	@echo "EXAMPLE_BINS: $(EXAMPLE_BINS)"
	@echo "EXAMPLES_BUILD_DIR: $(EXAMPLES_BUILD_DIR)"

$(EXAMPLES_BUILD_DIR):
	@mkdir -p $(EXAMPLES_BUILD_DIR)

$(EXAMPLES_BUILD_DIR)/%: $(EXAMPLES_DIR)/%.c
	@mkdir -p $(EXAMPLES_BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@ -L$(BUILD_DIR) -lvulkano $(LDFLAGS)

shaders: $(SHADER_SPV)

%.spv: %
	$(GLSLC) $< -o $@

$(BUILD_DIR)/libvulkano.a: $(OBJS)
	ar rcs $@ $^

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)
	rm -f $(SHADER_SPV)
