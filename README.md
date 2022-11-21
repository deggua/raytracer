A parallelized CPU raytracer for Linux and Windows with no dependencies

![alt text](https://github.com/deggua/raytracer/blob/main/assets/sample/image.jpg?raw=true)

Features:
* Mesh loading (Wavefront OBJ)
* Texture mapping (BMP only)
* Cubemap skyboxes (BMP only)
* Kd-Tree accelerator using the SAH
* Simple BSDFs (Lambertian, mirror, glass)
* Disney BSDFs (In-progress)

Build:
* Install make (for Windows see https://gnuwin32.sourceforge.net/packages/make.htm)
* Install clang
* Rename compile_flags_*.txt to compile_flags.txt (based on your platform)
* Rename makefile_* to makefile (based on your platform)
* Run 'make release' to compile
* Run ./bin/rt.exe [nthreads] [spp] [ray_depth] or ./bin/rt.out [nthreads] [spp] [ray_depth]

TODO:
* Implement normal maps
* PBR/RayTracing.io stuff
* Read the sampling section of the PBR book
* Denoising
* SIMDify the surface intersection code (may require supporting only triangles)
* Optimize render pre-pass
* GPU shader for more parallelization
* Procedural textures
* Real-time display of render
* Improve program structure and memory management of objects
* Improve asset loading and storage of assets in memory
* GUI
* Chunk image into squares, implement a work queue for better distribution of work, hetergeneous compute (CPU + GPU)
* Hierarchical hybrid KdTree/BVH (external processing of .obj to produce their KdTree, alleviates the need to re-process meshes)
* Fix the skybox mirroring

NOTE:
* Z+ is our up axis, X+ is facing right, and Y+ goes into the screen (right handed coordinate system)

Skyboxes:
* http://www.humus.name/index.php?page=Textures
