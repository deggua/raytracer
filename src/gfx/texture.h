#pragma once

#include <stdbool.h>

#include "gfx/color.h"
#include "gfx/image.h"

typedef struct Texture Texture;

Texture* Texture_New(void);
void     Texture_Delete(out Texture* tex);

bool Texture_Import_BMP(out Texture* tex, in FILE* fd);
bool Texture_Import_Color(out Texture* tex, Color color);

void Texture_Unload(out Texture* tex);

Color Texture_ColorAt(inout Texture* tex, point2 st);
