#include <math.h>                 /* sqrt() */

#include "stdinc.h"
#include "Complex_ADT.h"


/* Exports: */
complex_t const
  complexZero = COMPLEX_ZERO;


/* Function definitions: */
#if defined(FRACTAL_FLOAT_ADT) && C_STD < 1999
extern complex_t AddComplex (complex_t const *c1, complex_t const *c2)
{ complex_t
    result;

  result.real = c1->real + c2->real;
  result.imag = c1->imag + c2->imag;

  return (result);
}


extern complex_t MulComplex (complex_t const *c1, complex_t const *c2)
{ complex_t
    result;

  result.real = c1->real * c2->real - c1->imag * c2->imag;
  result.imag = c1->imag * c2->real + c1->real * c2->imag;

  return (result);
}


extern double AbsComplex (complex_t const *c)
{
  return (sqrt (c->real * c->real + c->imag * c->imag));
}
#endif
