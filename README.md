A parallelized CPU raytracer for Linux and Windows with texture mapping

![alt text](https://github.com/deggua/raytracer/blob/main/assets/sample/image.jpg?raw=true)

Build:
* Install make (for Windows see https://gnuwin32.sourceforge.net/packages/make.htm)
* Install clang
* Rename compile_flags_*.txt to compile_flags.txt (based on your platform)
* Rename makefile_* to makefile (based on your platform)
* Run 'make release' to compile
* Run ./bin/rt.exe [num_threads] or ./bin/rt.out [num_threads]

TODO:
* Implement texture mapping for skybox
* Implement normal maps
* PBR/RayTracing.io stuff
* Read the sampling section of the PBR book
* Denoising
* SIMDify the surface intersection code (may require supporting only triangles)
* Optimize render pre-pass
* GPU shader for more parallelization
* Other material types
* Procedural textures
* Real-time display of render
* Improve program structure and memory management of objects
