#define GLFW_INCLUDE_NONE

#include <GL/glew.h>
#include <GLFW/glfw3.h>
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
#include "platform/misc.h"
#include "platform/profiling.h"
#include "rt/renderer.h"
#include "world/camera.h"
#include "world/object.h"
#include "world/scene.h"

Material  g_mats[16];
ImageRGB* g_img;

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

intern void FillScene(Scene* scene, Skybox* skybox)
{
#if 0
    FILE* pear_file = fopen("assets/pear/obj/pear_export.obj", "r");
    if (pear_file == NULL) {
        ABORT("Failed to open pear mesh");
    }

    Mesh* pear_mesh = Mesh_New();
    Mesh_Import_OBJ(pear_mesh, pear_file);
    fclose(pear_file);

    FILE* pear_tex_file = fopen("assets/pear/tex/pear_diffuse.bmp", "rb");
    if (pear_tex_file == NULL) {
        ABORT("Failed to open pear texture");
    }

    Texture* pear_tex = Texture_New();
    Texture_Import_BMP(pear_tex, pear_tex_file);
    fclose(pear_tex_file);

    g_mats[0] = Material_Disney_Glass_Make(pear_tex, 0.0f, 0.0f, 1.52f);

    Mesh_Set_Material(pear_mesh, &g_mats[0]);
    Mesh_Set_Origin(pear_mesh, (point3){0, 0, 6});
    Mesh_Set_Scale(pear_mesh, 5.0f);

    Mesh_AddToScene(pear_mesh, scene);
    Mesh_Delete(pear_mesh);
#endif

#if 1
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
    g_mats[0] = Material_Disney_Glass_Make(texMesh, 0.2f, 0.0f, 1.52f);

    Mesh_Set_Material(mesh, &g_mats[0]);
    Mesh_Set_Origin(mesh, (point3){0, 0, 1});
    Mesh_Set_Scale(mesh, 1 / 10.0f);
    Mesh_AddToScene(mesh, scene);

    Mesh_Delete(mesh);
#endif

#if 0
    /* Spheres */
    Texture* tex = Texture_New();
    Texture_Import_Color(tex, COLOR_WHITE);

    g_mats[0] = Material_Disney_Diffuse_Make(tex, 0.4f, 1.0f);
    g_mats[1] = Material_Disney_Metal_Make(tex, 0.0f, 0.0f);
    g_mats[2] = Material_Disney_Clearcoat_Make(0.0f);
    g_mats[3] = Material_Disney_Glass_Make(tex, 0.2f, 0.0f, 1.54f);
    g_mats[4] = Material_Disney_Sheen_Make(tex, 1.0f);

    for (int64_t ii = 0; ii < 5; ii++) {
        Object sphere = {
            .material = &g_mats[ii],
            .surface  = Surface_Sphere_Make((point3){-24 + 12 * ii, 10, 6}, 5.0f),
        };
        Scene_Add_Object(scene, &sphere);
    }

    struct {
        f32 subsurface;
        f32 specular;
        f32 roughness;
        f32 specular_tint;
        f32 anistropic;
        f32 sheen_tint;
        f32 clearcoat_gloss;
        f32 eta;
        f32 weight_sheen;
        f32 weight_clearcoat;
        f32 weight_metallic;
        f32 weight_specular;
    } args = {
        .subsurface       = 0.0f,
        .specular         = 1.0f,
        .roughness        = 0.0f,
        .specular_tint    = 1.0f,
        .anistropic       = 0.0f,
        .sheen_tint       = 0.0f,
        .clearcoat_gloss  = 1.0f,
        .eta              = 1.52f,
        .weight_sheen     = 0.0f,
        .weight_clearcoat = 0.0f,
        .weight_metallic  = 0.5f,
        .weight_specular  = 1.0f,
    };

    g_mats[5] = Material_Disney_BSDF_Make(
        tex,
        args.subsurface,
        args.specular,
        args.roughness,
        args.specular_tint,
        args.anistropic,
        args.sheen_tint,
        args.clearcoat_gloss,
        args.eta,
        args.weight_sheen,
        args.weight_clearcoat,
        args.specular,
        args.weight_metallic);

    Object sphere = {
        .material = &g_mats[5],
        .surface  = Surface_Sphere_Make((point3){0, 0, 6}, 5.0f),
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
        .surface  = Surface_Sphere_Make((point3){0, 0, 0}, 1.0f),
    };

    Object sphere_x = {
        .material = &g_mats[1],
        .surface  = Surface_Sphere_Make((point3){20, 0, 0}, 1.0f),
    };

    Object sphere_y = {
        .material = &g_mats[2],
        .surface  = Surface_Sphere_Make((point3){0, 20, 0}, 1.0f),
    };

    Object sphere_z = {
        .material = &g_mats[3],
        .surface  = Surface_Sphere_Make((point3){0, 0, 20}, 1.0f),
    };

    Scene_Add_Object(scene, &sphere_origin);
    Scene_Add_Object(scene, &sphere_x);
    Scene_Add_Object(scene, &sphere_y);
    Scene_Add_Object(scene, &sphere_z);
#endif

#if 1
    /* Sphere Light */
    Texture* texLight = Texture_New();
    Texture_Import_Color(texLight, COLOR_WHITE);
    g_mats[15] = Material_DiffuseLight_Make(texLight, 5.0f);

    Object lightObj = {
        .material = &g_mats[15],
        .surface  = Surface_Sphere_Make((point3){20, 20, 10}, 8.0f),
    };

    Scene_Add_Object(scene, &lightObj);
#endif

#if 1
    /* Ground */
    Texture* texGround = Texture_New();
    Texture_Import_Color(texGround, COLOR_GREY);
    g_mats[14] = Material_Disney_Diffuse_Make(texGround, 1.0f, 0.0f);

    Object ground = {
        .material = &g_mats[14],
        .surface  = Surface_Sphere_Make((point3){0, 0, -1000}, 1000),
    };

    Scene_Add_Object(scene, &ground);
#endif

    (void)skybox;
}

intern void glfw_error_handler(int error_code, const char* description)
{
    ABORT("GLFW Error :: %s (%d)", description, error_code);
}

intern void glfw_input_handler(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    (void)window;
    (void)scancode;
    (void)mods;

    if (action == GLFW_RELEASE) {
        switch (key) {
            case GLFW_KEY_ESCAPE: {
                exit(EXIT_SUCCESS);
            } break;
        }
    }
}

const char* glGetErrorString(GLenum err)
{
    switch (err) {
        case GL_NO_ERROR:
            return "GL_NO_ERROR";

        case GL_INVALID_ENUM:
            return "GL_INVALID_ENUM";

        case GL_INVALID_VALUE:
            return "GL_INVALID_VALUE";

        case GL_INVALID_OPERATION:
            return "GL_INVALID_OPERATION";

        case GL_STACK_OVERFLOW:
            return "GL_STACK_OVERFLOW";

        case GL_STACK_UNDERFLOW:
            return "GL_STACK_UNDERFLOW";

        case GL_OUT_OF_MEMORY:
            return "GL_OUT_OF_MEMORY";

        case GL_TABLE_TOO_LARGE:
            return "GL_TABLE_TOO_LARGE";

        case GL_INVALID_FRAMEBUFFER_OPERATION:
            return "GL_INVALID_FRAMEBUFFER_OPERATION";

        case GL_CONTEXT_LOST:
            return "GL_CONTEXT_LOST";

        default:
            return "???";
    }
}

#define GL_CHECK(...)                                                       \
    do {                                                                    \
        __VA_ARGS__;                                                        \
        GLenum err_;                                                        \
        while ((err_ = glGetError()) != GL_NO_ERROR) {                      \
            ABORT("OpenGL Error :: %s (%u)", glGetErrorString(err_), err_); \
        }                                                                   \
    } while (0)

RenderCtx* SetupRender(Stopwatch* sw, size_t res_w, size_t res_h)
{
    point3 lookFrom    = (point3){20, -20, 20};
    point3 lookAt      = (point3){0, 0, 6};
    vec3   vup         = (vec3){0, 0, 1};
    f32    focusDist   = vmag(vsub(lookFrom, lookAt));
    f32    aperature   = 0.0f;
    f32    aspectRatio = (f32)res_w / (f32)res_h;
    f32    vFov        = 40.0f;

    size_t imageWidth  = res_w;
    size_t imageHeight = res_h;

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

    TIMEIT(sw, STOPWATCH_MILISECONDS, "Scene load", FillScene(scene, skybox));
    TIMEIT(sw, STOPWATCH_MILISECONDS, "Scene optimize", Scene_Prepare(scene));

    return Render_New(scene, img, cam);
}

int main(int argc, char** argv)
{
    Stopwatch* sw = Stopwatch_New();
    if (!sw) {
        ABORT("Failed to create stopwatch");
    }

    u64 samples_per_pixel = 32;
    u64 max_ray_bounces   = 8;
    u64 num_threads       = NUM_HYPERTHREADS;

    size_t res_w = 1280;
    size_t res_h = 720;

    if (argc > 1)
        samples_per_pixel = atol(argv[1]);
    if (argc > 2)
        max_ray_bounces = atol(argv[2]);

    printf(
        "Render settings:\n"
        "%lux%lu\n"
        "%lu threads\n"
        "%lu samples per pixel\n"
        "%lu max ray bounces\n\n",
        res_w,
        res_h,
        num_threads,
        samples_per_pixel,
        max_ray_bounces);

    // setup and start the render
    RenderCtx* ctx = SetupRender(sw, res_w, res_h);

    // create GLFW/GLEW and setup the window
    // TODO: move all this GL/window init into a separate function
    glfwSetErrorCallback(glfw_error_handler);
    if (!glfwInit()) {
        ABORT("Failed to initialize GLFW");
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    GLFWwindow* gl_window = glfwCreateWindow(res_w, res_h, "Ray Tracer", NULL, NULL);
    if (!gl_window) {
        ABORT("Failed to create GLFW window");
    }

    glfwMakeContextCurrent(gl_window);
    glfwSetKeyCallback(gl_window, glfw_input_handler);

    GLenum err = glewInit();
    if (err != GLEW_OK) {
        ABORT("Error initializing GLEW");
    }

    printf("Starting :: Render\n");
    Stopwatch_Start(sw);
    Render_Start(ctx, samples_per_pixel, max_ray_bounces);

    // TODO: I think we want to do this here
    signal(SIGINT, InterruptHandler);

    // setup texture
    GLuint gl_texture;
    GL_CHECK(glGenTextures(1, &gl_texture));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, gl_texture));
    GL_CHECK(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, res_w, res_h, 0, GL_BGR, GL_UNSIGNED_BYTE, ctx->img->pix));

    // setup framebuffer
    GLuint gl_framebuffer;
    GL_CHECK(glGenFramebuffers(1, &gl_framebuffer));
    GL_CHECK(glBindFramebuffer(GL_READ_FRAMEBUFFER, gl_framebuffer));
    GL_CHECK(glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gl_texture, 0));

    glfwSwapInterval(0);

    bool last_render_state = false;
    while (!glfwWindowShouldClose(gl_window)) {
        // copy texture to framebuffer
        GL_CHECK(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, res_w, res_h, GL_BGR, GL_UNSIGNED_BYTE, ctx->img->pix));

        // update frame buffer
        GL_CHECK(glBindFramebuffer(GL_READ_FRAMEBUFFER, gl_framebuffer));
        GL_CHECK(glBlitFramebuffer(0, 0, res_w, res_h, 0, 0, res_w, res_h, GL_COLOR_BUFFER_BIT, GL_LINEAR));
        glfwSwapBuffers(gl_window);

        // timing
        bool cur_render_state = Render_Done(ctx);
        if (cur_render_state && !last_render_state) {
            Stopwatch_Stop(sw);
            i64 time_elapsed_ms = Stopwatch_Elapsed(sw, STOPWATCH_MILISECONDS);
            printf(
                "Finished :: Render :: (%lu %s)\n\n",
                time_elapsed_ms,
                STOPWATCH_TIMESCALE_UNIT(STOPWATCH_MILISECONDS));
        }
        last_render_state = cur_render_state;

        // poll events and wait
        glfwPollEvents();
        SleepMS(1000 / RENDER_FPS);
    }

    if (ExportImage(ctx->img, "output.bmp")) {
        return EXIT_SUCCESS;
    }

    return EXIT_FAILURE;
}
