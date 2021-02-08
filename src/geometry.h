#ifndef GEOMETRY_H
#define GEOMETRY_H

float d_sqrt(float number);

typedef struct vec3f {
    union {
        struct {
            float x, y, z;
        };
        float raw[3];
    };
} vec3f;

void vec_sub3f(float *output, float *a, float *b);

void vec_xor3f(float *output, float *a, float *b);

void vec_cross3f(float *output, float *a, float *b);

void vec_normalize3f(float *vec);

float vec_dot3f(float *a, float *b);

typedef struct vec2f {
    union {
        struct {
            float x, y;
        };
        float raw[2];
    };
} vec2f;

typedef struct vec3i{
    union {
        struct {
            int x, y, z;
        };
        int raw[3];
    };
} vec3i;

void vec_cross3i(int *output, int *a, int *b);

typedef struct vec2i {
    union {
        struct {
            int x, y;
        };
        int raw[2];
    };
} vec2i;

#endif // GEOMETRY_H
