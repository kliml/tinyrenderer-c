#include "tgaimage.h"

tga_color color_from_rgba(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
    tga_color clr;
    clr.r = r;
    clr.g = g;
    clr.b = b;
    clr.a = a;
    clr.bytespp = 4;
    return clr;
}

tga_color color_from_raw(unsigned char *p, int bytespp) {
    tga_color clr;
    clr.bytespp = bytespp;
    for (int i = 0; i < bytespp; i++) {
        clr.raw[i] = p[i];
    }
    return clr;
}

tga_color color_from_val(unsigned int v, int bytespp) {
    tga_color clr;
    clr.bytespp = bytespp;
    clr.val = v;
    return clr;
}

void color_correct_rgba(tga_color *color, float val) {
    color->r *= val;
    color->g *= val;
    color->b *= val;
    color->a = 255;
}

void image_create(tga_image *image, unsigned int w, unsigned int h, int bytespp) {
    image->width = w;
    image->height = h;
    image->bytespp = bytespp;
    unsigned long nbytes = w * h * bytespp;
    image->data = (char*)malloc(sizeof(unsigned char) * nbytes);
    memset(image->data, 0, nbytes);
}

bool image_set_color(tga_image *image, unsigned int x, unsigned int y, tga_color c) {
    if (!image->data || x > image->width || y > image->height) return false;
    memcpy(image->data + (x + y * image->width) * image->bytespp, c.raw, image->bytespp);
    return true;
}

tga_color image_get_color(tga_image *image, unsigned int x, unsigned int y) {
    return color_from_raw(image->data + (x + y * image->width) * image->bytespp, image->bytespp);
}

bool image_flip_vertically(tga_image *image) {
    if (!image->data) return false;
    unsigned long bytes_per_line = image->width * image->bytespp;
    unsigned long *line = malloc(sizeof(unsigned char) * bytes_per_line);
    int half = image->height >> 1;
    for (int i = 0; i < half; i++) {
        unsigned long l1 = i * bytes_per_line;
        unsigned long l2 = (image->height - 1 - i) * bytes_per_line;
        memmove((void *)line, (void *)(image->data + l1), bytes_per_line);
        memmove((void *)(image->data + l1), (void *)(image->data + l2), bytes_per_line);
        memmove((void *)(image->data + l2), (void *)(line), bytes_per_line);
    }
    free(line);
    return true;
}

bool image_flip_horizontally(tga_image *image) {
    if (!image->data) return false;
    int half = image->width >> 1;
    for (int i = 0; i < half; i++) {
        for (int j = 0; j < image->height; j++) {
            tga_color c1 = image_get_color(image, i, j);
            tga_color c2 = image_get_color(image, image->width - 1 - j, j);
            image_set_color(image, i, j, c2);
            image_set_color(image, image->width - 1 - j, j, c1);
        }
    }
    return true;
}


bool image_write(tga_image *image, const char *filename, bool rle) {
    FILE *filep;
    filep = fopen(filename, "wb");

    if (!filep) {
        fprintf(stderr, "could not open file");
        return false;
    }

    unsigned char developer_area_ref[4] = { 0, 0, 0, 0 };
    unsigned char extension_area_ref[4] = { 0, 0, 0, 0 };
    unsigned char footer[18] = { 'T','R','U','E','V','I','S','I','O','N','-','X','F','I','L','E','.','\0' };

    tga_header header;
    memset((void *)&header, 0, sizeof(tga_header));
    header.bitsperpixel = image->bytespp << 3;
    header.width = image->width;
    header.height = image->height;
    header.datatypecode = (image->bytespp == GRAYSCALE ? (rle ? 11 : 3) : (rle ? 10 : 2));
    header.imagedescriptor = 0x20;

    fwrite((char *)&header, sizeof(header), 1, filep);
    fwrite((char *)image->data, image->width * image->height * image->bytespp, 1, filep);
    fwrite((char *)developer_area_ref, sizeof(developer_area_ref), 1, filep);
    fwrite((char *)extension_area_ref, sizeof(extension_area_ref), 1, filep);
    fwrite((char *)footer, sizeof(footer), 1, filep);

    fclose(filep);
}

bool image_read(tga_image *image, const char *filename) {
    image->data = NULL;
    
    FILE *filep;
    filep = fopen(filename, "rb");
    if (!filep) {
        fprintf(stderr, "could not open file %s", filename);
        return false;
    }

    tga_header header;
    fread((char *)&header, sizeof(header), 1, filep);
    fseek(filep, header.idlength, SEEK_CUR);
    
    image->width = header.width;
    image->height = header.height;
    image->bytespp = header.bitsperpixel >> 3;
    unsigned long nbytes = image->width * image->height * image->bytespp;
    image->data = (char*)malloc(sizeof(unsigned char) * nbytes);

    if (header.datatypecode == 3 || header.datatypecode == 2) {
        if (fgets((char *)image->data, nbytes, filep) == NULL) {
            fprintf(stderr, "an error occured while reading the data\n");
            fclose(filep);
            return false;
        }
    } else if (header.datatypecode == 10 || header.datatypecode == 11) {       
        if (!image_load_rle_data(image, filep)) {
            fprintf(stderr, "an error occured while reading the data\n");
            fclose(filep);
            return false;
        }
    } else {
        fclose(filep);
        fprintf(stderr, "unknown file format %d\n", header.datatypecode);
        return false;
    }

    if (!(header.imagedescriptor & 0x20)) {
        image_flip_vertically(image);
    }
    if (!(header.imagedescriptor & 0x10)) {
        image_flip_horizontally(image);
    }

    fprintf(stderr, "%s width: %d height: %d bytespp: %d\n", filename, image->width, image->height, image->bytespp * 8);
    fclose(filep);
    return true;
}

bool image_load_rle_data(tga_image *image, FILE *filep) {
    unsigned long pixel_count = image->width * image->height;
    unsigned long current_pixel = 0;
    unsigned long current_byte = 0;
    tga_color color_buffer;
    do {
        unsigned char chunk_header = 0;
        chunk_header = fgetc(filep);

        //if (chunk_header == NULL) {
        //    fprintf(stderr, "an error occured while reading the data, chunk_header: %d\n", chunk_header);
        //    return false;
        //}

        if (chunk_header < 128) {
            chunk_header++;
            for (int i = 0; i < chunk_header; i++) {
                //if (fgets((char *)&color_buffer, image->bytespp, filep) == NULL) {
                //    fprintf(stderr, "an error occured while reading the header\n");
                //    return false;
                //}

                fread((char *)&color_buffer, sizeof(unsigned char), image->bytespp, filep);

                for (int j = 0; j < image->bytespp; j++) {
                    image->data[current_byte++] = color_buffer.raw[j];
                }
                current_pixel++;
                if (current_pixel > pixel_count) {
                    fprintf(stderr, "too many pixels read\n");
                    return false;
                }
            }
        } else {
            chunk_header -= 127;
            //if (fgets((char *)&color_buffer.raw, image->bytespp, filep) == NULL) {
            //    fprintf(stderr, "an error occured while reading the header\n");
            //    return false;
            //}
            fread((char *)&color_buffer, sizeof(unsigned char), image->bytespp, filep);

            for (int i = 0; i < chunk_header; i++) {
                for (int j = 0; j < image->bytespp; j++) {
                    image->data[current_byte++] = color_buffer.raw[j];
                }
                current_pixel++;
                if (current_pixel > pixel_count) {
                    fprintf(stderr, "too many pixels read\n");
                    return false;
                }
            }
        }
    } while (current_pixel < pixel_count);
    return true;
}

void image_free(tga_image *image) {
    if (image->data) {
        free(image->data);
    }
    free(image);
}
