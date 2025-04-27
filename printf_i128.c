#include <stdio.h>   // fprintf(), stdout
#include <stdlib.h>  // EXIT_SUCCESS
#include <time.h>    // clock_gettime()
#include <limits.h>  // ULONG_LONG_MAX
#include <unistd.h>  // sleep()

__uint128_t const UINT128_MAX = (__int128_t)-1;
__int128_t  const INT128_MAX  = UINT128_MAX >> 1;
__int128_t  const INT128_MIN  = -INT128_MAX - 1;

int fprintf_ui128_div (FILE *restrict stream, __uint128_t ui128);
int fprintf_ui128 (FILE *restrict stream, __uint128_t ui128);
int fprintf_i128 (FILE *restrict stream, __int128_t i128);

int main (void)
{
  fprintf_ui128 (stdout, UINT128_MAX); fputc ('\n', stdout);
  fprintf_i128 (stdout,  INT128_MIN);  fputc ('\n', stdout);
  fprintf_i128 (stdout,  INT128_MAX);  fputc ('\n', stdout);

  { FILE *dev_null = fopen ("/dev/null", "w");
    struct timespec start, end;
    unsigned long long diff;
    size_t i;

    { clock_gettime (CLOCK_MONOTONIC_RAW, &start);
      {
        fprintf (stdout, "Testing fprintf_ui128():\n");
        for (i = 0; i < 100000000; ++i)
          fprintf_ui128 (dev_null, UINT128_MAX);
      }
      clock_gettime(CLOCK_MONOTONIC_RAW, &end);

      diff = ((end.tv_sec - start.tv_sec)*(1000*1000*1000) + (end.tv_nsec - start.tv_nsec)) / 1000;
      printf ("%llu usec passed\n", diff);
    }

    { clock_gettime (CLOCK_MONOTONIC_RAW, &start);
      {
        fprintf (stdout, "Testing fprintf_ui128_div():\n");
        for (i = 0; i < 100000000; ++i)
          fprintf_ui128_div (dev_null, UINT128_MAX);
      }
      clock_gettime(CLOCK_MONOTONIC_RAW, &end);

      diff = ((end.tv_sec - start.tv_sec)*(1000*1000*1000) + (end.tv_nsec - start.tv_nsec)) / 1000;
      printf ("%llu usec passed\n", diff);
    }
  }

  return (EXIT_SUCCESS);
}

int fprintf_ui128_div (FILE *restrict stream, __uint128_t ui128)
{
  if (ui128 == 0)
    return (fprintf (stream, "0"));

  { char str [sizeof("340282366920938463463374607431768211455")]; /* 2¹²⁸-1 */
    size_t digit_index = sizeof(str);

    str [--digit_index] = '\0';

    while (ui128 > 0)
    {
      str [--digit_index] = '0' + (ui128 % 10);
      ui128 /= 10;
    }

    return (fprintf (stream, "%s", str + digit_index));
  }
}

int fprintf_ui128 (FILE *restrict stream, __uint128_t ui128)
{ static unsigned long long lower_order_div = 10000000000000000000ull;

  if (ui128 <= ULLONG_MAX)
    return (fprintf (stream, "%llu", (unsigned long long)ui128));

  { unsigned long long lower_order_digits, higher_order_digits;
    int straddling_digit;

    lower_order_digits  = ui128 % lower_order_div;
    ui128               = ui128 / lower_order_div;
    straddling_digit    = ui128 % 10;
    higher_order_digits = ui128 / 10;

    return (fprintf (stream, "%llu%d%llu",
                     higher_order_digits, straddling_digit, lower_order_digits));
  }
}

int fprintf_i128 (FILE *restrict stream, __int128_t i128)
{
  if (i128 == INT128_MIN)
    return (fprintf (stream, "-170141183460469231731687303715884105728"));

  if (i128 < 0)
  {
    fputc ('-', stream);
    i128 = -i128;
  }

  return (fprintf_ui128 (stream, (__uint128_t)i128));
}
