#include "stdinc.h"

#include "SIMD/SIMDIntrin.h"


/* Externs: */
extern int
  vmode_colors;

extern __m128 const
  escape_radius;


/* Function definitions: */
#if defined(_MSC_VER)
  #pragma warning (push)
  #pragma warning (disable : 4133)  /* warning C4133: 'function' : incompatible types - from '__m128 *' to 'const float *' */

#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wincompatible-pointer-types"
#endif

/* XMM7  | c_imag        |
   XMM6  | c_real        |
   XMM5  | Z_imag        |  NEW(Z_imag)
   XMM4  | Z_real        |  NEW(Z_real)
   -----------------------
   XMM3  | iter_count    |
   XMM2  | iter_incr     |
   XMM1  | radius        |  <temporary to each iteration>
   XMM0  | Bkp(Z_imag^2) |  <temporary to each iteration>

 Benchmark (Z70, Core i7 5500U @ 2,40 GHz, SDL 2.09, single-threaded, Mariani-Silver disabled):
 Linux-1804 (x64)
 » MandelbrotPixelColor4SSE1       :  16.47 fps
 » MandelbrotPixelColor4SSE1Intrin :  16.04 fps  (~ 2.6% slower)
 Windows 10-1809 (x64)
 » MandelbrotPixelColor4SSE1       :  16.73 fps
 » MandelbrotPixelColor4SSE1Intrin :  17.33 fps  (~ 3.6% faster)
*/
#if INTRIN_SSE41
EXTERN_C void MandelbrotPixelColor4SSE1Intrin (__m128 *in_real, __m128 *in_imag, __m128i *result)
{ __m128
    radius,
    backup;
  int
    i = vmode_colors-1;

  __m128i iter_cnt = _mm_cvtsi32_si128 (i);             /* movd     ; XMM3[31..0] = vmode_colors-1 */
  __m128i iter_inc = _mm_cmpeq_epi32(iter_cnt,iter_cnt);/* pcmpeqd  ; XMM2(iter_inc) = <iteration increment> */
  iter_cnt = _mm_shuffle_epi32 (iter_cnt, 0);           /* pshufd   ; Shuffle low 32 bits into all vector positions */

  { __m128 c_real   = _mm_load_ps (in_real);            /* movaps   ; XMM6(c_real) */
    __m128 c_imag   = _mm_load_ps (in_imag);            /* movaps   ; XMM7(c_imag) */
    __m128 Z_real   = _mm_load_ps (&c_real);            /* movaps   ; XMM4(Z_real) = c_real */
    __m128 Z_imag   = _mm_load_ps (&c_imag);            /* movaps   ; XMM5(Z_imag) = c_imag */

    do {
      radius = _mm_load_ps (&Z_imag);                   /* movaps   ; XMM1(radius) */
      radius = _mm_mul_ps (radius, radius);             /* mulps    ; XMM1(radius) = Z_imag^2 */
      Z_imag = _mm_add_ps (Z_imag, Z_imag);             /* addps    ; XMM5(NEW(Z_imag)) = 2*Z_imag */
      Z_imag = _mm_mul_ps (Z_imag, Z_real);             /* mulps    ; XMM5(NEW(Z_imag)) = 2*Z_imag*Z_real */
      backup = _mm_load_ps (&radius);                   /* movaps   ; XMM0(Backup) = Z_imag^2 */
      backup = _mm_sub_ps (backup, c_real);             /* subps    ; XMM0(Backup) = Z_imag^2 - c_real [Variant 2: faster to subtract c_real, than to add it later.] */
      Z_real = _mm_mul_ps (Z_real, Z_real);             /* mulps    ; XMM4(NEW(Z_real)) = Z_Real^2 */
      Z_imag = _mm_add_ps (Z_imag, c_imag);             /* addps    ; XMM5(NEW(Z_imag)) = 2*Z_imag*Z_real + c_imag */
      radius = _mm_add_ps (radius, Z_real);             /* addps    ; XMM1(radius) = Z_imag^2 + Z_real^2 */
      Z_real = _mm_sub_ps (Z_real, backup);             /* subps    ; XMM4(NEW(Z_real)) = Z_real^2 - Z_imag^2 (+ c_real) */

      /* The result of each comparison is a doubleword mask of all 1s (comparison true) or all 0s (comparison false): */
      radius = _mm_cmple_ps (radius, escape_radius);            /* cmpleps ; XMM0 = radius <= 4.0? */

      iter_inc = _mm_and_si128 (iter_inc, *(__m128i *)&radius); /* pand; XMM2(iter_inc) &= (radius <= 4.0) */
      iter_cnt = _mm_add_epi32 (iter_cnt, iter_inc);            /* paddd    ; XMM3(iter_count) -= 1 */

    } while (--i > 0 && _mm_test_all_zeros (iter_inc, iter_inc) == 0); /* ptest ; --ECX > 0 && (ZF = (XMM2 & XMM2)) == 0 */
  }

  _mm_store_si128 (result, iter_cnt);                   /* movdqa   ; *result = XMM3 */
}
#endif


/* YMM7  | c_imag        |
   YMM6  | c_real        |
   YMM5  | Z_imag        |  NEW(Z_imag)
   YMM4  | Z_real        |  NEW(Z_real)
   -----------------------
   YMM3  | iter_count    |
   YMM2  | iter_incr     |
   YMM1  | radius        |  <temporary to each iteration>
   YMM0  | Bkp(Z_imag^2) |  <temporary to each iteration>

 Benchmark (Z70, Core i7 5500U @ 2,40 GHz, SDL 2.09, single-threaded, Mariani-Silver disabled):
 Linux-1804 (x64)
 » MandelbrotPixelColor4SSE1       :  29.61 fps
 » MandelbrotPixelColor4SSE1Intrin :  28.41 fps  (~ 4.0% slower)
 Windows 10-1809 (x64)
 » MandelbrotPixelColor8AVX1       :  30.56 fps
 » MandelbrotPixelColor8AVX1Intrin :  29.86 fps  (~ 2.3 % slower)
*/
#if INTRIN_AVX1
EXTERN_C void MandelbrotPixelColor8AVX1Intrin (__m256 *in_real, __m256 *in_imag, __m256i *result)
{ __m256
    radius,
    backup;
  int
    i = vmode_colors-1;

  __m256 escp_rad = _mm256_broadcast_ss(&escape_radius);/* broadcastss ; YMM8(radius) = 4.0 */
  __m256 iter_cnt = _mm256_set1_ps ((float)i);          /* broadcastss ; YMM3(iter_count) */
  __m256 iter_inc = _mm256_set1_ps ((float)-1.0);       /* broadcastss ; YMM2(iter_inc) = <iteration increment> */

  { __m256 c_real = _mm256_load_ps (in_real);           /* vmovaps  ; YMM6(c_real) */
    __m256 c_imag = _mm256_load_ps (in_imag);           /* vmovaps  ; YMM7(c_imag) */
    __m256 Z_real = _mm256_load_ps (&c_real);           /* vmovaps  ; YMM4(Z_real) = c_real */
    __m256 Z_imag = _mm256_load_ps (&c_imag);           /* vmovaps  ; YMM5(Z_imag) = c_imag */

    do {
      radius = _mm256_load_ps (&Z_imag);                /* vmovaps  ; YMM1(radius) */
      radius = _mm256_mul_ps (radius, radius);          /* vmulps   ; YMM1(radius) = Z_imag^2 */
      Z_imag = _mm256_add_ps (Z_imag, Z_imag);          /* vaddps   ; YMM5(NEW(Z_imag)) = 2*Z_imag */
      Z_imag = _mm256_mul_ps (Z_imag, Z_real);          /* vmulps   ; YMM5(NEW(Z_imag)) = 2*Z_imag*Z_real */
      backup = _mm256_load_ps (&radius);                /* vmovaps  ; YMM0(Backup) = Z_imag^2 */
      backup = _mm256_sub_ps (backup, c_real);          /* vsubps   ; YMM0(Backup) = Z_imag^2 - c_real [Variant 2: faster to subtract c_real, than to add it later.] */
      Z_real = _mm256_mul_ps (Z_real, Z_real);          /* vmulps   ; YMM4(NEW(Z_real)) = Z_Real^2 */
      Z_imag = _mm256_add_ps (Z_imag, c_imag);          /* vaddps   ; YMM5(NEW(Z_imag)) = 2*Z_imag*Z_real + c_imag */
      radius = _mm256_add_ps (radius, Z_real);          /* vaddps   ; YMM1(radius) = Z_imag^2 + Z_real^2 */
      Z_real = _mm256_sub_ps (Z_real, backup);          /* vsubps   ; YMM4(NEW(Z_real)) = Z_real^2 - Z_imag^2 (+ c_real) */

      /* The result of each comparison is a doubleword mask of all 1s (comparison true) or all 0s (comparison false): */
      radius = _mm256_cmp_ps (radius, escp_rad, _CMP_LE_OS); /* vcmpleps ; YMM0 = radius <= 4.0? */

      iter_inc = _mm256_and_ps (iter_inc, radius);      /* vandps   ; YMM2(iter_inc) &= (radius <= 4.0) */
      iter_cnt = _mm256_add_ps (iter_cnt, iter_inc);    /* vaddps   ; YMM3(iter_count) -= 1 */

    } while (--i > 0 && _mm256_testz_si256 (*(__m256i *)&iter_inc,
                                            *(__m256i *)&iter_inc) == 0); /* vptest ; --ECX > 0 && (ZF = (YMM2 & YMM2)) == 0 */
  }

  *result = _mm256_cvttps_epi32 (iter_cnt);             /* vcvttps2dq ; YMM0 = Truncate(YMM3) */
}
#endif


/* YMM7  | c_imag        |
   YMM6  | c_real        |
   YMM5  | Z_imag        |  NEW(Z_imag)
   YMM4  | Z_real        |  NEW(Z_real)
   -----------------------
   YMM3  | iter_count    |
   YMM2  | iter_incr     |
   YMM1  | radius        |  <temporary to each iteration>
   YMM0  | Bkp(Z_imag^2) |  <temporary to each iteration>

 Benchmark (Z70, Core i7 5500U @ 2,40 GHz, SDL 2.09, single-threaded, Mariani-Silver disabled):
 Linux-1804 (x64)
 » MandelbrotPixelColor4SSE1       :  27.06 fps
 » MandelbrotPixelColor4SSE1Intrin :  27.11 fps  (~ 0.18% faster)
 Windows 10-1809 (x64)
 » MandelbrotPixelColor8AVX2       :  28.21 fps
 » MandelbrotPixelColor8AVX2Intrin :  30.10 fps  (~ 6.7% faster)
*/
#if INTRIN_AVX1
EXTERN_C void MandelbrotPixelColor8AVX2Intrin (__m256 *in_real, __m256 *in_imag, __m256i *result)
{ __m256
    radius,
    backup;
  int
    i = vmode_colors-1;

  __m256 escp_rad = _mm256_broadcast_ss(&escape_radius);/* broadcastss  ; YMM8(radius) = 4.0 */
  __m256i iter_cnt = _mm256_set1_epi32 (i);             /* vpbroadcastd ; YMM3(iter_count) = vmode_colors-1 */
  __m256i iter_inc = _mm256_cmpeq_epi32 (iter_cnt,iter_cnt);/* vpcmpeqd ; YMM2(iter_inc) = -1 */

  { __m256 c_real = _mm256_load_ps (in_real);           /* vmovaps  ; YMM6(c_real) */
    __m256 c_imag = _mm256_load_ps (in_imag);           /* vmovaps  ; YMM7(c_imag) */
    __m256 Z_real = _mm256_load_ps (&c_real);           /* vmovaps  ; YMM4(Z_real) = c_real */
    __m256 Z_imag = _mm256_load_ps (&c_imag);           /* vmovaps  ; YMM5(Z_imag) = c_imag */

    do {
      radius = _mm256_load_ps (&Z_imag);                /* vmovaps     ; YMM1(radius) */
      radius = _mm256_mul_ps (radius, radius);          /* vmulps      ; YMM1(radius) = Z_imag^2 */
      Z_imag = _mm256_add_ps (Z_imag, Z_imag);          /* vaddps      ; YMM5(NEW(Z_imag)) = 2*Z_imag */
      Z_imag = _mm256_fmadd_ps (Z_real, Z_imag, c_imag);/* vfmadd213ps ; YMM5(NEW(Z_imag)) = 2*Z_imag*Z_real + c_imag */
      backup = _mm256_load_ps (&radius);                /* vmovaps     ; YMM0(Backup) = Z_imag^2 */
      radius = _mm256_fmadd_ps (Z_real, Z_real, radius);/* vfmadd231ps ; YMM1(radius) = Z_imag^2 + Z_real^2 */
      Z_real = _mm256_fmsub_ps (Z_real, Z_real, backup);/* vfmsub213ps ; YMM4(NEW(Z_real)) = Z_real ^ 2 - Z_imag ^ 2 */
      Z_real = _mm256_add_ps (Z_real, c_real);          /* vaddps      ; YMM4(NEW(Z_real)) = Z_real ^ 2 - Z_imag ^ 2 + c_real */

      /* The result of each comparison is a doubleword mask of all 1s (comparison true) or all 0s (comparison false): */
      radius = _mm256_cmp_ps(radius, escp_rad, _CMP_LE_OS);        /* vcmpleps ; YMM0 = radius <= 4.0? */

      iter_inc = _mm256_and_si256 (iter_inc, *(__m256i *)&radius); /* vandps   ; YMM2(iter_inc) &= (radius <= 4.0) */
      iter_cnt = _mm256_add_epi32 (iter_cnt, iter_inc);            /* vpaddd   ; YMM3(iter_count) -= 1 */

    } while (--i > 0 && _mm256_testz_si256 (*(__m256i *)&iter_inc,
                                            *(__m256i *)&iter_inc) == 0); /* vptest ; --ECX > 0 && (ZF = (YMM2 & YMM2)) == 0 */
  }

  _mm256_store_si256 (result, iter_cnt);                /* movdqa      ; *result = YMM3 */
}
#endif


/* ZMM7  | c_imag        |
   ZMM6  | c_real        |
   ZMM5  | Z_imag        |  NEW(Z_imag)
   ZMM4  | Z_real        |  NEW(Z_real)
   -----------------------
   ZMM3  | iter_count    |
   ZMM2  | iter_incr     |
   ZMM1  | radius        |  <temporary to each iteration>
   ZMM0  | Bkp(Z_imag^2) |  <temporary to each iteration>

 Benchmark (Z70, Core i7 5500U @ 2,40 GHz, SDL 2.09, single-threaded, Mariani-Silver disabled):
 Linux-1804 (x64)
 » MandelbrotPixelColor4SSE1       :  ??.?? fps
 » MandelbrotPixelColor4SSE1Intrin :  ??.?? fps  (~ ?.?% faster)
 Windows 10-1809 (x64)
 » MandelbrotPixelColor8AVX2       :  ??.?? fps
 » MandelbrotPixelColor8AVX2Intrin :  ??.?? fps  (~ ?.?% faster)
*/
#if INTRIN_AVX512 && !defined(__GNUC__)  /* GCC: AVX512F vector return without AVX512F enabled changes the ABI */
EXTERN_C void MandelbrotPixelColor16AVX512FIntrin (__m512 *in_real, __m512 *in_imag, __m512i *result)
{ __m512
    radius,
    backup;
  int
    i = vmode_colors-1;

  __m512 escp_rad = _mm512_broadcastss_ps(escape_radius); /* vbroadcastss ; ZMM8(radius) = 4.0 */
  __m512i iter_cnt = _mm512_set1_epi32 (i);               /* vpbroadcastd ; YMM3(iter_count) */
  __m512i iter_inc = _mm512_set1_epi32 (-1);              /* vpbroadcastd ; YMM2(iter_inc) = <iteration increment> */

  { __m512 c_real = _mm512_load_ps (in_real);             /* vmovaps  ; ZMM6(c_real) */
    __m512 c_imag = _mm512_load_ps (in_imag);             /* vmovaps  ; ZMM7(c_imag) */
    __m512 Z_real = _mm512_load_ps (&c_real);             /* vmovaps  ; ZMM4(Z_real) = c_real */
    __m512 Z_imag = _mm512_load_ps (&c_imag);             /* vmovaps  ; ZMM5(Z_imag) = c_imag */
    __mmask16 k1  = _mm512_kmov ((__mmask16 )-1);         /* kmovw    ; set 16 bits in mask register k1 */

    do {
      radius = _mm512_maskz_load_ps (k1, &Z_imag);                 /* vmovaps{k1}{z}     ; ZMM1(radius) */
      radius = _mm512_mask_mul_ps (radius, k1, radius, radius);    /* vmulps{k1}         ; ZMM1(radius) = Z_imag^2 */
      Z_imag = _mm512_mask_add_ps (Z_imag, k1, Z_imag, Z_imag);    /* vaddps{k1}         ; ZMM5(NEW(Z_imag)) = 2*Z_imag */
      Z_imag = _mm512_maskz_fmadd_ps (k1, Z_real, Z_imag, c_imag); /* vfmadd213ps{k1}{z} ; ZMM5(NEW(Z_imag)) = 2*Z_imag*Z_real + c_imag */
      backup = _mm512_load_ps (&radius);                           /* vmovaps            ; ZMM0(Backup) = Z_imag^2 */
      radius = _mm512_mask_fmadd_ps (Z_real, k1, Z_real, radius);  /* vfmadd231ps{k1}    ; ZMM1(radius) = Z_imag^2 + Z_real^2 */
      Z_real = _mm512_mask_fmsub_ps (Z_real, k1, Z_real, backup);  /* vfmsub213ps{k1}    ; ZMM4(NEW(Z_real)) = Z_real ^ 2 - Z_imag ^ 2 */
      Z_real = _mm512_maskz_add_ps (k1, Z_real, c_real);           /* vaddps{k1}{z}      ; ZMM4(NEW(Z_real)) = Z_real ^ 2 - Z_imag ^ 2 + c_real */

      /* The result of each comparison is a doubleword mask of all 1s (comparison true) or all 0s (comparison false): */
      k1 = _mm512_mask_cmp_ps_mask (k1, radius, escp_rad, _CMP_LE_OS);/* vcmpleps k1{k1} ; ZMM0 = radius <= 4.0? */

    iter_cnt = _mm512_mask_add_epi32 (iter_cnt, k1, iter_cnt, iter_inc); /* vpaddd{k1} ; ZMM3(iter_count) -= 1 */

    } while (--i > 0 && _mm512_kortestz(k1, k1) != 0);    /* kortestw; --ECX > 0 && (ZF = !(k1 | k1)) != 0 */
  }

  _mm512_store_si512 (result, iter_cnt);                  /* movdqa             ; *result = ZMM3 */
}
#endif

#if defined(_MSC_VER)
  #pragma warning (pop)

#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
