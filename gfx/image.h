#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "gfx/color.h"

typedef struct {
    struct {
        size_t width, height;
    } res;
    RGB pix[];
} Image;

Image* Image_New(size_t width, size_t height);
void Image_Delete(Image* img);
void Image_SetPixel(Image* img, size_t xx, size_t yy, RGB color);
RGB Image_GetPixel(const Image* img, size_t xx, size_t yy);

void Image_ExportPPM(const Image* img, FILE* fd);
void Image_ExportBMP(const Image* img, FILE* fd);
