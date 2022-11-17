#pragma once

#include <stdbool.h>

#include "gfx/color.h"
#include "gfx/image.h"

typedef struct Texture Texture;

Texture* Texture_New(void);
void     Texture_Delete(Texture* tex);

bool Texture_Import_BMP(Texture* tex, FILE* fd);
bool Texture_Import_Color(Texture* tex, Color color);

void Texture_Unload(Texture* tex);

Color Texture_ColorAt(const Texture* tex, point2 st);
