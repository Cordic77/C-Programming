#include <stdio.h>

#include <stdlib.h>               /* abort() */
#include <string.h>               /* strcmp() */
#include <iso646.h>               /* not */

#include <assert.h>               /* assert() macro */


/* GNU Multiple Precision Arithmetic (GMP) / Multiple Precision Integers and Rationals (MPIR): */
#include <gmp.h>                  /* mpz_t; mpz_init(), mpz_set_ui(); mpz_mul(); … */

extern void power (mpz_t const base, mpz_t const exp, mpz_t result);
extern void tetra (mpz_t const base, mpz_t const height, mpz_t result);

/* Standard integer types implementation: */
extern long long powll (long long base, unsigned long long exp);
extern long long tetrall (long long base, unsigned int height);


/* Program entry point: */
int main (void)
{ mpz_t base, height, result;

  /* mpz_t base = 4; */
  mpz_init (base);
  mpz_set_ui (base, 4);

  /* mpz_t height = 3; */
  mpz_init (height);
  mpz_set_ui (height, 3);

  /* mpz_t result; */
  mpz_init (result);

  /* Given test case: 4 ↑↑ 3 == 13407807929942597099574024998205846\
                                12747936582059239337772356144372176\
                                40300735469768018742981669034276900\
                                31858186486050853753882811946569946\
                                433649006084096 */
  tetra (base, height, result);

  /* Convert result to a string: */
  { char *result_str;

    /* N.b.: (possible) minus sign | bignum digits | NUL terminator: */
    { size_t result_bytes = 2 + mpz_sizeinbase (result, 10);

      result_str = (char *)malloc (result_bytes);
      if (not result_str)
        abort ();
    }

    (void)mpz_get_str (result_str, 10, result);

    /* Print result & assertion: */
  /*fprintf (stdout, "tetra(4, 3) == %s\n", result_str);*/
    gmp_printf ("tetra(%u, %u) == %Zd\n", 4, 3, result);
    fflush (stdout);

    /* Assertion: */
    { int correct = (strcmp (result_str, "13407807929942597099574024998205846"
                                         "12747936582059239337772356144372176"
                                         "40300735469768018742981669034276900"
                                         "31858186486050853753882811946569946"
                                         "433649006084096") == 0);
      free (result_str);
      assert (correct); (void)correct;
    }
  }

  return (0);
}


/* GNU Multiple Precision Arithmetic (GMP) / Multiple Precision Integers and Rationals (MPIR): */
extern void power (mpz_t const base, mpz_t const exp, mpz_t result)
{
  /* Set-up: simulate call-by-value (as integers are normally passed): */
  mpz_t squared_base, shifted_exp;

  mpz_init (squared_base); mpz_set (squared_base, base);/* mpz_t squared_base = base; */
  mpz_init (shifted_exp);  mpz_set (shifted_exp, exp);  /* mpz_t shifted_exp  = exp; */

  /* Function body: */
  {
    mpz_set_ui (result, 1);                               /* mpz_t result = 1; */

    while (mpz_cmp_ui (shifted_exp, 0) > 0)               /* while (shifted_exp > 0) */
    {
      if (mpz_odd_p (shifted_exp))                        /*   if ((exp & 1) != 0) */
        mpz_mul (result, result, squared_base);           /*     result *= squared_base; */

      mpz_fdiv_q_2exp (shifted_exp, shifted_exp, 1);      /*   shifted_exp >>= 1; */
      mpz_mul (squared_base, squared_base, squared_base); /*   squared_base *= squared_base; */
    }
  }

  /* Tear-down: free call-by-value helper variables: */
  mpz_clear (shifted_exp);
  mpz_clear (squared_base);

/*return (result);*/  /* Result already in ‘result’ */
}


extern void tetra (mpz_t const base, mpz_t const height, mpz_t result)
{
  /* Set-up: simulate call-by-value (as integers are normally passed): */
  mpz_t remain_height;

  mpz_init (remain_height); mpz_set (remain_height, height);  /* mpz_t remain_height = height; */

  /* Function body: */
  { mpz_t exp;
    mpz_init (exp);

    /* mpz_t result = 1; */
    mpz_set_ui (result, 1);

    /* while (exp > 0) */
    while (mpz_cmp_ui (remain_height, 0) > 0)       /* while (height > 0) */
    {
      mpz_set (exp, result);                        /*   exp = result; */
      power (base, exp, result);                    /*   result = ipow (base, exp); */
      mpz_sub_ui (remain_height, remain_height, 1); /*   height -= 1; */
    }
  }

  /* Tear-down: free call-by-value helper variables: */
  mpz_clear (remain_height);

/*return (result);*/  /* Result already in ‘result’ */
}


/* Standard integer types implementation: */
long long powll (long long base, unsigned long long exp)
{ long long result = 1;

  while (exp > 0)
  {
    if ((exp & 1) == 1)
      result *= base;

    exp >>= 1;
    base *= base;
  }

  return (result);
}


long long tetrall (long long base, unsigned int height)
{ long long result = 1;
  long long exp;

  while (height-- > 0)
  {
    exp = result;
    result = powll (base, exp);
  }

  return (result);
}
