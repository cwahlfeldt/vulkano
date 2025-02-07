// include/vulkano_cube.h
#ifndef VULKANO_CUBE_H
#define VULKANO_CUBE_H

#include "vulkano_vertex.h"

#define CUBE_VERTICES_COUNT 8
#define CUBE_INDICES_COUNT 36

static const Vertex cube_vertices[CUBE_VERTICES_COUNT] = {
    {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}}, // 0: Front-bottom-left
    {{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},  // 1: Front-bottom-right
    {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}},   // 2: Front-top-right
    {{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 0.0f}},  // 3: Front-top-left
    {{-0.5f, -0.5f, 0.5f}, {1.0f, 0.0f, 1.0f}},  // 4: Back-bottom-left
    {{0.5f, -0.5f, 0.5f}, {0.0f, 1.0f, 1.0f}},   // 5: Back-bottom-right
    {{0.5f, 0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}},    // 6: Back-top-right
    {{-0.5f, 0.5f, 0.5f}, {0.5f, 0.5f, 0.5f}}    // 7: Back-top-left
};

static const uint16_t cube_indices[CUBE_INDICES_COUNT] = {
    // Front face
    0, 1, 2, 2, 3, 0,
    // Right face
    1, 5, 6, 6, 2, 1,
    // Back face
    5, 4, 7, 7, 6, 5,
    // Left face
    4, 0, 3, 3, 7, 4,
    // Top face
    3, 2, 6, 6, 7, 3,
    // Bottom face
    4, 5, 1, 1, 0, 4};

#endif // VULKANO_CUBE_H
