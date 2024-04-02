#ifndef SAFE_MALLOC_H_
#define SAFE_MALLOC_H_

#if defined(_WIN32)
#include <crtdefs.h>              /* size_t */
#else
#include <stddef.h>
#endif

/* */
#define mem_alloc(elements, size) (mem_alloc) (elements, size, __FILE__, __LINE__)
#define mem_realloc(ptr, elements, size) (mem_realloc) (ptr, elements, size, __FILE__, __LINE__)
#define mem_free(memblock) (mem_free) ((void **)&memblock)

/* */
#ifdef __cplusplus
extern "C" {
#endif

void *
 (mem_alloc) (size_t elements, size_t size, char const *file, int line);
void *
 (mem_realloc) (void *ptr, size_t elements, size_t size, char const *file, int line);
void
 (mem_free) (void **ptr);

#ifdef __cplusplus
}
#endif

#endif
