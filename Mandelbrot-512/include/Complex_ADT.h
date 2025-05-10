#ifndef _COMPLEX_H_
#define _COMPLEX_H_

#include "Config/Compiler.h"

/* Preprocessor macros: */
#if SUPPORTED_IMPLEMENTATION == FRACTAL_FIXED_ASM
  #define C99COMPLEX_AVAILABLE 0
  #define COMPLEX_BASE_T signed long

  #define ST_FLOAT(floatVal) (*(signed int *)&floatVal)
  #define LD_FLOAT(floatVal) (*(float *)&floatVal)

  #define FLOAT2FIXED(floatVal) ((signed int)(floatVal * 8192.0f))
  #define FIXED2FLOAT(fixedVal) ((float)fixedVal / 8192.0f)
#else
  #if C_STD >= 1999
  #include <complex.h>
  #define C99COMPLEX_AVAILABLE 1
  #else
  #define C99COMPLEX_AVAILABLE 0
  #endif
  #define COMPLEX_BASE_T float

  #define ST_FLOAT(floatVal) (floatVal)
  #define LD_FLOAT(floatVal) (floatVal)

  #define FLOAT2FIXED(floatVal) (floatVal)
  #define FIXED2FLOAT(fixedVal) (fixedVal)
#endif


/* Structures: */
#if C99COMPLEX_AVAILABLE
  #if defined(_MSC_VER)
    typedef _Fcomplex complex_t;
    #define COMPLEX_ZERO { 0.0f, 0.0f }

    #define CMPLXF(real, imag) _FCbuild(real, imag)
    #define _FCaddcc(_Fcomplex_X, _Fcomplex_Y) _FCbuild(crealf (_Fcomplex_X) + crealf (_Fcomplex_Y), \
                                                        cimagf (_Fcomplex_X) + cimagf (_Fcomplex_Y))

    #define set_crealf(_Fcomplex_Z, realf_val) _Fcomplex_Z._Val [0] = realf_val
    #define set_cimagf(_Fcomplex_Z, imagf_val) _Fcomplex_Z._Val [1] = imagf_val
  #else
    typedef float _Complex complex_t;
    #define COMPLEX_ZERO CMPLXF(0.0f, 0.0f)

    #if defined(__GNUC__)
    #define set_crealf(_Fcomplex_Z, realf_val) __real__ _Fcomplex_Z = realf_val
    #define set_cimagf(_Fcomplex_Z, imagf_val) __imag__ _Fcomplex_Z = imagf_val
    #else
    #define set_crealf(_Fcomplex_Z, realf_val) ((float *)&_Fcomplex_Z) [0] = realf_val
    #define set_cimagf(_Fcomplex_Z, imagf_val) ((float *)&_Fcomplex_Z) [1] = imagf_val
    #endif
  #endif
#else
  struct Complex
  {
    #if SUPPORTED_IMPLEMENTATION == FRACTAL_FIXED_ASM
    signed int real;          /* real part (fixed-point) */
    signed int imag;          /* imaginary part (fixed-point) */
    #else
    float real;               /* real part (floating-point) */
    float imag;               /* imaginary part (floating-point) */
    #endif
  };
  typedef struct Complex complex_t;

  #define COMPLEX_ZERO { 0, 0 }

  #define crealf(_Fcomplex_Z) _Fcomplex_Z.real
  #define cimagf(_Fcomplex_Z) _Fcomplex_Z.imag

  #define set_crealf(_Fcomplex_Z, realf_val) _Fcomplex_Z.real = realf_val
  #define set_cimagf(_Fcomplex_Z, imagf_val) _Fcomplex_Z.imag = imagf_val
#endif


/* Exports: */
extern complex_t const
  complexZero;


/* Function declarations: */
#if SUPPORTED_IMPLEMENTATION == FRACTAL_FLOAT_ADT
extern complex_t
  AddComplex (complex_t const *c1, complex_t const *c2);
extern complex_t
  MulComplex (complex_t const *c1, complex_t const *c2);
extern double
  AbsComplex (complex_t const *c);
#endif

#endif
