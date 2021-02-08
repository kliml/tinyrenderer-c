#ifndef TGAIMAGE_H
#define TGAIMAGE_H

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h> 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#pragma pack(push,1)
typedef struct TGAHeader {
    char idlength;
    char colormaptype;
    char datatypecode;
    short colormaporigin;
    short colormaplength;
    char colormapdepth;
    short x_origin;
    short y_origin;
    short width;
    short height;
    char  bitsperpixel;
    char  imagedescriptor;
} tga_header;
#pragma pack(pop)

typedef struct TGAColor {
    union
    {
        struct {
            unsigned char b, g, r, a;
        };
        unsigned char raw[4];
        unsigned int val;
    };
    int bytespp;
} tga_color;

tga_color color_from_rgba(unsigned char r, unsigned char g, unsigned char b, unsigned char a);
tga_color color_from_raw(unsigned char *p, int bytespp);
tga_color color_from_val(unsigned int v, int bytespp);
void color_correct_rgba(tga_color *color, float val);

typedef struct TGAImage {
    unsigned char *data;
    size_t width;
    size_t height;
    int bytespp;
    enum Format {
        GRAYSCALE = 1, RGB = 3, RGBA = 4
    } format;
} tga_image;

void image_create(tga_image *image, unsigned int w, unsigned int h, int bytespp);

bool image_set_color(tga_image *image, unsigned int x, unsigned int y, tga_color c);
tga_color image_get_color(tga_image *image, unsigned int x, unsigned int y);
bool image_flip_vertically(tga_image *image);

bool image_write(tga_image *image, const char *filename, bool rle);
bool image_read(tga_image *image, const char *filename);
bool image_load_rle_data(tga_image *image, FILE *filep);

void image_free(tga_image *image);

#endif // TGAIMAGE_H
