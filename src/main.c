#include <float.h>
#include <math.h>

#include "tgaimage.h"
#include "model.h"

typedef unsigned int uint;

const int width = 800;
const int height = 800;

Model *model;

void draw_line(int x0, int y0, int x1, int y1, tga_image *image, tga_color color) {
    bool steep = false;
    if (abs(x0 - x1) < abs(y0 - y1)) {
        int temp = x0;
        x0 = y0;
        y0 = temp;
        temp = x1;
        x1 = y1;
        y1 = temp;
        steep = true;
    }
    if (x0 > x1) {
        int temp = x0;
        x0 = x1;
        x1 = temp;
        temp = y0;
        y0 = y1;
        y1 = temp;
    }
    int dx = x1 - x0;
    int dy = y1 - y0;
    int derror2 = abs(dy) * 2;
    int error2 = 0;
    int y = y0;

    for (int x = x0; x <= x1; x++) {
        if (steep) {
            image_set_color(image, y, x, color);
        }
        else {
            image_set_color(image, x, y, color);
        }
        error2 += derror2;
        if (error2 > dx) {
            y += (y1 > y0 ? 1 : -1);
            error2 -= dx * 2;
        }
    }
}

void barycentric(vec3f *output, vec3f A, vec3f B, vec3f C, vec3f P) {
    vec3f s[2];
    for (int i = 2; i--;) {
        s[i].x = C.raw[i] - A.raw[i];
        s[i].y = B.raw[i] - A.raw[i];
        s[i].z = A.raw[i] - P.raw[i];
    }
    vec3f u;
    vec_cross3f(&u, s, s + 1);
    if (fabs(u.z) > 1e-2) {
        output->x = 1.0f - (u.x + u.y) / u.z;
        output->y = u.y / u.z;
        output->z = u.x / u.z;
    }
    else {
        output->x = -1;
        output->y = -1;
        output->z = -1;
    }
}

void draw_triangle(vec3f *pts, vec2i *uv, float *zbuffer, tga_image *image, float intensity, tga_color *color) {
    float bboxmin[2] = { FLT_MAX, FLT_MAX };
    float bboxmax[2] = { -FLT_MAX, -FLT_MAX };
    float clamp[2] = { image->width - 1, image->height - 1 };
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 2; j++) {
            bboxmin[j] = fmax(0.0f, fmin(bboxmin[j], pts[i].raw[j]));
            bboxmax[j] = fmin(clamp[j], fmax(bboxmax[j], pts[i].raw[j]));
        }
    }
    vec3f P;
    for (P.x = bboxmin[0]; P.x <= bboxmax[0]; P.x++) {
        for (P.y = bboxmin[1]; P.y <= bboxmax[1]; P.y++) {
            vec3f bc_screen;
            barycentric(&bc_screen, pts[0], pts[1], pts[2], P);
            if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0) {
                continue;
            }
            P.z = 0;
            for (int i = 0; i < 3; i++) {
                P.z += pts[i].raw[2] * bc_screen.raw[i];
            }
            if (zbuffer[(int)(P.x + P.y * width)] < P.z) {
                zbuffer[(int)(P.x + P.y * width)] = P.z;

                vec2i uv_t = (vec2i) { 0, 0 };
                
                uv_t.x += uv[0].x * bc_screen.x + uv[1].x * bc_screen.y + uv[2].x * bc_screen.z;
                uv_t.y += uv[0].y * bc_screen.x + uv[1].y * bc_screen.y + uv[2].y * bc_screen.z;

                tga_color clr = model_diffuse(model, uv_t);
                color_correct_rgba(&clr, intensity);

                if (color) {
                    clr = *color;
                    color_correct_rgba(&clr, intensity);
                }

                image_set_color(image, P.x, P.y, clr);
            }
        }
    }
}

vec3f world_to_screen(vec3f v) {
    return (vec3f) {
        (int)((v.x + 1.0) * width / 2.0 + 0.5),
        (int)((v.y + 1.0) * height / 2.0 + 0.5),
        v.z
    };
}

int main() {
    //Model *model = malloc(sizeof(Model));
    model = malloc(sizeof(Model));
    if (!model_parse(model, "res/head.obj", "res/head_tex.tga")) {
        return -1;
    }
    //model = model;

    //tga_image *texture = malloc(sizeof(tga_image));
    //image_read(texture, "head_tex.tga");
    
    tga_color white = color_from_rgba(255, 255, 255, 255);
    tga_color red = color_from_rgba(255, 0, 0, 255);

    tga_image *image = malloc(sizeof(tga_image));
    image_create(image, width, height, 4);

    vec3f light_dir = { 0, 0, -1 };

    float *zbuffer = malloc(sizeof(float) * width * height);
    for (int i = 0; i < width * height; i++) {
        zbuffer[i] = -FLT_MAX;
    }

    for (int i = 0; i < model_nfaces(model); i++) {
        vec3i face = model_face(model, i);
        vec3f screen_coords[3];
        vec3f world_coords[3];
        for (int j = 0; j < 3; j++) {
            vec3f v = model->vertices[face.raw[j]];
            screen_coords[j] = world_to_screen(v);
            world_coords[j] = v;
        }
        vec3f n, n1, n2;
        vec_sub3f(&n1, &world_coords[2], &world_coords[0]);
        vec_sub3f(&n2, &world_coords[1], &world_coords[0]);
        vec_xor3f(&n, &n1, &n2);
        vec_normalize3f(&n);
        float intensity = vec_dot3f(&n, &light_dir);
        if (intensity > 0) {
            vec2i uv[3];
            for (int k = 0; k < 3; k++) {
                uv[k] = model_uv(model, i, k);
            }
            draw_triangle(screen_coords, &uv, zbuffer, image, intensity, NULL);
        }
    }

    image_flip_vertically(image);
    image_write(image, "output.tga", false);
    //image_write(texture, "texture.tga", false);
    
    image_free(image);
    //image_free(texture);

    free(zbuffer);
    model_free(model);
    //debug();
    return 0;
}
