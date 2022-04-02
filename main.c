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
#include "gfx/renderer.h"
#include "gfx/utils.h"
#include "object/object.h"
#include "object/scene.h"

static Point3 SpherePath(float time)
{
    (void)time;
    return (Point3){
        .x = -4.0f,
        .y = 1.0f,
        .z = 0.0f,
    };
}

void FillScene(Scene* scene)
{
    Object obj;

    // add ground
    obj.material.type    = MATERIAL_LAMBERT;
    obj.material.lambert = Lambert_Make((Color){0.2f, 0.2f, 0.2f});
    obj.surface.type     = SURFACE_SPHERE;
    obj.surface.sphere   = Sphere_Make((Point3){0, -1000, 0}, 1000.0f);
    Vector_Push(Object)(scene->objects, &obj);

    // large glass
    obj.material.type       = MATERIAL_DIELECTRIC;
    obj.material.dielectric = Dielectric_Make(COLOR_WHITE, 1.5f);
    obj.surface.type        = SURFACE_SPHERE;
    obj.surface.sphere      = Sphere_Make((Point3){0, 1, 0}, 1.0f);
    Vector_Push(Object)(scene->objects, &obj);

    // large lambert
    obj.material.type        = MATERIAL_LAMBERT;
    obj.material.lambert     = Lambert_Make(COLOR_NAVY);
    obj.surface.type         = SURFACE_MOVING_SPHERE;
    obj.surface.movingSphere = MovingSphere_Make(SpherePath, 1.0f);
    Vector_Push(Object)(scene->objects, &obj);

    // large metal
    obj.material.type  = MATERIAL_METAL;
    obj.material.metal = Metal_Make((Color){0.7f, 0.6f, 0.5f}, 0.0f);
    obj.surface.type   = SURFACE_SPHERE;
    obj.surface.sphere = Sphere_Make((Point3){4, 1, 0}, 1.0f);
    Vector_Push(Object)(scene->objects, &obj);
}

int main(void)
{
    const Point3 lookFrom    = (Point3){10, 10, 10};
    const Point3 lookAt      = (Point3){0, 0, 0};
    const Vec3   vup         = (Vec3){0, 1, 0};
    const float  focusDist   = 10.0f;
    const float  aperature   = 0.0f;
    const float  aspectRatio = 16.0f / 9.0f;
    const float  vFov        = 40.0f;
    const float  timeStart   = 0.0f;
    const float  timeEnd     = 1.0f;

    const size_t imageHeight = 720;
    const size_t imageWidth  = imageHeight * aspectRatio;

    Image*  img   = Image_New(imageWidth, imageHeight);
    Camera* cam   = Camera_New(lookFrom, lookAt, vup, aspectRatio, vFov, aperature, focusDist, timeStart, timeEnd);
    Scene*  scene = Scene_New(COLOR_WHITE);
    FillScene(scene);

    RenderCtx* ctx = RenderCtx_New(scene, img, cam);

    // Time the render
    struct timespec specStart;
    clock_gettime(CLOCK_MONOTONIC, &specStart);
    {
        // TODO: does it make sense for Render to create the image?
        Render(ctx, 256, 32, 4);
    }
    struct timespec specEnd;
    clock_gettime(CLOCK_MONOTONIC, &specEnd);

    float timeDeltaSec  = (float)(specEnd.tv_sec - specStart.tv_sec);
    float timeDeltaFrac = (float)(specEnd.tv_nsec - specStart.tv_nsec) * 1e-9;
    float timeDelta     = timeDeltaSec + timeDeltaFrac;
    printf("\n%.2f seconds taken\n", timeDelta);

    const char filename[] = "output.bmp";

    FILE* fd = fopen(filename, "w+");

    Image_ExportBMP(img, fd);
    printf("Output image written to %s\n", filename);

    Image_Delete(img);
    Scene_Delete(scene);
    RenderCtx_Delete(ctx);
    fclose(fd);

    return 0;
}
