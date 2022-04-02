#include <math.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>

#include "common/cext.h"
#include "gfx/camera.h"
#include "gfx/color.h"
#include "gfx/image.h"
#include "gfx/primitives.h"
#include "gfx/utils.h"
#include "object/object.h"
#include "object/scene.h"

#define COLOR_RED        \
    (Color)              \
    {                    \
        0.9f, 0.3f, 0.3f \
    }
#define COLOR_BLUE       \
    (Color)              \
    {                    \
        0.5f, 0.7f, 1.0f \
    }
#define COLOR_GREY       \
    (Color)              \
    {                    \
        0.5f, 0.5f, 0.5f \
    }
#define COLOR_WHITE      \
    (Color)              \
    {                    \
        1.0f, 1.0f, 1.0f \
    }
#define COLOR_BLACK      \
    (Color)              \
    {                    \
        0.0f, 0.0f, 0.0f \
    }
#define COLOR_YELLOW     \
    (Color)              \
    {                    \
        0.8f, 0.6f, 0.4f \
    }
#define COLOR_GREEN      \
    (Color)              \
    {                    \
        0.4f, 0.5f, 0.0f \
    }
#define COLOR_NAVY       \
    (Color)              \
    {                    \
        0.1f, 0.2f, 0.5f \
    }

Scene* g_Scene = NULL;
Image* g_Image = NULL;

Color RayColor(const Ray* ray, size_t depth)
{
    if (depth == 0) {
        return COLOR_BLACK;
    }

    Object* objHit = NULL;
    HitInfo hit;

    if (Scene_HitAt(g_Scene, ray, &objHit, &hit)) {
        Ray   bouncedRay;
        Color bouncedColor;

        if (Material_Bounce(&objHit->material, ray, &hit, &bouncedColor, &bouncedRay)) {
            return Color_Tint(bouncedColor, RayColor(&bouncedRay, depth - 1));
        } else {
            return COLOR_BLACK;
        }
    }

    Vec3  unitDir  = vunit(ray->dir);
    float normY    = 0.5f * (unitDir.y + 1.0f);
    Color skyColor = Color_Blend(COLOR_WHITE, COLOR_BLUE, normY);

    return skyColor;
}

static Point3 SpherePath(float time)
{
    const float radius = 2.0f;
    const float freq   = 2 * PI;
    return (Point3){
        .x = -4.0f,
        .y = radius * sinf(freq * time) + radius + 1,
        .z = radius * cosf(freq * time),
    };
}

void FillScene(void)
{
    Object obj;

    // add ground
    obj = (Object){
        .material = {
            .type = MATERIAL_LAMBERT,
            .lambert = {
                .albedo = (Color){0.2f, 0.2f, 0.2f},
            },
        },
        .surface = {
            .type = SURFACE_SPHERE,
            .sphere = {
                .radius = 1000,
                .center = (Point3){0, -1000, 0},
            },
        },
    };
    Vector_Push(Object)(g_Scene->objects, &obj);

    // large glass
    obj.material.type                      = MATERIAL_DIELECTRIC;
    obj.material.dielectric.albedo         = (Color){1.0f, 1.0f, 1.0f};
    obj.material.dielectric.refactiveIndex = 1.5f;

    obj.surface.type          = SURFACE_SPHERE;
    obj.surface.sphere.radius = 1.0f;
    obj.surface.sphere.center = (Point3){0, 1, 0};
    Vector_Push(Object)(g_Scene->objects, &obj);

    // large lambert
    obj.material.type           = MATERIAL_LAMBERT;
    obj.material.lambert.albedo = COLOR_NAVY;

    obj.surface.type                    = SURFACE_MOVING_SPHERE;
    obj.surface.movingSphere.radius     = 1.0f;
    obj.surface.movingSphere.centerPath = SpherePath;
    Vector_Push(Object)(g_Scene->objects, &obj);

    // large metal
    obj.material.type         = MATERIAL_METAL;
    obj.material.metal.albedo = (Color){0.7f, 0.6f, 0.5f};
    obj.material.metal.fuzz   = 0.0f;

    obj.surface.type          = SURFACE_SPHERE;
    obj.surface.sphere.radius = 1.0f;
    obj.surface.sphere.center = (Point3){4, 1, 0};
    Vector_Push(Object)(g_Scene->objects, &obj);
}

int main(void)
{
    const float aspectRatio = 16.0f / 9.0f;
    const int   imageHeight = 720;
    const int   imageWidth  = (int)(aspectRatio * imageHeight);

    const size_t samplesPerPixel = 512;
    const size_t maxRayDepth     = 32;

    Point3 lookFrom  = (Point3){10, 10, 10};
    Point3 lookAt    = (Point3){0, 0, 0};
    Vec3   vup       = (Vec3){0, 1, 0};
    float  focusDist = 10.0f;
    float  aperature = 0.0f;

    Camera cam = Camera_Make(lookFrom, lookAt, vup, 16.0f / 9.0f, 40.0f, aperature, focusDist, 0, 1);
    g_Scene    = Scene_New();
    FillScene();

    clock_t timeStart = clock();

    // compute the image
    Image* img = Image_New(imageWidth, imageHeight);
    for (ssize_t yy = imageHeight - 1; yy >= 0; yy--) {
        for (size_t xx = 0; xx < img->res.width; xx++) {
            Color cumColorPt = {0};

            for (size_t samples = 0; samples < samplesPerPixel; samples++) {
                float horizontalFraction = (xx + randf()) / (float)(img->res.width - 1);
                float verticalFraction   = (yy + randf()) / (float)(img->res.height - 1);

                Ray   ray      = Camera_GetRay(&cam, horizontalFraction, verticalFraction);
                Color rayColor = RayColor(&ray, maxRayDepth);
                cumColorPt     = vadd(cumColorPt, rayColor);
            }

            Point3 avgColorPt = vdiv(cumColorPt, samplesPerPixel);
            RGB    avgRGB     = RGB_FromFloat(
                sqrtf(clampf(avgColorPt.x, 0.0f, 0.999f)),
                sqrtf(clampf(avgColorPt.y, 0.0f, 0.999f)),
                sqrtf(clampf(avgColorPt.z, 0.0f, 0.999f)));

            Image_SetPixel(img, xx, imageHeight - 1 - yy, avgRGB);
        }
        printf("Finished line %03zd of %d\r", yy, imageHeight);
    }

    clock_t timeEnd      = clock();
    float   timeDeltaSec = ((float)(timeEnd - timeStart) / CLOCKS_PER_SEC);
    printf("\n%.2f seconds taken\n", timeDeltaSec);

    FILE* fd = fopen("output.bmp", "w+");

    Image_ExportBMP(img, fd);
    Image_Delete(img);
    Scene_Delete(g_Scene);

    fclose(fd);

    return 0;
}
