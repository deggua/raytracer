#include "image.h"

#include <assert.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "gfx/color.h"

#define PPM_HEADER_FMT \
    "P3\n"             \
    "%zu %zu\n"        \
    "255\n"

#define PPM_HEADER_ARG(img) img->res.width, img->res.height

#define PPM_RGB_FMT      "%" PRIu8 " %" PRIu8 " %" PRIu8
#define PPM_RGB_ARG(rgb) rgb.r, rgb.g, rgb.b

enum BMP_VALUES {
    ID_BM  = 0x4D42,
    BI_RGB = 0,
};

#pragma pack(push, 1)

typedef struct {
    struct {
        u16 id;
        u32 size;
        u16 reserved_0x06;
        u16 reserved_0x08;
        u32 pixelArrayOffset;
    } FileHeader;

    union {
        struct {
            u32 headerSize;
            i32 pixelWidth;
            i32 pixelHeight;
            u16 colorPlanes;
            u16 bitsPerPixel;
            u32 compressionMethod;
            u32 bitmapSize;
            i32 ppmHorizontal;
            i32 ppmVertical;
            u32 numColors;
            u32 numImportantColors;
        } BITMAPINFOHEADER;
    } DIBHeader;
} BMPHeader;

#pragma pack(pop)

static_assert_decl(sizeof(BMPHeader) == 0x36);

intern inline size_t GetPixelIndex(size_t width, size_t height, size_t xx, size_t yy)
{
    (void)height;
    return yy * width + xx;
}

/* ---- sRGB Colorspace Image ---- */

bool ImageRGB_Load_Empty(ImageRGB* img, size_t width, size_t height)
{
    RGB* pixel_buffer = (RGB*)calloc(width * height, sizeof(RGB));
    if (pixel_buffer == NULL) {
        return false;
    }

    img->pix        = pixel_buffer;
    img->res.width  = width;
    img->res.height = height;

    return true;
}

bool ImageRGB_Load_BMP(ImageRGB* img, FILE* fd)
{
    fpos_t init_fpos;
    if (fgetpos(fd, &init_fpos)) {
        ABORT("Failed to get initial file position");
    }

    BMPHeader header;
    if (fread(&header, sizeof(header), 1, fd) != 1) {
        ABORT("Failed to read header from BMP");
    }

    // validate the header
    if (header.FileHeader.id != ID_BM || header.DIBHeader.BITMAPINFOHEADER.bitsPerPixel != 24
        || header.DIBHeader.BITMAPINFOHEADER.compressionMethod != BI_RGB) {
        ABORT("BMP header looks corrupt");
    }

    // construct the image into a temp
    size_t width  = header.DIBHeader.BITMAPINFOHEADER.pixelWidth;
    size_t height = header.DIBHeader.BITMAPINFOHEADER.pixelHeight;

    ImageRGB tmp_img;
    if (!ImageRGB_Load_Empty(&tmp_img, width, height)) {
        ABORT("Failed to initialize image");
    }

    // compute the padding
    size_t pix_bytes_per_row = sizeof(RGB) * tmp_img.res.width;
    size_t pad_bytes_per_row = -pix_bytes_per_row % 4;

    // start reading rows of pixels into the image
    if (fseek(fd, header.FileHeader.pixelArrayOffset, SEEK_SET)) {
        ABORT("Failed to seek to pixel array offset in BMP");
    }

    // read row by row and fill image
    for (ssize_t yy = tmp_img.res.height - 1; yy >= 0; yy--) {
        RGB* pix_row = &tmp_img.pix[GetPixelIndex(tmp_img.res.width, tmp_img.res.height, 0, yy)];
        if (fread(pix_row, sizeof(RGB), tmp_img.res.width, fd) != tmp_img.res.width) {
            ABORT("Failed to read row of pixels from BMP");
        }

        // skip past the end of row padding
        if (fseek(fd, pad_bytes_per_row, SEEK_CUR)) {
            ABORT("Failed to skip padding bytes in BMP");
        }
    }

    // copy the temp's values to the result
    *img = tmp_img;
    return true;
}

bool ImageRGB_Load_ImageColor(ImageRGB* img, ImageColor* src)
{
    ImageRGB tmp_img;
    if (!ImageRGB_Load_Empty(&tmp_img, src->res.width, src->res.height)) {
        ABORT("Failed to initialize image");
    }

    for (size_t yy = 0; yy < src->res.height; yy++) {
        for (size_t xx = 0; xx < src->res.width; xx++) {
            Color pix = ImageColor_GetPixel(src, xx, yy);
            ImageRGB_SetPixel(&tmp_img, xx, yy, RGB_FromColor(pix));
        }
    }

    *img = tmp_img;
    return true;
}

bool ImageRGB_Save_PPM(ImageRGB* img, FILE* fd)
{
    fpos_t init_fpos;
    if (fgetpos(fd, &init_fpos)) {
        ABORT("Failed to get initial file position");
    }

    if (fprintf(fd, PPM_HEADER_FMT, PPM_HEADER_ARG(img)) < 0) {
        ABORT("Failed to write header to PPM");
    }

    for (size_t yy = 0; yy < img->res.height; yy++) {
        for (size_t xx = 0; xx < img->res.width; xx++) {
            if (fprintf(fd, PPM_RGB_FMT "\n", PPM_RGB_ARG(ImageRGB_GetPixel(img, xx, yy))) < 0) {
                ABORT("Failed to write pixel to PPM");
            }
        }
    }

    if (fflush(fd)) {
        ABORT("Failed to flush PPM");
    }

    return true;
}

bool ImageRGB_Save_BMP(ImageRGB* img, FILE* fd)
{
    fpos_t init_fpos;
    if (fgetpos(fd, &init_fpos)) {
        ABORT("Failed to get initial file position of BMP");
    }

    size_t pix_bytes_per_row = sizeof(RGB) * img->res.width;
    size_t pad_bytes_per_row = -pix_bytes_per_row % 4;
    size_t bytes_per_row     = pix_bytes_per_row + pad_bytes_per_row;

    BMPHeader header  = {
        .FileHeader = {
            .id = ID_BM,
            .size = (u32)(sizeof(header) + bytes_per_row * img->res.height),
            .pixelArrayOffset = sizeof(BMPHeader),
        },

        .DIBHeader.BITMAPINFOHEADER = {
            .headerSize         = sizeof(header.DIBHeader.BITMAPINFOHEADER),
            .pixelWidth         = (i32)img->res.width,
            .pixelHeight        = -(i32)img->res.height,
            .colorPlanes        = 1,
            .bitsPerPixel       = 24,
            .compressionMethod  = BI_RGB,
            .bitmapSize         = (u32)(bytes_per_row * img->res.height),
            .ppmHorizontal      = (i32)img->res.width,
            .ppmVertical        = (i32)img->res.height,
            .numColors          = 0,
            .numImportantColors = 0,
        },
    };

    // write the header to the file
    if (fwrite(&header, sizeof(header), 1, fd) != 1) {
        ABORT("Failed to write header to BMP");
    }

    // write out rows bottom to top
    u8 padding[4] = {0};
    for (ssize_t yy = img->res.height - 1; yy >= 0; yy--) {
        // write out the row
        RGB* pix_row = &img->pix[GetPixelIndex(img->res.width, img->res.height, 0, yy)];
        if (fwrite(pix_row, sizeof(RGB), img->res.width, fd) != img->res.width) {
            ABORT("Failed to write pixel row to BMP");
        }

        // write out the padding
        if (fwrite(padding, sizeof(u8), pad_bytes_per_row, fd) != pad_bytes_per_row) {
            ABORT("Failed to write padding to BMP");
        }
    }

    if (fflush(fd)) {
        ABORT("Failed to flush BMP");
    }

    return true;
}

void ImageRGB_Unload(ImageRGB* img)
{
    free(img->pix);

    img->res.width  = 0;
    img->res.height = 0;
    img->pix        = NULL;
}

void ImageRGB_SetPixel(ImageRGB* img, size_t xx, size_t yy, RGB color)
{
    img->pix[GetPixelIndex(img->res.width, img->res.height, xx, yy)] = color;
}

RGB ImageRGB_GetPixel(ImageRGB* img, size_t xx, size_t yy)
{
    return img->pix[GetPixelIndex(img->res.width, img->res.height, xx, yy)];
}

/* ---- Linear Colorspace Image ---- */

bool ImageColor_Load_Empty(ImageColor* img, size_t width, size_t height)
{
    Color* pixel_buffer = (Color*)calloc(width * height, sizeof(Color));
    if (pixel_buffer == NULL) {
        return false;
    }

    img->pix        = pixel_buffer;
    img->res.width  = width;
    img->res.height = height;

    return true;
}

bool ImageColor_Load_ImageRGB(ImageColor* img, ImageRGB* src)
{
    ImageColor tmp_img;
    if (!ImageColor_Load_Empty(&tmp_img, src->res.width, src->res.height)) {
        ABORT("Failed to initialize empty image");
    }

    for (size_t yy = 0; yy < src->res.height; yy++) {
        for (size_t xx = 0; xx < src->res.width; xx++) {
            RGB pix = ImageRGB_GetPixel(src, xx, yy);
            ImageColor_SetPixel(&tmp_img, xx, yy, Color_FromRGB(pix));
        }
    }

    *img = tmp_img;
    return true;
}

void ImageColor_Unload(ImageColor* img)
{
    free(img->pix);

    img->res.width  = 0;
    img->res.height = 0;
    img->pix        = NULL;
}

void ImageColor_SetPixel(ImageColor* img, size_t xx, size_t yy, Color color)
{
    img->pix[GetPixelIndex(img->res.width, img->res.height, xx, yy)] = color;
}

Color ImageColor_GetPixel(ImageColor* img, size_t xx, size_t yy)
{
    return img->pix[GetPixelIndex(img->res.width, img->res.height, xx, yy)];
}
