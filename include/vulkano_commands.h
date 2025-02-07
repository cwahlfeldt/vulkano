// src/vulkano_commands.h
#ifndef VULKANO_COMMANDS_H
#define VULKANO_COMMANDS_H

#include "vulkano.h"
#include "vulkano_renderer.h"

VulkanoResult vulkano_create_command_buffers(VulkanoRenderer *renderer);
void vulkano_destroy_command_buffers(VulkanoRenderer *renderer);
VulkanoResult vulkano_record_command_buffer(VulkanoRenderer *renderer,
                                            uint32_t image_index);

#endif // VULKANO_COMMANDS_H
