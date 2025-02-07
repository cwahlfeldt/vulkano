// src/vulkano_commands.h
#ifndef VULKANO_MATH_H
#define VULKANO_MATH_H

#include <math.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static void matrix_identity(float *m) {
  memset(m, 0, 16 * sizeof(float));
  m[0] = 1.0f;
  m[5] = 1.0f;
  m[10] = 1.0f;
  m[15] = 1.0f;
}

static void matrix_perspective(float *m, float fovy, float aspect, float near,
                               float far) {
  float f = 1.0f / tanf(fovy * 0.5f);

  m[0] = f / aspect;
  m[5] = f;
  m[10] = (far + near) / (near - far);
  m[11] = -1.0f;
  m[14] = (2.0f * far * near) / (near - far);
  m[15] = 0.0f;
}

static void matrix_rotation_y(float *m, float angle) {
  float c = cosf(angle);
  float s = sinf(angle);

  matrix_identity(m);
  m[0] = c;
  m[2] = -s;
  m[8] = s;
  m[10] = c;
}
#endif
