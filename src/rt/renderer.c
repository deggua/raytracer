#include "renderer.h"

#include <math.h>
#include <stdlib.h>

#include "math/math.h"
#include "math/random.h"
#include "platform/threads.h"

RenderCtx* Render_New(const Scene* scene, ImageRGB* img, const Camera* cam)
{
    RenderCtx* ctx = malloc(sizeof(*ctx));

    if (ctx == NULL) {
        return NULL;
    }

    ctx->cam   = cam;
    ctx->scene = scene;
    ctx->img   = img;

    return ctx;
}

void Render_Delete(RenderCtx* ctx)
{
    free(ctx);
}

static Color RayColor(const Scene* scene, const Ray* ray, size_t depth)
{
    if (depth == 0) {
        return COLOR_BLACK;
    }

    Object* objHit = NULL;
    HitInfo hit;

    if (Scene_ClosestHit(scene, ray, &objHit, &hit)) {
        Ray   bouncedRay;
        Color surfaceColor;
        Color emittedColor;

        if (Material_Bounce(objHit->material, ray, &hit, &surfaceColor, &emittedColor, &bouncedRay)) {
            return Color_BrightenBy(emittedColor, Color_Tint(surfaceColor, RayColor(scene, &bouncedRay, depth - 1)));
        } else {
            return emittedColor;
        }
    }

    return Scene_Get_SkyColor(scene, ray->dir);
}

typedef struct {
    struct {
        size_t samplesPerPixel;
        size_t maxRayDepth;
    } renderParams;

    const RenderCtx* ctx;

    size_t lineOffset;
    size_t lineStep;
} RenderThreadArg;

static void RenderThread(void* arg)
{
    RenderThreadArg* args = arg;

    Random_Seed_HighEntropy();

    ImageRGB*     img   = args->ctx->img;
    const Camera* cam   = args->ctx->cam;
    const Scene*  scene = args->ctx->scene;

    const size_t lineStep   = args->lineStep;
    const size_t lineOffset = args->lineOffset;

    const size_t samplesPerPixel = args->renderParams.samplesPerPixel;
    const size_t maxRayDepth     = args->renderParams.maxRayDepth;

    const size_t imageHeight = args->ctx->img->res.height;
    const size_t imageWidth  = args->ctx->img->res.width;

    // compute the image
    for (i64 yy = imageHeight - 1 - lineOffset; yy >= 0; yy -= lineStep) {
        for (size_t xx = 0; xx < imageWidth; xx++) {
            Color cumColorPt = {0};

            for (size_t samples = 0; samples < samplesPerPixel; samples++) {
                f32 horizontalFraction = (xx + Random_Unilateral()) / (f32)(imageWidth - 1);
                f32 verticalFraction   = (yy + Random_Unilateral()) / (f32)(imageHeight - 1);

                Ray   ray      = Camera_GetRay(cam, horizontalFraction, verticalFraction);
                Color rayColor = RayColor(scene, &ray, maxRayDepth);
                cumColorPt     = vadd(cumColorPt, rayColor);
            }

            Color avgColorPt = vdiv(cumColorPt, samplesPerPixel);
            RGB   avgRGB     = RGB_FromColor(avgColorPt);

            ImageRGB_SetPixel(img, xx, imageHeight - 1 - yy, avgRGB);
        }

        // TODO: figure out a better way to print progress
        printf("\rFinished line %03zd of %zu", yy, imageHeight);
        fflush(stdout);
    }

    return;
}

ImageRGB* Render_Do(const RenderCtx* ctx, size_t samplesPerPixel, size_t maxRayDepth, size_t numThreads)
{
    const size_t minStackSize = 16 * 1024 * 1024;

    Thread**         threads    = calloc(numThreads, sizeof(Thread*));
    RenderThreadArg* threadArgs = calloc(numThreads, sizeof(*threadArgs));

    for (size_t ii = 0; ii < numThreads; ii++) {
        threads[ii] = Thread_New();
        Thread_Set_StackSize(threads[ii], minStackSize);

        threadArgs[ii].ctx                          = ctx;
        threadArgs[ii].renderParams.maxRayDepth     = maxRayDepth;
        threadArgs[ii].renderParams.samplesPerPixel = samplesPerPixel;
        threadArgs[ii].lineStep                     = numThreads;
        threadArgs[ii].lineOffset                   = ii;

        Thread_Spawn(threads[ii], RenderThread, &threadArgs[ii]);
    }

    for (size_t ii = 0; ii < numThreads; ii++) {
        Thread_Join(threads[ii]);
    }

    printf("\nFinished rendering\n");

    for (size_t ii = 0; ii < numThreads; ii++) {
        Thread_Delete(threads[ii]);
    }

    free(threads);
    free(threadArgs);

    return ctx->img;
}
