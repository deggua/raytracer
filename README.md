TODO:
* Refactor code it better folder/names/etc
* Implement texture mapping for at least spheres/skybox
* Implement normal maps
* PBR/RayTracing.io stuff
* Fix diffuse lighting model (atm it's a unit sphere at the surface normal, but I think it needs to be a half-sphere)
* Read the sampling section of the PBR book
* Read more about how I should be generating random numbers (if it even matters, xorshiro is fast which is good)
* Add more compiler magic into vector.h to handle const with a pointer typedef, apparently using typeof(*((type)0)) can
get you the original pointer type which can be used correctly with const, probably need to throw in some __builtin_compatible_p etc
to figure out if it is a pointer and do all the weird stuff to it
