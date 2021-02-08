#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "model.h"
#include "geometry.h"
#include "vec.h"

bool model_parse(Model *model, const char *model_filename, const char *diffusemap_filename) {
    FILE *filep;
    char buffer[255];
    filep = fopen(model_filename, "r");
    if (!filep) {
        fprintf(stderr, "could not open file %s", model_filename);
        return false;
    }

    vec3f *vertices = vector_create();
    vec3i **faces = vector_create();
    vec3f *norms = vector_create();
    vec2f *uv = vector_create();

    while (fgets(buffer, sizeof(buffer), filep)) {
        if (!strncmp(buffer, "v ", 2)) {
            vec3f *temp = vector_add_asg(&vertices);
            char *start = buffer + 2;
            char *end;
            for (int i = 0; i < 3; i++) {
                *((float *)temp + i) = strtof(start, &end);
                start = end;
            }
            temp = NULL;

        } else if (!strncmp(buffer, "vn ", 3)) {
            vec3f *temp = vector_add_asg(&norms);
            char *start = buffer + 3;
            char *end;
            for (int i = 0; i < 3; i++) {
                *((float *)temp + i) = strtof(start, &end);
                start = end;
            }
            temp = NULL;

        } else if (!strncmp(buffer, "vt ", 3)) {
            vec2f *temp = vector_add_asg(&uv);
            char *start = buffer + 3;
            char *end;
            for (int i = 0; i < 2; i++) {
                *((float *)temp + i) = strtof(start, &end);
                start = end;
            }
            temp = NULL;

        } else if (!strncmp(buffer, "f ", 2)) {
            vec3i *face = vector_create();
            char *start = buffer + 2;
            char *end;
            for (int i = 0; i < 3; i++) {
                vec3i *temp = vector_add_asg(&face);
                for (int j = 0; j < 3; j++) {
                    *((int *)temp + j) = (int)strtol(start, &end, 10) - 1;
                    start = end + 1;
                }
            }
            vector_add(&faces, face);


            //vec_int face = vector_create();
            //
            //vector_add(&face, int, (int)strtol(buffer + 2, NULL, 10) - 1);
            //
            //char *pend = NULL;
            //char *pchar = strchr(buffer + 2, ' ');
            //vector_add(&face, int, (int)strtol(pchar, &pend, 10) - 1);
            //
            //char *pchar2 = strchr(pend, ' ');
            //
            //vector_add(&face, int, (int)strtol(pchar2, NULL, 10) - 1);
            //vector_add(&faces, vec_int, face);
        }
    }

   
    printf(
        "#%s v#%d f#%d vt#%d vn#%d\n",
        model_filename,
        vector_size(vertices),
        vector_size(faces),
        vector_size(uv),
        vector_size(norms)
    );

    model->vertices = vertices;
    model->faces = faces;
    model->norms = norms;
    model->uv = uv;

    fclose(filep);

    if (diffusemap_filename) {
        tga_image *diffusemap = malloc(sizeof(tga_image));
        image_read(diffusemap, diffusemap_filename);
        image_flip_vertically(diffusemap);
        model->diffusemap = diffusemap;
    } else {
        model->diffusemap = NULL;
    }

    return true;
}

vec3i model_face(Model *model, int index) {
    vec3i face;
    for (int i = 0; i < 3; i++) {
        *((int *)&face + i) = model->faces[index][i].raw[0];
    }
    return face;
}

vec2i model_uv(Model *model, int nface, int nvert) {
    int idx = model->faces[nface][nvert].raw[1];
    
    return (vec2i){model->uv[idx].x * model->diffusemap->width, model->uv[idx].y * model->diffusemap->height};
}

tga_color model_diffuse(Model *model, vec2i uv) {
    return image_get_color(model->diffusemap, uv.x, uv.y);
}

int model_nverts(Model *model) {
    return vector_size(model->vertices);
}

int model_nfaces(Model *model) {
    return vector_size(model->faces);
}

void model_free(Model *model) {
    vector_free(model->vertices);
    for (int i = 0; i < vector_size(model->faces); i++) {
        vector_free(model->faces[i]);
    }
    vector_free(model->faces);
    vector_free(model->norms);
    vector_free(model->uv);

    if (model->diffusemap) {
        image_free(model->diffusemap);
    }

    free(model);
}
