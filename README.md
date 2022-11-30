A parallelized CPU raytracer for Linux and Windows

![alt text](https://github.com/deggua/raytracer/blob/main/assets/sample/image.jpg?raw=true)

Features:
* Mesh loading (Wavefront OBJ)
* Texture mapping (Diffuse in BMP format)
* Cubemap skyboxes (BMP format)
* Simple Materials (Lambertian, Mirror, Glass)
* Disney BSDF (Diffuse + SS, Metal + Specular highlight, Clearcoat, Glass, Sheen)
* Kd-Tree accelerator using the SAH

Build:
* Install make (for Windows see https://gnuwin32.sourceforge.net/packages/make.htm)
* Install clang
* Install GLFW + GLEW
* Rename compile_flags_*.txt to compile_flags.txt (based on your platform)
* Rename makefile_* to makefile (based on your platform)
* Run 'make release' to compile
* Run ./bin/rt.out [spp] [ray_depth]

TODO:
* Volumetric surfaces
* Implement normal maps
* Better sampling
* Procedural textures
* Denoising
* SIMDify the surface intersection code (may require supporting only triangles)
* Optimize render pre-pass
* GPU shader for more parallelization
* Procedural textures
* Improve program structure and memory management of objects
* Improve asset loading and storage of assets in memory
* GUI + Interactivity
* Implement a BVH, provide options for exporting scene kd-trees or BVHs for objects to avoid recomputation where possible
* Fix the skybox mirroring

NOTE:
* Z+ is our up axis, X+ is facing right, and Y+ goes into the screen (right handed coordinate system)

Skyboxes:
* http://www.humus.name/index.php?page=Textures
