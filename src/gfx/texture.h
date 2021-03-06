#pragma once

#include "gfx/color.h"
#include "gfx/image.h"

typedef struct Texture Texture;

Texture* Texture_New(void);
void Texture_Delete(Texture* tex);

Texture* Texture_Import_BMP(Texture* tex, FILE* fd);
Texture* Texture_Import_Color(Texture* tex, Color color);

Color Texture_ColorAt(const Texture* tex, point2 st);
