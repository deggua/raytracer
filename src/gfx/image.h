#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "gfx/color.h"

// sRGB Colorspace Image stored as u8x3 in [0, 255]
// 3 bytes per pixel
// Should be used for import/export
typedef struct {
    struct {
        size_t width, height;
    } res;

    RGB* pix;
} ImageRGB;

// Linear Colorspace Image stored as f32x3 in [0, 1]
// 12 bytes per pixel
// Should only be used for imported textures, doesn't require conversions
typedef struct {
    struct {
        size_t width, height;
    } res;

    Color* pix;
} ImageColor;

/* ---- sRGB Colorspace Image ---- */

bool ImageRGB_Load_Empty(out ImageRGB* img, size_t width, size_t height);
bool ImageRGB_Load_BMP(out ImageRGB* img, in FILE* fd);
bool ImageRGB_Load_ImageColor(out ImageRGB* img, in ImageColor* src);

bool ImageRGB_Save_PPM(in ImageRGB* img, out FILE* fd);
bool ImageRGB_Save_BMP(in ImageRGB* img, out FILE* fd);

void ImageRGB_Unload(out ImageRGB* img);

void ImageRGB_SetPixel(inout ImageRGB* img, size_t xx, size_t yy, RGB color);
RGB  ImageRGB_GetPixel(in ImageRGB* img, size_t xx, size_t yy);

/* ---- Linear Colorspace Image ---- */

bool ImageColor_Load_Empty(out ImageColor* img, size_t width, size_t height);
bool ImageColor_Load_ImageRGB(out ImageColor* img, in ImageRGB* src);

void ImageColor_Unload(out ImageColor* img);

void  ImageColor_SetPixel(inout ImageColor* img, size_t xx, size_t yy, Color color);
Color ImageColor_GetPixel(in ImageColor* img, size_t xx, size_t yy);

// TODO: when we implement normal maps (or any other non-sRGB texture) we need
// to provide a way to not do sRGB -> Linear conversion
