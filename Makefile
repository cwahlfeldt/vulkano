CC := gcc
CFLAGS := -std=c11 -Wall -Wextra -I./include
LDFLAGS := -lvulkan -lm

# Platform-specific settings
ifeq ($(OS),Windows_NT)
    CFLAGS += -DVK_USE_PLATFORM_WIN32_KHR
    LDFLAGS += -lgdi32
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        CFLAGS += -DVK_USE_PLATFORM_XCB_KHR
        LDFLAGS += -lxcb -lX11 -lX11-xcb
    endif
    ifeq ($(UNAME_S),Darwin)
        CFLAGS += -DVK_USE_PLATFORM_METAL_EXT
        LDFLAGS += -framework Metal -framework Foundation -framework QuartzCore
    endif
endif

GLSLC := glslc
SHADER_SRC := $(wildcard shaders/*.vert) $(wildcard shaders/*.frag)
SHADER_SPV := $(SHADER_SRC:%=%.spv)

# Examples

# EXAMPLES := $(wildcard $(EXAMPLES_DIR)/*)
# EXAMPLE_BUILDS := $(EXAMPLES:%=%/bin)

# Platform-specific settings
ifeq ($(OS),Windows_NT)
    CFLAGS += -DVK_USE_PLATFORM_WIN32_KHR
    LDFLAGS += -lgdi32
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        CFLAGS += -DVK_USE_PLATFORM_XCB_KHR
        LDFLAGS += -lxcb
    endif
    ifeq ($(UNAME_S),Darwin)
        CFLAGS += -DVK_USE_PLATFORM_METAL_EXT
        LDFLAGS += -framework Metal -framework Foundation -framework QuartzCore
    endif
endif

SRC_DIR := src
BUILD_DIR := build
INCLUDE_DIR := include

SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

EXAMPLES_DIR := examples
# EXAMPLE_SRCS := $(wildcard $(EXAMPLES_DIR)/*.c)
EXAMPLE_SRCS := $(wildcard $(EXAMPLES_DIR)/*.c)
EXAMPLE_BINS := $(EXAMPLE_SRCS:$(EXAMPLES_DIR)/%.c=$(BUILD_DIR)/$(EXAMPLES_DIR)/%)

.PHONY: all clean shaders examples

all: shaders $(BUILD_DIR)/libvulkano.a examples

examples: $(EXAMPLE_BINS)

# Rule to compile each example source file into an executable
$(BUILD_DIR)/$(EXAMPLES_DIR)/%: $(EXAMPLES_DIR)/%.c $(BUILD_DIR)/libvulkano.a
	@mkdir -p $(dir $@)
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
