#ifndef _FRACTAL_H_
#define _FRACTAL_H_

#include "SIMD/SIMDTypes.h"


/* Preprocessor macros: */
#if defined(_DEBUG)
#define UPDATE_SET_DIMENSIONS
#define FRACTAL_SCREEN_COORDS
#endif

#if SUPPORTED_IMPLEMENTATION == FRACTAL_FIXED_ASM
  typedef signed long SCREEN_MAPPING;
  /* Use integer math when the pixel spacing is greater than the Escape */
  /* Radius (2.0) divided by 2^13 <? pixel spacing > 0,000244140625 */
  #define MIN_PIXEL_SPACING  (2.0 / 8192.0)
#else
  typedef float SCREEN_MAPPING;
  #define MIN_PIXEL_SPACING  (0.00005 / screen_height)
#endif


  /* Function declarations: */
EXTERN_C unsigned long
  MandelbrotPixelColorFIXED (signed long x, signed long y, BOOL screenCoords);
EXTERN_C unsigned long
  MandelbrotPixelColorFLOAT (float x, float y, BOOL screenCoords);
EXTERN_C void
  MandelbrotPixelColor4SSE1 (__m128 *c_real, __m128 *c_imag, __m128i *result);
/* */
EXTERN_C void
  MandelbrotPixelColor8AVX1 (__m256 *c_real, __m256 *c_imag, __m256i *result);
EXTERN_C void
  MandelbrotPixelColor8AVX2 (__m256 *c_real, __m256 *c_imag, __m256i *result);
EXTERN_C void
  MandelbrotPixelColor16AVX512F (__m512 *c_real, __m512 *c_imag, __m512i *result);


/* Preprocessor macros: */
#if SUPPORTED_IMPLEMENTATION == FRACTAL_FLOAT_ADT
  #if SELECTED_FRACTAL == FRACTAL_JULIA
  #define FractalPixelColor(x, y, result, screenCoords) (result=JuliaPixelColorSimpleADT(x, y, screenCoords))
  #else
  #define FractalPixelColor(x, y, result, screenCoords) (result=MandelbrotPixelColorADT(x, y, screenCoords))
  #endif

#elif SUPPORTED_IMPLEMENTATION == FRACTAL_FLOAT_INLINE
  #if SELECTED_FRACTAL == FRACTAL_JULIA
  #define FractalPixelColor(x, y, result, screenCoords) (result=JuliaPixelColorINL(x, y, screenCoords))
  #else
  #define FractalPixelColor(x, y, result, screenCoords) (result=MandelbrotPixelColorINL(x, y, screenCoords))
  #endif

#elif SUPPORTED_IMPLEMENTATION == FRACTAL_FLOAT_ASM
  #define FractalPixelColor(x, y, result, screenCoords) (result=MandelbrotPixelColorFLOAT(x, y, screenCoords))
  EXTERN_C BOOL FractalDrawFLOAT (int l, int u, int r, int b);
/*#define FractalDraw(l,u,r,b) (FractalDraw)(l,u,r,b) */
  #define FractalDraw(l,u,r,b) FractalDrawFLOAT(l,u,r,b)

#elif SUPPORTED_IMPLEMENTATION == FRACTAL_FIXED_ASM
  #define FractalPixelColor(x, y, result, screenCoords) (result=MandelbrotPixelColorFIXED(x, y, screenCoords))
  EXTERN_C BOOL FractalDrawFIXED (int l, int u, int r, int b);
/*#define FractalDraw(l,u,r,b) (FractalDraw)(l,u,r,b) */
  #define FractalDraw(l,u,r,b) FractalDrawFIXED(l,u,r,b)

#elif SUPPORTED_IMPLEMENTATION == FRACTAL_FLOAT_SSE1_ASM
  #if PREFER_INTRINSICS
  #define FractalPixelColor(x, y, result, screenCoords) MandelbrotPixelColor4SSE1Intrin(x, y, result)
  #else
  #define FractalPixelColor(x, y, result, screenCoords) MandelbrotPixelColor4SSE1(x, y, result)
  #endif

#elif SUPPORTED_IMPLEMENTATION == FRACTAL_FLOAT_AVX1_ASM
  #if PREFER_INTRINSICS
  #define FractalPixelColor(x, y, result, screenCoords) MandelbrotPixelColor8AVX1Intrin(x, y, result)
  #else
  #define FractalPixelColor(x, y, result, screenCoords) MandelbrotPixelColor8AVX1(x, y, result)
  #endif

#elif SUPPORTED_IMPLEMENTATION == FRACTAL_FLOAT_AVX2_ASM
  #if PREFER_INTRINSICS
  #define FractalPixelColor(x, y, result, screenCoords) MandelbrotPixelColor8AVX2Intrin(x, y, result)
  #else
  #define FractalPixelColor(x, y, result, screenCoords) MandelbrotPixelColor8AVX2(x, y, result)
  #endif

#elif SUPPORTED_IMPLEMENTATION == FRACTAL_FLOAT_AVX512_ASM
  #if PREFER_INTRINSICS
  #define FractalPixelColor(x, y, result, screenCoords) MandelbrotPixelColor16AVX512FIntrin(x, y, result)
  #else
  #define FractalPixelColor(x, y, result, screenCoords) MandelbrotPixelColor16AVX512F(x, y, result)
  #endif

#elif SUPPORTED_IMPLEMENTATION == FRACTAL_FLOAT_AVX1024_ASM
  #if PREFER_INTRINSICS
  #define FractalPixelColor(x, y, result, screenCoords) MandelbrotPixelColor32AVX1024Intrin(x, y, result)
  #else
  #define FractalPixelColor(x, y, result, screenCoords) MandelbrotPixelColor32AVX1024(x, y, result)
  #endif
#endif

#if !defined(FractalDraw) || defined(MANDEL_MARIANI_SILVER_SUBDIVISION)
  #undef FractalDraw
  #define FractalDraw(l,u,r,b) (FractalDraw)(l,u,r,b)
#endif


/* Exports: */
extern complex_t
  leftUpper [],
  rightLower [];

/* Statistics: */
extern complex_t
  minInSet,
  maxInSet;
extern unsigned long long
  add_count,
  mul_count;



/* Function declarations: */
extern void
  FractalSetView (int view, BOOL forceReinit);
extern void
  FractalGetMousePos (int x, int y, float *real, float *imag);
extern float
  FractalZoom (int x, int y, int direction);

/* Fractals: */
#if TARGETED_IMPLEMENTATION == FRACTAL_FLOAT_ADT
extern unsigned long
  MandelbrotPixelColorADT (float x, float y, BOOL screenCoords);
extern unsigned long
  JuliaPixelColorSimpleADT (float x, float y, BOOL screenCoords);
#endif

#if TARGETED_IMPLEMENTATION == FRACTAL_FLOAT_INLINE
extern unsigned long
  MandelbrotPixelColorINL (float x, float y, BOOL screenCoords);
extern unsigned long
  JuliaPixelColorINL (float x, float y, BOOL screenCoords);
#endif

#endif
