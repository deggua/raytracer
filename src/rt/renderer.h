#pragma once

#include "gfx/image.h"
#include "world/camera.h"
#include "world/scene.h"

typedef struct {
    const Camera* cam;
    ImageRGB*     img;
    const Scene*  scene;
} RenderCtx;

RenderCtx* Render_New(const Scene* scene, ImageRGB* img, const Camera* cam);
void       Render_Delete(RenderCtx* ctx);
ImageRGB*  Render_Do(const RenderCtx* ctx, size_t samplesPerPixel, size_t maxRayDepth, size_t numThreads);
