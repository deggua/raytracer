#include "image.h"

#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "common/cext.h"

Image* Image_New(size_t width, size_t height)
{
    Image* img      = calloc(1, sizeof(Image) + (width * height) * sizeof(RGB));
    img->res.width  = width;
    img->res.height = height;

    return img;
}

void Image_Delete(Image* img)
{
    free(img);
}

void Image_SetPixel(Image* img, size_t xx, size_t yy, RGB color)
{
    img->pix[yy * img->res.width + xx] = color;
}

RGB Image_GetPixel(const Image* img, size_t xx, size_t yy)
{
    return img->pix[yy * img->res.width + xx];
}

// TODO:
// Image_Resize
// Image_Crop

/* ---- PPM ---- */

#define PPM_HEADER_FMT \
    "P3\n"             \
    "%zu %zu\n"        \
    "255\n"

#define PPM_HEADER_ARG(img) img->res.width, img->res.height

#define PPM_RGB_FMT      "%" PRIu8 " %" PRIu8 " %" PRIu8
#define PPM_RGB_ARG(rgb) rgb.r, rgb.g, rgb.b

void Image_ExportPPM(const Image* img, FILE* fd)
{
    fprintf(fd, PPM_HEADER_FMT, PPM_HEADER_ARG(img));

    for (size_t yy = 0; yy < img->res.height; yy++) {
        for (size_t xx = 0; xx < img->res.width; xx++) {
            fprintf(fd, PPM_RGB_FMT "\n", PPM_RGB_ARG(Image_GetPixel(img, xx, yy)));
        }
    }

    fflush(fd);
}

/* ---- BMP ---- */

enum BMP_VALUES
{
    ID_BM  = 'MB',
    BI_RGB = 0,
};

typedef struct {
    uint8_t b;
    uint8_t g;
    uint8_t r;
} attr_aligned(1) BGR;

typedef struct {
    struct {
        uint16_t id;
        uint32_t size;
        uint16_t reserved_0x06;
        uint16_t reserved_0x08;
        uint32_t pixelArrayOffset;
    } attr_aligned(1) FileHeader;

    union {
        struct {
            uint32_t headerSize;
            int32_t  pixelWidth;
            int32_t  pixelHeight;
            uint16_t colorPlanes;
            uint16_t bitsPerPixel;
            uint32_t compressionMethod;
            uint32_t bitmapSize;
            int32_t  ppmHorizontal;
            int32_t  ppmVertical;
            uint32_t numColors;
            uint32_t numImportantColors;
        } attr_aligned(1) BITMAPINFOHEADER;
    } attr_aligned(1) DIBHeader;
} attr_aligned(1) BMPHeader;

static_assert(sizeof(BMPHeader) == 0x36, "Incorrect BMP header size");
static_assert(sizeof(BGR) == 3, "Incorrect BGR pixel size");

void Image_ExportBMP(const Image* img, FILE* fd)
{
    const size_t pixBytesPerRow = sizeof(BGR) * img->res.width;

    size_t padBytesPerRow;
    if (pixBytesPerRow % sizeof(uint32_t) != 0) {
        padBytesPerRow = sizeof(uint32_t) - pixBytesPerRow;
    } else {
        padBytesPerRow = 0;
    }

    const size_t rowBytes = pixBytesPerRow + padBytesPerRow;

    BMPHeader header  = {
        .FileHeader = {
            .id = ID_BM,
            .size = sizeof(header) + rowBytes * img->res.height,
            .pixelArrayOffset = sizeof(BMPHeader),
        },

        .DIBHeader.BITMAPINFOHEADER = {
            .headerSize         = sizeof(header.DIBHeader.BITMAPINFOHEADER),
            .pixelWidth         = img->res.width,
            .pixelHeight        = img->res.height,
            .colorPlanes        = 1,
            .bitsPerPixel       = 24,
            .compressionMethod  = BI_RGB,
            .bitmapSize         = rowBytes * img->res.height,
            .ppmHorizontal      = img->res.width,
            .ppmVertical        = img->res.height,
            .numColors          = 0,
            .numImportantColors = 0,
        },
    };

    uint8_t* pixelBuffer = calloc(1, rowBytes * img->res.height);
    uint8_t* curPixel    = pixelBuffer;
    for (ssize_t yy = img->res.height - 1; yy >= 0; yy--) {
        for (size_t xx = 0; xx < img->res.width; xx++) {
            BGR* bmpPix = (BGR*)curPixel;
            RGB  pix    = Image_GetPixel(img, xx, yy);
            *bmpPix     = (BGR){.b = pix.b, .g = pix.g, .r = pix.r};

            curPixel += sizeof(BGR);
        }
        curPixel += padBytesPerRow;
    }

    // fill out the header with relevant size data
    fwrite(&header, sizeof(header), 1, fd);
    fwrite(pixelBuffer, rowBytes * img->res.height, 1, fd);
    free(pixelBuffer);
    fflush(fd);
}
