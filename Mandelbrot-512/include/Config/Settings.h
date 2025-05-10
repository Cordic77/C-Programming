#ifndef _SETTINGS_H_
#define _SETTINGS_H_

/* Option group 1  Simple DirectMedia Layer (SDL): */
#ifdef _WIN32
  #if SDL_VER > 1
  #define SDL_DRAW_SUPPORTED  0    /* As of version 2.0, SDL_Draw has been integrated into SDL itself! */
  #else
  #define SDL_DRAW_SUPPORTED  1    /* SDL_Draw library available? */
  #endif
#endif

#define SDL_TTF_SUPPORTED  1       /* SDL_TTF library available? */
#define SDL_SW_RENDERER    1       /* Render via SDL_Surfaceʼs, whether or not we have SDL1? */

/* Option group 2  Drawing strategy: */
/*#define MANDEL_SINGLE_PIXEL*/             /* Draw a single pixel with each invocation */
/*#define MANDEL_SINGLE_LINE                // Draw a single screen line with each invocation */
#define MANDEL_SINGLE_SCREEN                /* Draw the complete screen with each invocation */

/* Option group 3  Single instruction, multiple data (SIMD)? */
#define SIMD_INSTRUCTIONS             1     /* make use of SSE/AVX SIMD instructions?
                                               0: FRACTAL_FLOAT_ADT … FRACTAL_FLOAT_ASM
                                               1: FRACTAL_FLOAT_SSE1_ASM … FRACTAL_FLOAT_AVX1024_ASM)? */
#define PREFER_INTRINSICS             0     /* prefer the use of intrinsics to raw assembly
                                               0: MandelbrotPixelColor4SSE1 … MandelbrotPixelColor32AVX1024
                                               1: MandelbrotPixelColor4SSE1Intrin … MandelbrotPixelColor16AVX512FIntrin */

/* Option group 4  Implementation: */                /* SINGLE PIXELS PER ITERATION */
#define FRACTAL_FLOAT_ADT             1     /* 32-bit float: rely on Complex_ADT.h/Complex.c */
#define FRACTAL_FLOAT_INLINE          2     /* 32-bit float: inlined calculation  calculate new real/imaginary coordinate separately */

#define FRACTAL_FIXED_ASM             3     /* 32-bit fixed-point: use multiples of 8192 (shift by 13) to simulate fractional digits */
#define FRACTAL_FLOAT_ASM             4     /* 32-bit float: optimized x87 floating point assembly implementation */

#if SIMD_INSTRUCTIONS || defined(MANDEL_SINGLE_SCREEN)/* MULTIPLE PIXELS PER ITERATION */
  #define FRACTAL_FLOAT_SSE1_ASM      5     /* 32-bit float:  4 coordinates at a time  SSE1 (P3 "Katmai") / SSE4.1 (Core 2 Duo T9xxx "Penryn") */
#endif
/*#if SIMD_INSTRUCTIONS*/
  #define FRACTAL_FLOAT_AVX1_ASM      6     /* 32-bit float:  8 coordinates at a time  AVX1 ("Sandy Bridge", "Ivy Bridge") */
  #define FRACTAL_FLOAT_AVX2_ASM      7     /* 32-bit float:  8 coordinates at a time  AVX2 and FMA ("Haswell") */
  #define FRACTAL_FLOAT_AVX512_ASM    8     /* 32-bit float: 16 coordinates at a time  AVX512 and FMA ("Cannon Lake") */
  #define FRACTAL_FLOAT_AVX1024_ASM   9     /* 32-bit float: 16 coordinates at a time  AVX1024 and FMA ("???") */
/*#endif*/

#if SIMD_INSTRUCTIONS
#define SUPPORTED_IMPLEMENTATION   FRACTAL_FLOAT_AVX2_ASM   /* Available instruction set? */
#define TARGETED_IMPLEMENTATION    FRACTAL_FLOAT_AVX2_ASM   /* Targeted instruction set (possibly different)? */
#else
#define SUPPORTED_IMPLEMENTATION   FRACTAL_FLOAT_ASM
#define TARGETED_IMPLEMENTATION    SUPPORTED_IMPLEMENTATION
/* N.b.: TARGETED_IMPLEMENTATION has to be equal to SUPPORTED_IMPLEMENTATION in this case! */
#endif

/*
 derived preprocessor macros: */
#if SUPPORTED_IMPLEMENTATION == FRACTAL_FLOAT_INLINE
  #if defined(_DEBUG)
  #define CALCULATE_OP_STATISTICS           /* Calculate number of required additions/multiplications? */
  #endif
#endif
#define FRACTAL_SIMD_OPTIMIZATION   (TARGETED_IMPLEMENTATION >= FRACTAL_FLOAT_SSE1_ASM)

/* simd && supp <= targ || !simd && supp == targ: */
STATIC_ASSERT ((FRACTAL_SIMD_OPTIMIZATION && SUPPORTED_IMPLEMENTATION < TARGETED_IMPLEMENTATION)
            || SUPPORTED_IMPLEMENTATION == TARGETED_IMPLEMENTATION, SupportedImplementation);

/* Option group 5  Fractal: */
#define FRACTAL_MANDELBROT  1
#define FRACTAL_JULIA       2

#define SELECTED_FRACTAL FRACTAL_MANDELBROT

#if SUPPORTED_IMPLEMENTATION >= FRACTAL_FIXED_ASM
  #undef SELECTED_FRACTAL
  #define SELECTED_FRACTAL FRACTAL_MANDELBROT  /* Mandelbrot fractal is the only one implemented so far … */
#endif

/* Option group 6  Optimizations: */
#if TARGETED_IMPLEMENTATION <= FRACTAL_FLOAT_INLINE
#define MANDEL_CARDIOID_PERIOD2BULB         /* Try to detect if the current point lies within the cardioid/period-2 bulb? */
#endif

#if defined(MANDEL_SINGLE_SCREEN)
/* [1680x1050x8 frame] quadtree: reduces the number of required additions and multiplications from 236.090.620 to 64.938.464 (~ 72,5%): */
#define MANDEL_MARIANI_SILVER_SUBDIVISION   /* Rely on Mariani-Silver subdivision to speed up the calculation? */
/*#define MANDEL_RECREATE_THREADS_PER_ITER  // Restart all threads at the start of each iteration? */
#endif

#endif
