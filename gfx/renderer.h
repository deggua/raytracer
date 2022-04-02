#pragma once

#include "gfx/camera.h"
#include "gfx/image.h"
#include "object/scene.h"

typedef struct {
    const Camera* cam;
    Image* img;
    const Scene* scene;
} RenderCtx;

RenderCtx* RenderCtx_New(const Scene* scene, Image* img, const Camera* cam);
void RenderCtx_Delete(RenderCtx* ctx);
Image* Render(const RenderCtx* ctx, size_t samplesPerPixel, size_t maxRayDepth, size_t numThreads);
