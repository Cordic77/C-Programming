#ifndef _SIMD_TYPES_H_
#define _SIMD_TYPES_H_

#include "SIMDetect.h"


/* GCC equivalent for m256_f32? https://stackoverflow.com/questions/13603319/gcc-equivalent-for-m256-f32

   E.g. xmmintrin.h:
   typedef float __m128 __attribute__ ((__vector_size__ (16), __may_alias__))

   … because of this, we define our own SIMD types under Linux (as if the
   mmintrin headers weren't available)!
*/

/* Structures: padding, alignment, visibility: */
#if !defined(PACKALIGN)
  #if defined(_MSC_VER)
    #define PACK(DECLARATION)                 \
  __pragma(pack (push, 1))                    \
  DECLARATION;                                \
  __pragma(pack (pop))

    #define PACKALIGN(ALIGN,DECLARATION)      \
  __pragma(pack (push, 1))                    \
  _declspec(align (ALIGN))                    \
  DECLARATION;                                \
  __pragma(pack (pop))
  #else
    #define PACK(DECLARATION)                 \
  DECLARATION                                 \
  __attribute__ ((packed));

    #define PACKALIGN(ALIGN,DECLARATION)      \
  DECLARATION                                 \
  __attribute__ ((aligned (ALIGN), packed));
  #endif
#endif


/* MMX – mmintrin.h: */
#if INTRIN_MMX && defined(_MSC_VER)
  #include <mmintrin.h>          /* Microsoft Visual C++ defines __m64 as a union type! */
#else
  PACKALIGN(16, union __m64 {
    unsigned long long  m64_u64;
    float               m64_f32 [2];
    signed char         m64_i8  [8];
    signed short        m64_i16 [4];
    signed int          m64_i32 [2];
    signed long long    m64_i64;
    unsigned char       m64_u8  [8];
    unsigned short      m64_u16 [4];
    unsigned int        m64_u32 [2];
  })
  typedef union __m64 __m64;
#endif


/* SSE – xmmintrin.h: */
#if INTRIN_SSE && defined(_MSC_VER)
  #include <xmmintrin.h>          /* Microsoft Visual C++ defines __m128 as a union type! */
#else
  PACKALIGN(16, union __m128 {
    float               m128_f32 [4];
    unsigned long long  m128_u64 [2];
    signed char         m128_i8  [16];
    signed short        m128_i16 [8];
    signed int          m128_i32 [4];
    signed long long    m128_i64 [2];
    unsigned char       m128_u8  [16];
    unsigned short      m128_u16 [8];
    unsigned int        m128_u32 [4];
  })
  typedef union __m128 __m128;
#endif


/* SSE2 – emmintrin.h: */
#if INTRIN_SSE2 && defined(_MSC_VER)
  #include <emmintrin.h>          /* Microsoft Visual C++ defines __m128d and __m128i as structure/union types! */
#else
  PACKALIGN(16, union __m128i {
    signed char         m128i_i8  [16];
    signed short        m128i_i16 [8];
    signed int          m128i_i32 [4];
    signed long long    m128i_i64 [2];
    unsigned char       m128i_u8  [16];
    unsigned short      m128i_u16 [8];
    unsigned int        m128i_u32 [4];
    unsigned long long  m128i_u64 [2];
  })
  typedef union __m128i __m128i;

  PACKALIGN(16, struct __m128d {
    double              m128d_f64 [2];
  })
  typedef struct __m128d __m128d;
#endif


/* AVX1/AVX2 – immintrin.h: */
#if INTRIN_AVX1 && defined(_MSC_VER)
  #include <immintrin.h>          /* Microsoft Visual C++ defines __m256d and __m256, __m256i as structure/union types! */
#else
  PACKALIGN(32, union m256 {
    float               m256_f32 [8];
  })
  typedef union m256 __m256;

  PACKALIGN(32, struct m256d {
    double              m256d_f64 [4];
  })
  typedef struct m256d __m256d;
/*#if INTRIN_AVX2 && defined(_MSC_VER)*/  /* Even AVX1 included some support for m256i! */
  PACKALIGN(32, union m256i {
    signed char         m256i_i8  [32];
    signed short        m256i_i16 [16];
    signed int          m256i_i32 [8];
    signed long long    m256i_i64 [4];
    unsigned char       m256i_u8  [32];
    unsigned short      m256i_u16 [16];
    unsigned int        m256i_u32 [8];
    unsigned long long  m256i_u64 [4];
  })
  typedef union m256i __m256i;
#endif


/* AVX512 – zmmintrin.h: */
#if INTRIN_AVX512 && defined(_MSC_VER)
  #include <zmmintrin.h>          /* Microsoft Visual C++ defines __m512d and __m512, __m512i as structure/union types! */
#else
  PACKALIGN(64, union m512 {
    float               m512_f32 [16];
  })
  typedef union m512 __m512;

  PACKALIGN(64, struct m512d {
    double              m512d_f64 [8];
  })
  typedef struct m512d __m512d;

  PACKALIGN(64, union m512i {
    signed char         m512i_i8  [64];
    signed short        m512i_i16 [32];
    signed int          m512i_i32 [16];
    signed long long    m512i_i64 [8];
    unsigned char       m512i_u8  [64];
    unsigned short      m512i_u16 [32];
    unsigned int        m512i_u32 [16];
    unsigned long long  m512i_u64 [8];
  })
  typedef union m512i __m512i;
#endif


/* AVX1024: */
#if INTRIN_AVX1024 && defined(_MSC_VER)
  #include <
mmintrin.h>          /* Microsoft Visual C++ defines __m1024d and __m1024, __m1024i as structure/union types! */
#else
  PACKALIGN(128, union m1024 {
    float               m1024_f32 [32];
  })
  typedef union m1024 __m1024;

  PACKALIGN(128, struct m1024d {
    double              m1024d_f64[16];
  })
  typedef struct m1024d __m1024d;

  PACKALIGN(128, union m1024i {
    signed char         m1024i_i8  [128];
    signed short        m1024i_i16 [64];
    signed int          m1024i_i32 [32];
    signed long long    m1024i_i64 [16];
    unsigned char       m1024i_u8  [128];
    unsigned short      m1024i_u16 [64];
    unsigned int        m1024i_u32 [32];
    unsigned long long  m1024i_u64 [16];
  })
  typedef union m1024i __m1024i;
#endif

#endif
