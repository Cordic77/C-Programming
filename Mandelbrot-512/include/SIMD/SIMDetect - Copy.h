#ifndef _SIMD_DETECT_H_
#define _SIMD_DETECT_H_

/* Header files for x86 SIMD intrinsics?
   https://stackoverflow.com/questions/11228855/header-files-for-x86-simd-intrinsics#answer-11228864 */
/*                                               GCC     VS9_2008
                        <mmintrin.h>   MMX       3.1     Microsoft Visual C++ Processor Pack Release (with SP5) for Visual C++ 6.0¹
                        <xmmintrin.h>  SSE       3.1     (Had to be installed separately; realistically speaking, MMX, SSE/SSE2
                        <emmintrin.h>  SSE2      3.1      support was only available from Visual Studio 2002 onwards.)
                        <pmmintrin.h>  SSE3      3.3.3   VS8_2005SP1
                        <tmmintrin.h>  SSSE3     4.4.3   VS9_2008²
                        <smmintrin.h>  SSE4.1    4.3     VS9_2008
                        <nmmintrin.h>  SSE4.2    4.3     VS9_2008
 [AMD only]             <ammintrin.h>  SSE4A     4.3     VS10SP1(?)
                        <wmmintrin.h>  AES-NI    4.4.0  VS9_2008SP1
                        <immintrin.h>  AVX       4.6.0  VS10_2010
 [initially AMD only]                  FMA4      4.5.0  VS10_2010SP1
 [initially Intel only]                FMA3      4.7.0  VS11_2012
                        <immintrin.h>  AVX2      4.7.0  VS12_2013U2
                        <zmmintrin.h>  AVX512    4.9.0  VS15_2017
                                       AVX1024?   ???      ???

 ¹ https://bytepointer.com/masm/vcpp5_readme.htm
 ² https://www.g-truc.net/post-0359.html
*/

#if defined(__GNUC__)
  #ifndef GCC_VER
  #define GCC_VER  (__GNUC__*10000 + __GNUC_MINOR__*100 + __GNUC_PATCHLEVEL__)
  #endif

  #if GCC_VER >= 30100
  #define INTRIN_MMX     1
  #define INTRIN_SSE     1
  #define INTRIN_SSE2    1
  #endif
  #if GCC_VER >= 30303
  #define INTRIN_SSE3    1
  #endif
  #if GCC_VER >= 40300
  #define INTRIN_SSE41   1
  #define INTRIN_SSE42   1
  #define INTRIN_SSE4A   1
  #endif
  #if GCC_VER >= 40400
  #define INTRIN_AES_NI  1
  #endif
  #if GCC_VER >= 40403
  #define INTRIN_SSSE3   1
  #endif
  #if GCC_VER >= 40500
  #define INTRIN_FMA4    1
  #endif
  #if GCC_VER >= 40600
  #define INTRIN_AVX1    1
  #endif
  #if GCC_VER >= 40700
  #define INTRIN_AVX2    1
  #define INTRIN_FMA3    1
  #endif
  #if GCC_VER >= 40900
  #define INTRIN_AVX512  1
  #endif
#else
  #if _MSC_VER >= 1300  /* Visual Studio 2002 */
  #define INTRIN_MMX     1
  #define INTRIN_SSE     1
  #define INTRIN_SSE2    1
  #endif
  #if _MSC_VER >= 1400 && _MSC_FULL_VER >= 140050727  /* Visual Studio 2005 SP1 */
  #define INTRIN_SSE3    1
  #endif
  #if _MSC_VER >= 1500  /* Visual Studio 2008 */
  #define INTRIN_SSSE3   1
  #define INTRIN_SSE41   1
  #define INTRIN_SSE42   1
  #if _MSC_FULL_VER >= 150030729  /* Visual Studio 2008 SP1 */
  #define INTRIN_AES_NI  1
  #endif
  #endif
  /* “In VS2010 release, all AVX features and instructions are fully supported via intrinsic and /arch:AVX.”
     https://blogs.msdn.microsoft.com/vcblog/2009/11/02/visual-c-code-generation-in-visual-studio-2010/ */
  #if _MSC_VER >= 1600  /* Visual Studio 2010 */
  #define INTRIN_AVX1    1
  #endif
  #if _MSC_FULL_VER >= 160040219  /* Visual Studio 2010 SP1 */
  #define INTRIN_FMA4    1
  #endif
  #if _MSC_VER >= 1700  /* Visual Studio 2012 */
  #define INTRIN_FMA3    1
  #endif
  /* https://docs.microsoft.com/en-us/cpp/build/reference/arch-x86
     “The /arch:AVX2 option was introduced in Visual Studio 2013 Update 2” */
  #if _MSC_VER >= 1800 && _MSC_FULL_VER >= 180030501  /* Visual Studio 2013 (Update 2) */
  #define INTRIN_AVX2    1
  #endif
  /* https://blogs.msdn.microsoft.com/vcblog/2017/07/11/microsoft-visual-studio-2017-supports-intel-avx-512/
     “Microsoft Visual Studio 2017 Supports Intel AVX-512” */
  #if _MSC_VER >= 1910  /* Visual Studio 2017 */
  #define INTRIN_AVX512  1
  #endif
#endif

#ifndef INTRIN_MMX
#define INTRIN_MMX     0
#endif
#ifndef INTRIN_SSE
#define INTRIN_SSE     0
#endif
#ifndef INTRIN_SSE2
#define INTRIN_SSE2    0
#endif
#ifndef INTRIN_SSE3
#define INTRIN_SSE3    0
#endif
#ifndef INTRIN_SSE41
#define INTRIN_SSE41   0
#endif
#ifndef INTRIN_SSE42
#define INTRIN_SSE42   0
#endif
#ifndef INTRIN_SSE4A
#define INTRIN_SSE4A   0
#endif
#ifndef INTRIN_AES_NI
#define INTRIN_AES_NI  0
#endif
#ifndef INTRIN_SSSE3
#define INTRIN_SSSE3   0
#endif
#ifndef INTRIN_FMA4
#define INTRIN_FMA4    0
#endif
#ifndef INTRIN_AVX1
#define INTRIN_AVX1    0
#endif
#ifndef INTRIN_AVX2
#define INTRIN_AVX2    0
#endif
#ifndef INTRIN_FMA3
#define INTRIN_FMA3    0
#endif
#ifndef INTRIN_AVX512
#define INTRIN_AVX512  0
#endif
#ifndef INTRIN_AVX1024
#define INTRIN_AVX1024 0
#endif

#endif
