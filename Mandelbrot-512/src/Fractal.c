#include <stdlib.h>               /* NULL */
#include <float.h>                /* FLT_MIN, FLT_MAX */
#include <math.h>                 /* fabs() */

#include <pthread.h>              /* pthread_create() */

#include "stdinc.h"
#include "Graphics.h"             /* screen_width, screen_height */
#include "Complex_ADT.h"           /* AddComplex(), MulComplex(), AbsComplex() */
#include "Fractal.h"

/* Preprocessor macros: */
#define MAX_ITERATIONS  (vmode_colors-1)  /* Calculation starts with step Z_1 (not Z_0)! */

/* Exports: */
complex_t
  leftUpper [] = {
    COMPLEX_ZERO,                 /* Temporary */
    COMPLEX_ZERO,                 /* Left upper corner – min. position: Mandelbrot & Julia */
  },
  rightLower [] = {
    COMPLEX_ZERO,                 /* Temporary */
    COMPLEX_ZERO,                 /* Right bottom corner – max. position: Mandelbrot & Julia */
  };
complex_t                         /* Min./max. coordinates within the fractal set? */
  minInSet = COMPLEX_ZERO,
  maxInSet = COMPLEX_ZERO;
SCREEN_MAPPING                    /* Factor to convert between pixel and fractal coordinates? */
  real_factor,
  imag_factor;

/* Statistics: */
#if defined(CALCULATE_OP_STATISTICS)
unsigned long long
  add_count = 0,                  /* Number of necessary additions */
  mul_count = 0;                  /* Number of necessary multiplications */
#endif

/* Globals: */
static int
  fractview = -1;                 /* Current view into fractal plane */
#if SUPPORTED_IMPLEMENTATION == FRACTAL_FLOAT_ADT
static const int
  incolor = 1;                    /* Draw in color? */
#endif


/* Function definitions: */
#if SUPPORTED_IMPLEMENTATION == FRACTAL_FIXED_ASM
  /*LD_FLOAT()/ST_FLOAT: dereferencing type-punned pointer will break strict-aliasing rules [-Werror=strict-aliasing] */
  #if defined(__GNUC__)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wstrict-aliasing"
  #endif
#endif
extern void FractalSetView (int view, BOOL forceReinit)
{ float f;

  if (view <= 0) view = max (fractview, 1);

  /* Initialize view? */
  forceReinit |= memcmp (leftUpper + view, &complexZero, sizeof(complexZero)) == 0
              && memcmp (rightLower + view, &complexZero, sizeof(complexZero)) == 0;

  if (forceReinit)
  {
    fractview = -1;
    #if C99COMPLEX_AVAILABLE  /* ⇔ SUPPORTED_IMPLEMENTATION != FRACTAL_FIXED_ASM, all LD_FLOAT()/ST_FLOAT() calls are therefore unnecessary! */
    leftUpper  [view] = CMPLXF (-2.008f,  1.400f);
    rightLower [view] = CMPLXF ( 1.657f, -1.400f);
    #else
    f = -2.008f; leftUpper[view].real = ST_FLOAT(f);
    f =  1.400f; leftUpper[view].imag = ST_FLOAT(f);
    f =  1.657f; rightLower[view].real = ST_FLOAT(f);
    f = -1.400f; rightLower[view].imag = ST_FLOAT(f);
    #endif
  }

  /* Initialize view 0? */
  if (fractview != view)
  {
    leftUpper  [0] = leftUpper  [view];
    rightLower [0] = rightLower [view];
    fractview = view;
  }

  /* Adjust ‘real_factor’ and ‘imag_factor’: */
  { float real_dist = fabsf (LD_FLOAT(crealf (rightLower [0])) - LD_FLOAT(crealf (leftUpper [0])));
    float imag_dist = fabsf (LD_FLOAT(cimagf (rightLower [0])) - LD_FLOAT(cimagf (leftUpper [0])));
    float ratio = real_dist / imag_dist;
    float corr_dist;

    #if defined(UPDATE_SET_DIMENSIONS)
    #if C99COMPLEX_AVAILABLE
    minInSet = CMPLXF (FLT_MAX, FLT_MAX);
    maxInSet = CMPLXF (FLT_MIN, FLT_MIN);
    #else
    f = FLT_MAX;
    minInSet.real = minInSet.imag = ST_FLOAT(f);
    f = FLT_MIN;
    maxInSet.real = maxInSet.imag = ST_FLOAT(f);
    #endif
    #endif

    /* y1 = y2 + (x2 - x1) * h / w = -1,2 + (1,0 - -2,0) * 600 / 800 = +1,05i */
    if (ratio < (float)screen_width / screen_height)
    {
      corr_dist = imag_dist * screen_width / screen_height;
      corr_dist = (real_dist - corr_dist) / 2;

      f = LD_FLOAT(crealf (leftUpper [0])) + corr_dist;
      set_crealf (leftUpper [0], ST_FLOAT(f));

      f = LD_FLOAT(crealf (rightLower [0])) - corr_dist;
      set_crealf (rightLower [0], ST_FLOAT(f));
    }
    else
    {
      corr_dist = real_dist * screen_height / screen_width;
      corr_dist = (imag_dist - corr_dist) / 2;

      f = LD_FLOAT(cimagf (leftUpper [0])) - corr_dist;
      set_cimagf (leftUpper [0], ST_FLOAT(f));

      f = LD_FLOAT(cimagf (rightLower [0])) + corr_dist;
      set_cimagf (rightLower [0], ST_FLOAT(f));
    }

    f = (LD_FLOAT(crealf (rightLower [0])) - LD_FLOAT(crealf (leftUpper [0]))) / (screen_width - 1);
    real_factor = ST_FLOAT(f);
    f = -(LD_FLOAT(cimagf (leftUpper [0])) - LD_FLOAT(cimagf (rightLower [0]))) / (screen_height - 1);
    imag_factor = ST_FLOAT(f);

    #if SUPPORTED_IMPLEMENTATION == FRACTAL_FIXED_ASM
    f = LD_FLOAT(leftUpper[0].real); leftUpper[0].real = FLOAT2FIXED(f);
    f = LD_FLOAT(leftUpper[0].imag); leftUpper[0].imag = FLOAT2FIXED(f);
    f = LD_FLOAT(rightLower[0].real); rightLower[0].real = FLOAT2FIXED(f);
    f = LD_FLOAT(rightLower[0].imag); rightLower[0].imag = FLOAT2FIXED(f);

    f = LD_FLOAT(real_factor); real_factor = FLOAT2FIXED(f);
    f = LD_FLOAT(imag_factor); imag_factor = FLOAT2FIXED(f);
    #endif
  }
}


extern void FractalGetMousePos (int x, int y, float *real, float *imag)
{
  if (real != NULL)
   *real = FIXED2FLOAT(crealf (leftUpper [0])) + x*FIXED2FLOAT(real_factor);

  if (imag != NULL)
   *imag = FIXED2FLOAT(cimagf (leftUpper [0])) + y*FIXED2FLOAT(imag_factor);
}


extern float FractalZoom (int x, int y, int direction)
{ float
    factor = (direction < 0)? 0.98f : (1.0f/0.98f);
  float
    f, mreal, mimag;
  SCREEN_MAPPING
    sl, su, sr, sb;

  /* Zoom into view: */
  FractalGetMousePos (x, y, &mreal, &mimag);

  sl = crealf (leftUpper [0]);  f = (1.0f - factor)*mreal + factor*FIXED2FLOAT(sl);
  set_crealf (leftUpper [0], ST_FLOAT(f));

  su = cimagf (leftUpper [0]);  f = (1.0f - factor)*mimag + factor*FIXED2FLOAT(su);
  set_cimagf (leftUpper [0], ST_FLOAT(f));

  sr = crealf (rightLower [0]); f = (1.0f - factor)*mreal + factor*FIXED2FLOAT(sr);
  set_crealf (rightLower [0], ST_FLOAT(f));

  sb = cimagf (rightLower [0]); f = (1.0f - factor)*mimag + factor*FIXED2FLOAT(sb);
  set_cimagf (rightLower [0], ST_FLOAT(f));

  /* Update screen mapping factors: */
  FractalSetView (0, FALSE);

  /* Outside of valid area? */
  f = (float)fabs (FIXED2FLOAT(cimagf (rightLower [0])) - FIXED2FLOAT(cimagf (leftUpper [0])));

  if (f < MIN_PIXEL_SPACING*screen_height)
  {
    #if C99COMPLEX_AVAILABLE
    leftUpper  [0] = CMPLXF (sl, su);
    rightLower [0] = CMPLXF (sr, sb);
    #else
    leftUpper  [0].real = sl;
    leftUpper  [0].imag = su;
    rightLower [0].real = sr;
    rightLower [0].imag = sb;
    #endif
  }

  factor = (float)(f / fabs(LD_FLOAT(cimagf (rightLower [fractview])) - LD_FLOAT(cimagf (leftUpper [fractview]))));

  if (factor > 1.0f)
  {
    FractalSetView (0, TRUE);
    factor = 1.0f;
  }

  return (factor);
}
#if SUPPORTED_IMPLEMENTATION == FRACTAL_FIXED_ASM
  /*LD_FLOAT()/ST_FLOAT: dereferencing type-punned pointer will break strict-aliasing rules [-Werror=strict-aliasing] */
  #if defined(__GNUC__)
    #pragma GCC diagnostic pop
  #endif
#endif


#if defined(UPDATE_SET_DIMENSIONS)
static INLINE void FractalUpdateInSet (complex_t const c)
{
  { COMPLEX_BASE_T const c_real = crealf (c);

    if (c_real < crealf (minInSet))
      set_crealf (minInSet, c_real);
    if (c_real > crealf (maxInSet))
      set_crealf (maxInSet, c_real);
  }

  { COMPLEX_BASE_T const c_imag = crealf (c);

    if (c_imag < cimagf (minInSet))
      set_cimagf (minInSet, c_imag);
    if (c_imag > cimagf (maxInSet))
      set_cimagf (maxInSet, c_imag);
  }
}
#else
#define FractalUpdateInSet(c)
#endif


#if SUPPORTED_IMPLEMENTATION == FRACTAL_FLOAT_ADT

/*
// c … current position in the fractal plane (or on the screen)
//
// Z_0   = c
// Z_n+1 = Z_n*Z_n + c
//
// Mandelbrot set = { c ∈ ℂ : lim |Z_n| ≠ ∞ }
*/
extern /*INLINE*/ unsigned long MandelbrotPixelColorADT (float x, float y, BOOL screenCoords)
{ complex_t
    Z, c;
  unsigned int
    color;
  int
    i;

  if (not screenCoords)
  {
    #if C99COMPLEX_AVAILABLE
    c = CMPLXF (x, y);
    #else
    c.real = x;
    c.imag = y;
    #endif
  }
  else
  {
    set_crealf (c, crealf (leftUpper [0]) + x * real_factor);
    set_cimagf (c, cimagf (leftUpper [0]) + y * imag_factor);
  }

  Z = c;

  /* |Z_i| > 2.0 ⇒ sequence Z_i is not bounded: */
  for (i=MAX_ITERATIONS; i > 0; --i)
  {
  #if C99COMPLEX_AVAILABLE
    if (cabsf (Z) > 2.0f)
      break;

    #if defined(_MSC_VER)
    Z = _FCmulcc (Z, Z);
    Z = _FCaddcc (Z, c);
    #else
    Z = Z * Z;
    Z = Z + c;
    #endif
  #else
    if (AbsComplex (&Z) > 2.0f)
      break;

    Z = MulComplex (&Z, &Z);
    Z = AddComplex (&Z, &c);
  #endif
  }

  /* i==0 ⇒ current point belongs to the fractal set (=drawn in black): */
  if (i==0)
    FractalUpdateInSet (c);

  if (incolor)
    color = i;
  else
    color = (i==0)? (vmode_colors-1) : 0;

  return (color);
}

#elif SUPPORTED_IMPLEMENTATION == FRACTAL_FLOAT_INLINE

extern /*INLINE*/ int MandelbrotCardoidPeriod2Bulb (float x, float y)
{
  /* Cardioid: */
  float x_  = (x - 0.25f);
  float y_2 = y*y;
  float q   = x_*x_ + y_2;

  if (4.0f * q*(q + x_) < y_2)
    return (TRUE);

  /* Period-2 bulb: */
  x_ += 1.25f;

  if (16.0f * (x_*x_ + y_2) < 1.0f)
    return (TRUE);

  return (FALSE);
}

/*
// Z_i^2 + c     =  (x_i + y_i*i)^2 + x_0 + y_0  <⇒
// Re(Z_i^2 + c) =  x_i^2 - y_i^2 + x_0
// Im(Z_i^2 + c) =  2*x_i*y_i + y_0
// Abs(Z_i+1)    =  x_i+1^2 + y_i+1^2
*/
extern /*INLINE*/ unsigned long MandelbrotPixelColorINL (float x, float y, BOOL screenCoords)
{ complex_t
    c;
  float
    xi, yi;
  float
    xi2, yi2;
  float
    radius;
  int
    i;

  /* Z_0 = x + y*i: */
  if (not screenCoords)
  {
    xi = x;
    yi = y;
  }
  else
  {
    xi = crealf (leftUpper [0]) + x * real_factor;
    yi = cimagf (leftUpper [0]) + y * imag_factor;
  }

  #if C99COMPLEX_AVAILABLE
  c = CMPLXF (xi, yi);
  #else
  c.real = xi;
  c.imag = yi;
  #endif

  /* |Z_i| > 4.0 ⇒ sequence Z_i is not bounded: */
  for (i=MAX_ITERATIONS; i > 0; --i)
  {
    xi2 = xi * xi;
    yi2 = yi * yi;

    radius = xi2 + yi2;

    if (radius > 4.0)
      break;

    #if defined(MANDEL_CARDIOID_PERIOD2BULB)
    /* Only ~40% of all points don't terminate during the first three iterations: */
    if (i==MAX_ITERATIONS-2 && MandelbrotCardoidPeriod2Bulb (crealf (c), cimagf (c)))
    {
      i = 0;
      break;
    }
    #endif

    yi = 2*xi*yi + cimagf (c);
    xi = xi2 - yi2 + crealf (c);

    #if defined(CALCULATE_OP_STATISTICS)
    add_count += 4;
    mul_count += 4;
    #endif
  }

  /* i==0 ⇒ current point belongs to the fractal set (=drawn in black): */
  if (i==0)
    FractalUpdateInSet (c);

  return (i);
}

#endif


static INLINE float GoldenRatio (void)
{ static float
    ratio = (float)((1.0 + 2.2360679774997896964091736687313/*sqrt(5)*/) / 2.0);

  return (ratio);
}


#if SUPPORTED_IMPLEMENTATION == FRACTAL_FLOAT_ADT

/*
// c … current position in the fractal plane (or on the screen)
//
// Z_0   = c
// Z_n+1 = Z_n*Z_n + K
//
// K=(φ−2)+(φ−1)i, where φ is the golden ratio
//
// Julia set = { c ∈ ℂ : lim |Z_n| ≠ ∞ }
*/
extern /*INLINE*/ unsigned long JuliaPixelColorSimpleADT (float x, float y, BOOL screenCoords)
{ complex_t
    Z, K;
  unsigned int
    color;
  int
    i;

  if (not screenCoords)
  {
    #if C99COMPLEX_AVAILABLE
    Z = CMPLXF (x, y);
    #else
    Z.real = x;
    Z.imag = y;
    #endif
  }
  else
  {
    set_crealf (Z, crealf (leftUpper [0]) + x * real_factor);
    set_cimagf (Z, cimagf (leftUpper [0]) + y * imag_factor);
  }

  /* K = (phi-2) + (phi-1)i */
  #if C99COMPLEX_AVAILABLE
  K = CMPLXF (GoldenRatio() - 2, GoldenRatio() - 1);
  #else
  K.real = GoldenRatio() - 2;
  K.imag = GoldenRatio() - 1;
  #endif

  /* |Z_i| > 2.0 ⇒ sequence Z_i is not bounded: */
  for (i=MAX_ITERATIONS; i > 0; --i)
  {
  #if C99COMPLEX_AVAILABLE
    if (cabsf (Z) > 2.0f)
      break;

    #if defined(_MSC_VER)
    Z = _FCmulcc (Z, Z);
    Z = _FCaddcc (Z, K);
    #else
    Z = Z * Z;
    Z = Z + K;
    #endif
  #else
    if (AbsComplex (&Z) > 2.0)
      break;

    Z = MulComplex (&Z, &Z);
    Z = AddComplex (&Z, &K);
  #endif
  }

  /* i==0 ⇒ current point belongs to the fractal set (=drawn in black): */
  if (i==0)
  {
    set_crealf (Z, crealf (leftUpper [0]) + x * real_factor);
    set_cimagf (Z, cimagf (leftUpper [0]) + y * imag_factor);
    FractalUpdateInSet (Z);
  }

  if (incolor)
    color = i;
  else
    color = (i==0)? vmode_colors-1 : 0;

  return (color);
}

#elif SUPPORTED_IMPLEMENTATION == FRACTAL_FLOAT_INLINE

/*
// Z_i^2 + c     =  (x_i + y_i*i)^2 + x_0 + y_0  <⇒
// Re(Z_i^2 + c) =  x_i^2 - y_i^2 + x_0
// Im(Z_i^2 + c) =  2*x_i*y_i + y_0
// Abs(Z_i+1)    =  x_i+1^2 + y_i+1^2
*/
extern /*INLINE*/ unsigned long JuliaPixelColorINL (float x, float y, BOOL screenCoords)
{ complex_t
    K;
  float
    xi, yi;
  float
    xi2, yi2;
  float
    radius;
  int
    i;

  /* K = (phi-2) + (phi-1)i */
  #if C99COMPLEX_AVAILABLE
  K = CMPLXF (GoldenRatio() - 2, GoldenRatio() - 1);
  #else
  K.real = GoldenRatio() - 2;
  K.imag = GoldenRatio() - 1;
  #endif

  /* Z_0 = x + y*i: */
  if (not screenCoords)
  {
    xi = x;
    yi = y;
  }
  else
  {
    xi = crealf (leftUpper [0]) + x * real_factor;
    yi = cimagf (leftUpper [0]) + y * imag_factor;
  }

  xi2 = xi * xi;
  yi2 = yi * yi;

  radius = xi2 + yi2;

  #if defined(CALCULATE_OP_STATISTICS)
  ++add_count;
  mul_count += 2;
  #endif

  /* |Z_i| > 4.0 ⇒ sequence Z_i is not bounded: */
  for (i=MAX_ITERATIONS; i > 0; --i)
  {
    if (radius > 4.0)
      break;

    yi = 2*xi*yi + cimagf (K);
    xi = xi2 - yi2 + crealf (K);

    xi2 = xi * xi;
    yi2 = yi * yi;

    radius = xi2 + yi2;

    #if defined(CALCULATE_OP_STATISTICS)
    add_count += 4;
    mul_count += 4;
    #endif
  }

  /* i==0 ⇒ current point belongs to the fractal set (=drawn in black): */
  if (i==0)
  {
    set_crealf (K, crealf (leftUpper [0]) + x * real_factor);
    set_cimagf (K, cimagf (leftUpper [0]) + y * imag_factor);
    FractalUpdateInSet (K);
  }

  return (i);
}

#endif
