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

void FillScene(Scene* scene)
{
    const float scale        = 8;
    const float groundOffset = 1;
    Object      obj;

    for (ssize_t xx = -2; xx < 2; xx++) {
        for (ssize_t yy = 0; yy < 5; yy++) {
            for (ssize_t zz = -2; zz < 2; zz++) {
                float rr = randrf(0.0f, 1.0f);
                if (rr < 0.33) {
                    // lambert
                    obj.material.type    = MATERIAL_LAMBERT;
                    obj.material.lambert = Lambert_Make(COLOR_NAVY);
                    obj.surface.type     = SURFACE_SPHERE;
                    obj.surface.sphere = Sphere_Make((Point3){xx * scale, groundOffset + yy * scale, zz * scale}, 1.0f);
                } else if (rr < 0.67) {
                    // metal
                    obj.material.type  = MATERIAL_METAL;
                    obj.material.metal = Metal_Make((Color){0.7f, 0.6f, 0.5f}, 0.0f);
                    obj.surface.type   = SURFACE_SPHERE;
                    obj.surface.sphere = Sphere_Make((Point3){xx * scale, groundOffset + yy * scale, zz * scale}, 1.0f);
                } else {
                    // dielectric
                    obj.material.type       = MATERIAL_DIELECTRIC;
                    obj.material.dielectric = Dielectric_Make(COLOR_WHITE, 1.5f);
                    obj.surface.type        = SURFACE_SPHERE;
                    obj.surface.sphere = Sphere_Make((Point3){xx * scale, groundOffset + yy * scale, zz * scale}, 1.0f);
                }
                Scene_Add_Object(scene, &obj);
            }
        }
    }

    // ground
    obj.material.type    = MATERIAL_LAMBERT;
    obj.material.lambert = Lambert_Make(COLOR_GREY);
    obj.surface.type     = SURFACE_SPHERE;
    obj.surface.sphere   = Sphere_Make((Point3){0, -1000, 0}, 1000.0f);
    Scene_Add_Object(scene, &obj);
}

int main(void)
{
    const Point3 lookFrom    = (Point3){20, 12, 20};
    const Point3 lookAt      = (Point3){3, 3, 3};
    const Vec3   vup         = (Vec3){0, 1, 0};
    const float  focusDist   = 10.0f;
    const float  aperature   = 0.0f;
    const float  aspectRatio = 16.0f / 9.0f;
    const float  vFov        = 40.0f;
    const float  timeStart   = 0.0f;
    const float  timeEnd     = 1.0f;

    const size_t imageHeight = 720;
    const size_t imageWidth  = imageHeight * aspectRatio;

    struct timespec specStart;
    struct timespec specEnd;

    Image*  img   = Image_New(imageWidth, imageHeight);
    Camera* cam   = Camera_New(lookFrom, lookAt, vup, aspectRatio, vFov, aperature, focusDist, timeStart, timeEnd);
    Scene*  scene = Scene_New(COLOR_WHITE);

    Random_Seed(__builtin_readcyclecounter());
    FillScene(scene);
    Scene_Prepare(scene);

    RenderCtx* ctx = RenderCtx_New(scene, img, cam);

    // Time the render
    {
        clock_gettime(CLOCK_MONOTONIC, &specStart);
        // TODO: does it make sense for Render to create the image?
        // TODO: pass in worker thread stack size? or could we compute the required stack size from the ray depth?
        Render(ctx, 64, 32, 4);

        clock_gettime(CLOCK_MONOTONIC, &specEnd);
        float timeDeltaSec  = (float)(specEnd.tv_sec - specStart.tv_sec);
        float timeDeltaFrac = (float)(specEnd.tv_nsec - specStart.tv_nsec) * 1e-9;
        float timeDelta     = timeDeltaSec + timeDeltaFrac;
        printf("\n%.2f seconds spent rendering\n", timeDelta);
    }

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
