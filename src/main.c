#include <math.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "gfx/color.h"
#include "gfx/image.h"
#include "gfx/mesh.h"
#include "math/math.h"
#include "math/random.h"
#include "math/vec.h"
#include "platform/profiling.h"
#include "rt/renderer.h"
#include "world/camera.h"
#include "world/object.h"
#include "world/scene.h"

intern bool ExportImage(ImageRGB* img, const char* filename)
{
    FILE* fd = fopen(filename, "wb+");
    if (fd == NULL) {
        printf("Failed to export render: Cannot open %s\n", filename);
        return false;
    }

    if (!ImageRGB_Save_BMP(img, fd)) {
        fclose(fd);
        printf("Failed to export render: Cannot write to %s\n", filename);
        return false;
    }

    fclose(fd);
    printf("Exported render to %s\n", filename);

    return true;
}

ImageRGB* g_img;

intern void InterruptHandler(int sig)
{
    (void)sig;

    printf("\nExporting partial image to disk\n");

    if (ExportImage(g_img, "partial.bmp")) {
        exit(EXIT_SUCCESS);
    } else {
        ABORT("Failed to write image to disk");
    }
}

Material g_matMesh;
Material g_matMesh2;
Material g_matGround;
Material g_matLight;
Material g_mats[16];

intern void FillScene(Scene* scene, Skybox* skybox)
{
#if 0
    /* Pear Mesh */
    FILE* obj = fopen("assets/pear/obj/pear_export.obj", "r");
    FILE* tex = fopen("assets/pear/tex/pear_diffuse.bmp", "rb");

    Mesh* mesh = Mesh_New();
    Mesh_Import_OBJ(mesh, obj);
    fclose(obj);

    Texture* texMesh = Texture_New();
    Texture_Import_BMP(texMesh, tex);
    fclose(tex);

    g_matMesh.type           = MATERIAL_DIFFUSE;
    g_matMesh.diffuse.albedo = texMesh;

    Mesh_Set_Material(mesh, &g_matMesh);
    Mesh_Set_Origin(mesh, (point3){0, 6, -4});
    Mesh_Set_Scale(mesh, 2.0f);
    Mesh_AddToScene(mesh, scene);

#elif 1

    /* Little Dragon Mesh */
    FILE* littleDragon = fopen("assets/little_dragon.obj", "r");
    if (littleDragon == NULL) {
        ABORT("Couldn't find assets/little_dragon.obj");
    }

    Mesh* mesh = Mesh_New();
    Mesh_Import_OBJ(mesh, littleDragon);
    fclose(littleDragon);

    Texture* texMesh = Texture_New();
    Texture_Import_Color(texMesh, COLOR_YELLOW);

    // g_matMesh = Material_Disney_Glass_Make(texMesh, 0.0f, 0.0f, 1.52f);
    g_matMesh = Material_Disney_Glass_Make(texMesh, 0.2f, 0.0f, 1.52f);
    // g_matMesh = Material_Disney_Metal_Make(texMesh, 0.0f, 0.0f);
    // g_matMesh = Material_Disney_Sheen_Make(texMesh, 1.0f);
    // g_matMesh = Material_Disney_Clearcoat_Make(1.0f);

    Mesh_Set_Material(mesh, &g_matMesh);
    Mesh_Set_Origin(mesh, (point3){0, 0, 1});
    Mesh_Set_Scale(mesh, 1 / 10.0f);
    Mesh_AddToScene(mesh, scene);

    // Mesh_Set_Origin(mesh, (point3){-15, 1, 0});
    // Mesh_AddToScene(mesh, scene);

    Mesh_Delete(mesh);

#elif 0
    /* Spheres */
    Texture* tex = Texture_New();
    Texture_Import_Color(tex, COLOR_WHITE);

    g_mats[0] = Material_Disney_Diffuse_Make(tex, 0.4f, 1.0f);
    g_mats[1] = Material_Disney_Metal_Make(tex, 0.0f, 0.0f);
    g_mats[2] = Material_Disney_Clearcoat_Make(1.0f);
    g_mats[3] = Material_Disney_Glass_Make(tex, 0.2f, 0.0f, 1.54f);
    g_mats[4] = Material_Disney_Sheen_Make(tex, 1.0f);

    for (int64_t ii = 0; ii < 5; ii++) {
        Object sphere = {
            .obj_name = "NULL",
            .material = &g_mats[ii],
            .surface = {
                .type = SURFACE_SPHERE,
                .sphere = {
                    .r = 5.0f,
                    .c = (point3){-24 + 12 * ii, 10, 6},
                },
            },
        };
        Scene_Add_Object(scene, &sphere);
    }

    g_mats[5] = Material_Disney_BSDF_Make(tex, 0.0f, 0.8f, 0.1f, 0.3f, 0.2f, 0.0f, 0.0f, 0.1f, 0.3f, 1.0f, 1.0f);
    Object sphere = {
        .obj_name = "special",
        .material = &g_mats[5],
        .surface = {
            .type = SURFACE_SPHERE,
            .sphere = {
                .r = 5.0f,
                .c = (point3){0, 0, 6},
            },
        },
    };
    Scene_Add_Object(scene, &sphere);
#endif

#if 0
    /* Spheres on Axes */
    Texture* tex1 = Texture_New();
    Texture_Import_Color(tex1, COLOR_WHITE);
    g_mats[0] = Material_Disney_Diffuse_Make(tex1, 1.0f, 0.0f);

    Texture* tex2 = Texture_New();
    Texture_Import_Color(tex2, COLOR_RED);
    g_mats[1] = Material_Disney_Diffuse_Make(tex2, 1.0f, 0.0f);

    Texture* tex3 = Texture_New();
    Texture_Import_Color(tex3, COLOR_GREEN);
    g_mats[2] = Material_Disney_Diffuse_Make(tex3, 1.0f, 0.0f);

    Texture* tex4 = Texture_New();
    Texture_Import_Color(tex4, COLOR_BLUE);
    g_mats[3] = Material_Disney_Diffuse_Make(tex4, 1.0f, 0.0f);

    Object sphere_origin = {
        .material = &g_mats[0],
        .surface = {
            .type = SURFACE_SPHERE,
            .sphere = {
                .r = 1.0f,
                .c = (point3){0, 0, 0},
            },
        },
    };

    Object sphere_x = {
        .material = &g_mats[1],
        .surface = {
            .type = SURFACE_SPHERE,
            .sphere = {
                .r = 1.0f,
                .c = (point3){20, 0, 0},
            },
        },
    };

    Object sphere_y = {
        .material = &g_mats[2],
        .surface = {
            .type = SURFACE_SPHERE,
            .sphere = {
                .r = 1.0f,
                .c = (point3){0, 20, 0},
            },
        },
    };

    Object sphere_z = {
        .material = &g_mats[3],
        .surface = {
            .type = SURFACE_SPHERE,
            .sphere = {
                .r = 1.0f,
                .c = (point3){0, 0, 20},
            },
        },
    };

    Scene_Add_Object(scene, &sphere_origin);
    Scene_Add_Object(scene, &sphere_x);
    Scene_Add_Object(scene, &sphere_y);
    Scene_Add_Object(scene, &sphere_z);
#endif

#if 0
    /* Sphere Light */
    Texture* texLight = Texture_New();
    // c3d7f0
    Texture_Import_Color(texLight, COLOR_WHITE);
    g_matLight = Material_DiffuseLight_Make(texLight, 5.0f);

    Object lightObj;
    lightObj.material         = &g_matLight;
    lightObj.surface.type     = SURFACE_SPHERE;
    lightObj.surface.sphere.c = (point3){20, 20, 10.0f};
    lightObj.surface.sphere.r = 8.0f;
    Scene_Add_Object(scene, &lightObj);
#endif

#if 0
    /* Ground */
    Texture* texGround = Texture_New();
    Texture_Import_Color(texGround, COLOR_GREY);
    g_matGround = Material_Disney_Diffuse_Make(texGround, 1.0f, 0.0f);

    Object ground = {
        .material = &g_matGround,
        .surface = {
            .type = SURFACE_SPHERE,
            .sphere = {
                .c = (point3){0, 0, -1000},
                .r = 1000,
            },
        },
    };

    Scene_Add_Object(scene, &ground);
#endif
    (void)skybox;
}

int main(int argc, char** argv)
{
    point3 lookFrom    = (point3){20, -20, 20};
    point3 lookAt      = (point3){0, 0, 6};
    vec3   vup         = (vec3){0, 0, 1};
    f32    focusDist   = vmag(vsub(lookFrom, lookAt));
    f32    aperature   = 0.0f;
    f32    aspectRatio = 16.0f / 9.0f;
    f32    vFov        = 40.0f;

    size_t imageHeight = 720;
    size_t imageWidth  = imageHeight * aspectRatio;

    size_t numThreads      = 16;
    size_t samplesPerPixel = 32;
    size_t maxBounces      = 10;

    if (argc > 1)
        numThreads = atol(argv[1]);
    if (argc > 2)
        samplesPerPixel = atol(argv[2]);
    if (argc > 3)
        maxBounces = atol(argv[3]);

    printf(
        "Render settings:\n"
        "%zu threads\n"
        "%zu samples per pixel\n"
        "%zu max ray bounces\n\n",
        numThreads,
        samplesPerPixel,
        maxBounces);

    signal(SIGINT, InterruptHandler);

    ImageRGB* img = calloc(1, sizeof(ImageRGB));
    if (img == NULL) {
        ABORT("Failed to create image container");
    }

    if (!ImageRGB_Load_Empty(img, imageWidth, imageHeight)) {
        ABORT("Failed to create image buffer");
    }

    g_img = img;

    Camera* cam = Camera_New(lookFrom, lookAt, vup, aspectRatio, vFov, aperature, focusDist);
    if (cam == NULL) {
        ABORT("Failed to create camera");
    }

    Skybox* skybox = Skybox_Import_BMP("assets/skybox2");
    if (skybox == NULL) {
        ABORT("Failed to load skybox");
    }

    Scene* scene = Scene_New(skybox);
    if (scene == NULL) {
        ABORT("Failed to create scene");
    }

    TIMEIT("Scene load", FillScene(scene, skybox));
    TIMEIT("Scene prepare", Scene_Prepare(scene));

    RenderCtx* ctx = Render_New(scene, img, cam);

    TIMEIT("Scene render", Render_Do(ctx, samplesPerPixel, maxBounces, numThreads));

    // TODO: cleanup on exit

    if (ExportImage(img, "output.bmp")) {
        return EXIT_SUCCESS;
    } else {
        return EXIT_FAILURE;
    }
}
