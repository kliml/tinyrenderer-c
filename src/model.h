#ifndef MODEL_H
#define MODEL_H

#include "geometry.h"
#include "vec.h"
#include "tgaimage.h"

typedef struct {
    vec3f *vertices;
    vec3i **faces;
    vec3f *norms;
    vec2f *uv;
    tga_image *diffusemap;
} Model;

bool model_parse(Model *model, const char *model_filename, const char *diffusemap_filename);

vec3i model_face(Model *model, int index);

vec2i model_uv(Model *model, int nface, int nvert);

tga_color model_diffuse(Model *model, vec2i uv);

int model_nverts(Model *model);

int model_nfaces(Model *model);

void model_free(Model *model);

#endif // MODEL_H
