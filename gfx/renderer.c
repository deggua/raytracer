#include "renderer.h"

#include <math.h>
#include <pthread.h>
#include <stdlib.h>

#include "gfx/random.h"
#include "gfx/utils.h"

RenderCtx* RenderCtx_New(const Scene* scene, Image* img, const Camera* cam)
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

void RenderCtx_Delete(RenderCtx* ctx)
{
    free(ctx);
}

static Color RayColor(const Scene* scene, const Ray* ray, size_t depth)
{
    // TODO: This can be pulled out into pre-RayColor, along with computing every moving object's position
    const Color skyColor = Scene_Get_SkyColor(scene);

    if (depth == 0) {
        return COLOR_BLACK;
    }

    Object* objHit = NULL;
    HitInfo hit;

    if (Scene_ClosestHit(scene, ray, &objHit, &hit)) {
        Ray   bouncedRay;
        Color bouncedColor;

        if (Material_Bounce(objHit->material, ray, &hit, &bouncedColor, &bouncedRay)) {
            return Color_Tint(bouncedColor, RayColor(scene, &bouncedRay, depth - 1));
        } else {
            return COLOR_BLACK;
        }
    }

    return skyColor;
}

typedef struct {
    struct {
        size_t samplesPerPixel;
        size_t maxRayDepth;
    } renderParams;

    const RenderCtx* ctx;

    size_t lineOffset;
    size_t numThreads;
} RenderThreadArg;

static void* RenderThread(void* arg)
{
    RenderThreadArg* args = arg;

    Random_Seed(__builtin_readcyclecounter());

    Image*        img   = args->ctx->img;
    const Camera* cam   = args->ctx->cam;
    const Scene*  scene = args->ctx->scene;

    const size_t numThreads = args->numThreads;
    const size_t lineOffset = args->lineOffset;

    const size_t samplesPerPixel = args->renderParams.samplesPerPixel;
    const size_t maxRayDepth     = args->renderParams.maxRayDepth;

    const size_t imageHeight = args->ctx->img->res.height;
    const size_t imageWidth  = args->ctx->img->res.width;

    // compute the image
    for (ssize_t yy = imageHeight - 1 - lineOffset; yy >= 0; yy -= numThreads) {
        for (size_t xx = 0; xx < imageWidth; xx++) {
            Color cumColorPt = {0};

            for (size_t samples = 0; samples < samplesPerPixel; samples++) {
                float horizontalFraction = (xx + Random_Float()) / (float)(imageWidth - 1);
                float verticalFraction   = (yy + Random_Float()) / (float)(imageHeight - 1);

                Ray   ray      = Camera_GetRay(cam, horizontalFraction, verticalFraction);
                Color rayColor = RayColor(scene, &ray, maxRayDepth);
                cumColorPt     = vadd(cumColorPt, rayColor);
            }

            Point3 avgColorPt = vdiv(cumColorPt, samplesPerPixel);
            RGB    avgRGB     = RGB_FromFloat(
                sqrtf(clampf(avgColorPt.x, 0.0f, 0.999f)),
                sqrtf(clampf(avgColorPt.y, 0.0f, 0.999f)),
                sqrtf(clampf(avgColorPt.z, 0.0f, 0.999f)));

            Image_SetPixel(img, xx, imageHeight - 1 - yy, avgRGB);
        }
        // TODO: figure out a better way to print progress
        printf("Finished line %03zd of %zu\n", yy, imageHeight);
    }

    return NULL;
}

Image* Render(const RenderCtx* ctx, size_t samplesPerPixel, size_t maxRayDepth, size_t numThreads)
{
    const size_t minStackSize = 16 * 1024 * 1024;

    pthread_t*       threads    = calloc(numThreads, sizeof(*threads));
    RenderThreadArg* threadArgs = calloc(numThreads, sizeof(*threadArgs));

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, minStackSize);

    for (size_t ii = 0; ii < numThreads; ii++) {
        threadArgs[ii].ctx                          = ctx;
        threadArgs[ii].renderParams.maxRayDepth     = maxRayDepth;
        threadArgs[ii].renderParams.samplesPerPixel = samplesPerPixel;
        threadArgs[ii].numThreads                   = numThreads;
        threadArgs[ii].lineOffset                   = ii;

        pthread_create(&threads[ii], NULL, RenderThread, &threadArgs[ii]);
    }

    for (size_t ii = 0; ii < numThreads; ii++) {
        pthread_join(threads[ii], NULL);
    }

    free(threads);
    free(threadArgs);

#if 0
    pthread_attr_destroy(&attr);
#endif

    return ctx->img;
}
