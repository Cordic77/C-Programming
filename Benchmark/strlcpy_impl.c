#define _DEFAULT_SOURCE 1  /* (_BSD_SOURCE 1 causes __USE_MISC to be defined… and thus strlcpy() to be declared.) */

#include <stdio.h>
#include <stdlib.h>                /* srand(), rand() */
#include <string.h>                /* strlcpy() */
#include <time.h>                  /* time(), clock_t, CLOCKS_PER_SEC */

#include <stddef.h>                /* size_t */
#include <sys/types.h>             /* ssize_t */
#include <float.h>                 /* DBL_MAX */

#include <sys/random.h>            /* getrandom() */


/* Preprocessor macros: */
#define ARRAY_SIZE  (1024ull * 1048576ull)
#define WARM_UP 10u
#define SUM_UP 1u
#define ITERATIONS 100u

/* Results! – The Big Array Size Survey for C */
#if __STDC_VERSION__ < 202312L  /*FIXME once the next standard after C23 is released */
  /* Results! - The Big Array Size Survey for C. “C26” (?) will have a countof() operator: */
  #if (!defined(__STDC_VERSION__) || __STDC_VERSION__ <= 202311L) && !defined(countof)
  #define countof(arr)  (sizeof(arr)/sizeof((arr)[0]))
  #endif
#endif


/* Globals: */
static char *src;
static char *dst;


/* Function definitions: */
static void nonzero_random (char buffer [], size_t const size);
static void bench_strlcpy (void);
static size_t my_strlcpy (char * restrict dst, char const * restrict src, size_t size);


/* Program entry point: */
int main (void)
{
  setvbuf (stdout, NULL, _IONBF, 0);  /* (disable output buffering.) */

  /* Seed rand(): */
  srand ((unsigned int)time (NULL));

  /* Allocate arrays: */
  src = (char *)malloc (ARRAY_SIZE);
  dst = (char *)malloc (ARRAY_SIZE);

  /* Initialize src[i]: */
  setvbuf(stdout, NULL, _IONBF, 0);

  fprintf (stdout, "Initializing src [%llu] ... ", ARRAY_SIZE);
  nonzero_random (src, ARRAY_SIZE);
  src [ARRAY_SIZE-1] = '\0';
  fprintf (stdout, "done\n");

  fprintf (stdout, "strlen (src) → %zu == %zu? %s\n",
                   strlen (src), (size_t)(ARRAY_SIZE-1),
                   (strlen (src) == ARRAY_SIZE-1)? "yes" : "no");

  /* Run benchmark and print results: */
  bench_strlcpy ();

  return (0);
}


/* Function definitions: */
static void nonzero_random (char buffer [], size_t const size)
{ char rand_data [4096];
  size_t filled = 0;
  ssize_t read, i;

  while (filled < size)
  {
    /* Get 4 KiB of random data: */
    read = getrandom (rand_data, sizeof(rand_data), 0);

    if (read < 0)
    {
      perror ("getrandom()");
      exit (255);
    }

    /* Copy non-zero bytes only: */
    for (i=0; i < read; i++)
      buffer [filled + i] = (rand_data [i] == 0) + rand_data [i];

    filled += read;
  }
}


static void bench_strlcpy (void)
{ double glibc_fastest = DBL_MAX, mlstr_fastest = DBL_MAX;
  double duration;
  clock_t begin, end;
  size_t i, j;

  fprintf (stdout, "\nBenchmark (%u warm up, %u iterations, summing up %u invocations ... ",
                   WARM_UP, ITERATIONS, SUM_UP);

  for (i=WARM_UP+ITERATIONS; i > 0; --i)
  {
    /* glibc 2.38 (or later) */
    {
      begin = clock ();
      for (j=SUM_UP; j > 0; --j)
        strlcpy (dst, src, ARRAY_SIZE);
      end = clock();
      duration = (double)(end - begin) / CLOCKS_PER_SEC;

      if (i <= ITERATIONS && duration < glibc_fastest)
        glibc_fastest = duration;
    }

    /* custom implementation */
    {
      begin = clock ();
      for (j=SUM_UP; j > 0; --j)
        my_strlcpy (dst, src, ARRAY_SIZE);
      end = clock();
      duration = (double)(end - begin) / CLOCKS_PER_SEC;

      if (i <= ITERATIONS && duration < mlstr_fastest)
        mlstr_fastest = duration;
    }
  }

  fprintf (stdout, "done\n\n");

  fprintf (stdout, "glibc_fastest = %.17f, mlstr_fastest = %.17f\n",
                   glibc_fastest, mlstr_fastest);
}


static size_t my_strlcpy (char * restrict dst, char const * restrict src, size_t size)
{ char const *
    start = src;
  size_t
    n = size;

  /* Copy as many bytes as will fit: */
  if (n != 0 && --n != 0)
  {
    do {
      if ((*dst++ = *src++) == '\0')
        break;
    } while (--n != 0);
  }

  /* Not enough room in dst, add NUL and traverse rest of src: */
  if (n == 0)
  {
    if (size != 0)
      *dst = '\0';
    while (*src++)
      ;
  }

  return (start - src - 1);  /* count does not include NUL */
}
