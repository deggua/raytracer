## A parallelized CPU raytracer for Linux and Windows

![alt text](https://github.com/deggua/raytracer/blob/main/assets/sample/image.jpg?raw=true)

## Features:
* Mesh loading (Wavefront OBJ)
* Texture mapping (Diffuse in BMP format)
* Cubemap skyboxes (BMP format)
* Simple Materials (Lambertian, Metal, Dielectrics)
* Disney BSDF (Diffuse + SS, Metal + Specular highlight, Clearcoat, Glass, Sheen)
* Kd-Tree accelerator using the SAH

## Build:
* Install make (for Windows see [GnuWin32](https://gnuwin32.sourceforge.net/packages/make.htm))
* Install clang
* Install GLFW + GLEW
* Rename `compile_flags_*.txt` to `compile_flags.txt` (based on your platform)
* Rename `makefile_*` to `makefile` (based on your platform)
* Run `make release` to compile
* Run `./bin/rt.out [spp] [ray_depth]`

## TODO (prep for CUDA):
* Convert surfaces and textures to use surface/texture pools (easier to copy to GPU)
* Better separation of scene construction, scene optimization, and render steps (better for CUDA separation)
* Remove GOTOs and redundant error handling (not point in returning under most circumstances, can't recover/no purpose)
* Use the updated vmath.h header (from the other repo)
* Separate the GUI/GLFW code better (currently a bit messy, should probably be on its own thread)
* Improve program structure and memory management of objects (related to asset pools)
* Improve asset loading and storage of assets in memory (should improve memory storage efficiency of meshes)
* CUDA kernel for GPU execution

## TODO (eventually):
* Improve OBJ loading (multiple meshes per OBJ, proper obj <-> mtl mapping)
* Other texture formats
* Better BMP compatibility
* Volumetric surfaces
* Implement normal maps
* Better sampling
* Procedural textures
* Denoising
* SIMD intersection code
* Optimize render pre-pass
* Procedural textures
* GUI + Interactivity
* Implement a BVH, provide options for exporting scene kd-trees or BVHs for objects to avoid recomputation where possible
* Fix the skybox mirroring
* Load scenes from scene descriptor files
* Hetergeneous compute

## NOTE:
* Z+ is our up axis, X+ is facing right, and Y+ goes into the screen (right handed coordinate system)

## Skyboxes:
* http://www.humus.name/index.php?page=Textures
