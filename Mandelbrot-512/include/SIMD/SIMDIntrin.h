#ifndef _SIMD_INTRINSICS_H_
#define _SIMD_INTRINSICS_H_

#include "SIMDetect.h"


/* SIMD data types: */
#if !defined(SIMD_INC_INTRIN_HEADERS_SEPARATELY)
  /* Microsoft Visual C++? */
  #if defined(_MSC_VER)
    #include <intrin.h>

    #if _MSC_VER >= 1500 && _MSC_VER < 1600  /* in VS2008, <intrin.h> doesn't really include everything */
    #include <smmintrin.h>
    #endif

  /* GCC-compatible compiler, targeting x86/x86-64? */
  #elif defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__))
    #include <x86intrin.h>
  #endif
#else
  #if INTRIN_MMX
  #include <mmintrin.h>
  #endif
  #if INTRIN_SSE
  #include <xmmintrin.h>
  #endif
  #if INTRIN_SSE2
  #include <emmintrin.h>
  #endif
  #if INTRIN_SSE3
  #include <pmmintrin.h>
  #endif
  #if INTRIN_SSSE3
  #include <tmmintrin.h>
  #endif
  #if INTRIN_SSE41
  #include <smmintrin.h>
  #endif
  #if INTRIN_SSE42
  #include <nmmintrin.h>
  #endif
  #if INTRIN_SSE4A
  #include <ammintrin.h>
  #endif
  #if INTRIN_AES_NI
  #include <wmmintrin.h>
  #endif
  #if INTRIN_AVX1 || INTRIN_AVX2 || INTRIN_FMA4 || INTRIN_FMA3
  #include <immintrin.h>
  #endif
  #if INTRIN_AVX512
  #include <zmmintrin.h>
  #endif
#endif

#endif
