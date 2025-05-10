#ifndef _DRAW_FRACTAL_H_
#define _DRAW_FRACTAL_H_

/* Selected (SIMD) target implementation? */
#if TARGETED_IMPLEMENTATION == FRACTAL_FLOAT_AVX1024_ASM
  #define SIMD_FLT      __m1024
  #define SIMD_DBL      __m1024d
  #define SIMD_INT      __m1024i

  #define _f            m1024_f32
  #define _d            m1024d_f64
  #define _b            m1024i_i8
  #define _ub           m1024i_u8
  #define _w            m1024i_i16
  #define _uw           m1024i_u16
  #define _i            m1024i_i32
  #define _ui           m1024i_u32
  #define _l            m1024i_i64
  #define _ul           m1024i_u64

  #define SIMD_FLT_COUNT    32    /* AVX1024: 32 float to one AVX1024 register */

#elif TARGETED_IMPLEMENTATION == FRACTAL_FLOAT_AVX512_ASM
  #define SIMD_FLT      __m512
  #define SIMD_DBL      __m512d
  #define SIMD_INT      __m512i

  #define _f            m512_f32
  #define _d            m512d_f64
  #define _b            m512i_i8
  #define _ub           m512i_u8
  #define _w            m512i_i16
  #define _uw           m512i_u16
  #define _i            m512i_i32
  #define _ui           m512i_u32
  #define _l            m512i_i64
  #define _ul           m512i_u64

  #define SIMD_FLT_COUNT    16    /* AVX512: 16 float to one AVX512 register */

#elif TARGETED_IMPLEMENTATION >= FRACTAL_FLOAT_AVX1_ASM  /* AVX1 <or> AVX2? */
  #define SIMD_FLT      __m256
  #define SIMD_DBL      __m256d
  #define SIMD_INT      __m256i

  #define _f            m256_f32
  #define _d            m256d_f64
  #define _b            m256i_i8
  #define _ub           m256i_u8
  #define _w            m256i_i16
  #define _uw           m256i_u16
  #define _i            m256i_i32
  #define _ui           m256i_u32
  #define _l            m256i_i64
  #define _ul           m256i_u64

  #define SIMD_FLT_COUNT    8     /* AVX: 8 float to one AVX(2) register */

#else
  #if SUPPORTED_IMPLEMENTATION == FRACTAL_FIXED_ASM
  #define SIMD_FLT      __m128i
  #define SIMD_DBL      __m128i
  #define SIMD_INT      __m128i

  #define _f            m128i_i32
  #define _d            m128i_i64
  #else
  #define SIMD_FLT      __m128
  #define SIMD_DBL      __m128d
  #define SIMD_INT      __m128i

  #define _f            m128_f32
  #define _d            m128d_f64
  #endif
  #define _b            m128i_i8
  #define _ub           m128i_u8
  #define _w            m128i_i16
  #define _uw           m128i_u16
  #define _i            m128i_i32
  #define _ui           m128i_u32
  #define _l            m128i_i64
  #define _ul           m128i_u64

  #if SUPPORTED_IMPLEMENTATION == FRACTAL_FLOAT_SSE1_ASM || defined(MANDEL_MARIANI_SILVER_SUBDIVISION)
  #define SIMD_FLT_COUNT    4     /* SSE1-SSE4: 4 float to one SSE register */
  #else
  #define SIMD_FLT_COUNT    1     /* GPR: 1 float at a time */
  #endif
#endif

/* How many coordinates can be processed in parallel? */
#define SIMD_COUNT(simdtype,elemtype) ((int)(sizeof(simdtype) / sizeof(elemtype)))
#define SIMD_PARALLEL() SIMD_COUNT(SIMD_FLT,float)
#define SIMD_MINSIZE()  ((SIMD_PARALLEL() / 4) + 1)


/* Structures: */
struct FRACTAL_COORDS
{
  /* Iteration: */
  SIMD_FLT
    pt_real,                /* [in]   SIMD array of initial real coordinates */
    pt_imag;                /* [in]   SIMD array of initial imaginary coordinates */
  SIMD_INT
    rect_clr,               /* [out]  Mariani/Silver: SIMD array of calculated bounding box color */
    pt_clr;                 /* [out]  SIMD array of calculated pixel color */

  /* Coordinates: */
  SCREEN_MAPPING
    c_real,                 /* Real coordinate of left screen border (with respect to the current zoom level) */
    c_curr,                 /* Current real coordinate (within the current screen line) */
    c_imag;                 /* Current imaginary coordinate (within the current screen line) */
  char
    align [16];
}; /* [alignment] sizeof(struct FRACTAL_COORDS)=96 <? 3*sizeof(__m256) */


/* Imports: */
extern SCREEN_MAPPING
  real_factor,
  imag_factor;
extern int
  nrThreads,
  maxThreads;
extern BOOL
  quit;


/* Function declarations: */
extern BOOL
  (FractalDraw) (int l, int u, int r, int b);

#endif
