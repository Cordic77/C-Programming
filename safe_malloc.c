#include "safe_malloc.h"

#include <stdio.h>                /* fprintf() */
#include <stdlib.h>               /* exit() */

#include <string.h>               /* strerror() */
#include <errno.h>                /* extern int errno; */

#include <iso646.h>               /* “C95”: not, and, or, xor, … */

#include <inttypes.h>             /* “C99”: PRIu32, PRIuMAX, … (includes <stdint.h>) */


/* */
extern char const *PROGRAM_NAME;


/* */
void *(mem_alloc) (size_t elements, size_t size, char const *file, int line)
{
  void *ptr = malloc (elements * size);

  if (not ptr)
  {
    fprintf (stderr, "\n[%s] %s:%" PRIu32 " mem_alloc(%" PRIuMAX ",%" PRIuMAX ") failed:\n%s\n",
                        PROGRAM_NAME, file, (uint32_t)line,
                        (uintmax_t)elements, (uintmax_t)size,
                        strerror (errno));
    exit (255);
  }

  return (ptr);
}


void *(mem_realloc) (void *ptr, size_t elements, size_t size, char const *file, int line)
{
  ptr = realloc (ptr, elements * size);

  if (not ptr)
  {
    fprintf (stderr, "\n[%s] %s:%" PRIu32 " mem_realloc(%" PRIuMAX ",%" PRIuMAX ") failed:\n%s\n",
                        PROGRAM_NAME, file, (uint32_t)line,
                        (uintmax_t)elements, (uintmax_t)size,
                        strerror (errno));
    exit (255);
  }

  return (ptr);
}


void (mem_free) (void **ptr)
{
  if (ptr && *ptr)
  {
    free (*ptr);
    *ptr = NULL;
  }
}
