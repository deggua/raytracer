#pragma once

/* ---- Rendering Parameters ---- */

// FPS the render preview window updates at
#define RENDER_FPS (5)

// The tile size for a render work unit in pixels (tiles are square)
#define RENDER_TILE_W_PX (2)
#define RENDER_TILE_H_PX (2)

/* ---- Raytracing Parameters ---- */

// Epsilon used for RT calculations
#define RT_EPSILON (0.0001f)

/* ---- Host CPU Parameters ---- */

// Specifies the size of the host CPU cache line in bytes
#define SZ_CACHE_LINE (64ull)

// Number of hyperthreads on the CPU (independent threads of execution)
#define NUM_HYPERTHREADS (16ull)

// Total L1d cache size of the CPU
#define SZ_L1_CACHE_TOTAL (512ull * 1024ull)

// Amount of L1 cache to use per hyperthread
#define SZ_L1_CACHE (SZ_L1_CACHE_TOTAL / NUM_HYPERTHREADS)
