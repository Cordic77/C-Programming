#define __USE_XOPEN2K             /* pthread_barrier_t */

#include <stdlib.h>               /* NULL */
#include <errno.h>                /* int errno; */

#include <pthread.h>              /* pthread_create() */
#include <semaphore.h>            /* sem_t */

#include "stdinc.h"

#if SDL_VER > 1                   /* Simple DirectMedia Layer */
  WARNING_PUSH()
  MSCWARN_DISABLE(4127)           /* _MSC: warning C4127: conditional expression is constant */
  #include <SDL2/SDL.h>
  WARNING_POP()
#else
  #include <SDL/SDL.h>
#endif

#include "Graphics.h"             /* screen_width, screen_height */
#include "FloatPoint.h"           /* FPU_SetControlWord(), SSE_SetControlWord() */
#include "Complex_ADT.h"              /* AddComplex(), MulComplex(), AbsComplex() */
#include "Fractal.h"              /* FractalPixelColor() */  /* Also includes ‘SIMDTypes.h’! */
#include "DrawFractal.h"

/* Structures: */
struct DRAW_CREW;

struct THREAD_ARG
{
  SDL_Rect           rect;        /* Screen rectangle */
  pthread_t          thread;      /* Current thread */
  struct DRAW_CREW  *crew;        /* The threads crew */
  int                tid;         /* Thread index (1..N) */
};

struct DRAW_CREW
{
  int                size;        /* Size of the crew */
  struct THREAD_ARG *member;      /* Array of threads */
  SDL_Rect           tiles;       /* w: width of each screen tile; h: height of each screen tile;
                                     x: number of horizontal threads; y: number of vertical threads */
  #if !defined(MANDEL_RECREATE_THREADS_PER_ITER)
  pthread_mutex_t    mutex;       /* Mutex – mutual exclusion object */
/*pthread_cond_t     redraw; */   /* Condition variable/Semaphore – redraw crew->rect? */
  sem_t              redraw;
  pthread_barrier_t *barrier;     /* Thread barrier to wait for all threads to finish */
  #endif
};


/* Globals: */
struct DRAW_CREW
  crew; /*= {0};                  // Crew has external visibility */
BOOL
  drawFractal = TRUE;             /* draw calculated pixel color values? */
BOOL
  drawPartitions = FALSE;         /* draw result of Mariani-Silver subdivision? */

/* Thread-local storage: */
#if defined(MANDEL_MARIANI_SILVER_SUBDIVISION)
static pthread_once_t
  run_once = PTHREAD_ONCE_INIT;       /* InitializeTLS() must only be called once! */
static pthread_key_t
  tsd_key;                            /* Thread-specific data key */
static struct FRACTAL_COORDS
 *threadCoord = NULL;

#define _pt_real    tsd->pt_real
#define _pt_imag    tsd->pt_imag
#define _rect_clr   tsd->rect_clr
#define _pt_clr     tsd->pt_clr
#define _c_real     tsd->c_real
#define _c_curr     tsd->c_curr
#define _c_imag     tsd->c_imag
#endif


/* Function declarations: */
extern BOOL
  (FractalDraw) (int l, int u, int r, int b);
extern void *
  FractalDrawThread (void *arg);

#if defined(MANDEL_MARIANI_SILVER_SUBDIVISION)
static void
  InitializeTLS (void);
static void
  FractalDrawPartition (int l, int u, int r, int b);
#elif defined(MANDEL_SINGLE_SCREEN)
static BOOL
  FractalDrawScreen (int l, int u, int r, int b);
#elif defined(MANDEL_SINGLE_PIXEL) || defined(MANDEL_SINGLE_LINE)
static BOOL
  FractalDrawPixelLine (int l, int u, int r, int b);
#endif


/* SIMD support: */
#if SIMD_INSTRUCTIONS || defined(MANDEL_SINGLE_SCREEN)/*|| defined(MANDEL_MARIANI_SILVER_SUBDIVISION)*/
  #if PREFER_INTRINSICS
  EXTERN_C void
    MandelbrotPixelColor4SSE1Intrin (__m128 *in_real, __m128 *in_imag, __m128i *result);
  EXTERN_C void
    MandelbrotPixelColor8AVX1Intrin (__m256 *in_real, __m256 *in_imag, __m256i *result);
  EXTERN_C void
    MandelbrotPixelColor8AVX2Intrin (__m256 *in_real, __m256 *in_imag, __m256i *result);
  EXTERN_C void
    MandelbrotPixelColor16AVX512FIntrin (__m512 *in_real, __m512 *in_imag, __m512i *result);
  #endif


  #if SUPPORTED_IMPLEMENTATION >= FRACTAL_FLOAT_SSE1_ASM
  /* MandelbrotPixelColor4SSE1[Intrin] (x, y, result) */
  #define SIMD4_FractalPixelColor(c_real,c_imag,result) FractalPixelColor((__m128 *)(c_real), (__m128 *)(c_imag), (__m128i *)(result), FALSE)
  #else
  static INLINE void SIMD4_FractalPixelColor (SIMD_FLT *c_real, SIMD_FLT *c_imag, SIMD_INT *result)
  {
    FractalPixelColor (c_real->_f[0], c_imag->_f[0], result->_ui[0], FALSE);
    FractalPixelColor (c_real->_f[1], c_imag->_f[1], result->_ui[1], FALSE);
    FractalPixelColor (c_real->_f[2], c_imag->_f[2], result->_ui[2], FALSE);
    FractalPixelColor (c_real->_f[3], c_imag->_f[3], result->_ui[3], FALSE);
  }
  #endif


  #if SUPPORTED_IMPLEMENTATION >= FRACTAL_FLOAT_AVX2_ASM
  /* MandelbrotPixelColor8AVX2[Intrin] (x, y, result) */
  #define SIMD8_FractalPixelColor(c_real,c_imag,result) FractalPixelColor((__m256 *)(c_real), (__m256 *)(c_imag), (__m256i *)(result), FALSE)
  #elif SUPPORTED_IMPLEMENTATION >= FRACTAL_FLOAT_AVX1_ASM
  /* MandelbrotPixelColor8AVX1[Intrin] (x, y, result) */
  #define SIMD8_FractalPixelColor(c_real,c_imag,result) FractalPixelColor((__m256 *)(c_real), (__m256 *)(c_imag), (__m256i *)(result), FALSE)
  #else
    #if SUPPORTED_IMPLEMENTATION >= FRACTAL_FLOAT_SSE1_ASM
    static INLINE void SIMD8_FractalPixelColor (SIMD_FLT *c_real, SIMD_FLT *c_imag, SIMD_INT *result)
    {
      SIMD4_FractalPixelColor ((__m128 *)c_real    , (__m128 *)c_imag    , (__m128i *)result    );
      SIMD4_FractalPixelColor ((__m128 *)c_real + 1, (__m128 *)c_imag + 1, (__m128i *)result + 1);
    }
    #else
    static INLINE void SIMD8_FractalPixelColor (SIMD_FLT *c_real, SIMD_FLT *c_imag, SIMD_INT *result)
    {
      FractalPixelColor (c_real->_f[0], c_imag->_f[0], result->_ui[0], FALSE);
      FractalPixelColor (c_real->_f[1], c_imag->_f[1], result->_ui[1], FALSE);
      FractalPixelColor (c_real->_f[2], c_imag->_f[2], result->_ui[2], FALSE);
      FractalPixelColor (c_real->_f[3], c_imag->_f[3], result->_ui[3], FALSE);
      FractalPixelColor (c_real->_f[4], c_imag->_f[4], result->_ui[4], FALSE);
      FractalPixelColor (c_real->_f[5], c_imag->_f[5], result->_ui[5], FALSE);
      FractalPixelColor (c_real->_f[6], c_imag->_f[6], result->_ui[6], FALSE);
      FractalPixelColor (c_real->_f[7], c_imag->_f[7], result->_ui[7], FALSE);
    }
    #endif
  #endif


  #if SUPPORTED_IMPLEMENTATION == FRACTAL_FLOAT_AVX512_ASM
  /* MandelbrotPixelColor16AVX512F[Intrin] (x, y, result) */
  #define SIMD16_FractalPixelColor(c_real,c_imag,result) FractalPixelColor((__m512 *)(c_real), (__m512 *)(c_imag), (__m512i *)(result), FALSE)
  #else
    #if SUPPORTED_IMPLEMENTATION >= FRACTAL_FLOAT_AVX1_ASM  /* AVX1 <or> AVX2? */
    static INLINE void SIMD16_FractalPixelColor (SIMD_FLT *c_real, SIMD_FLT *c_imag, SIMD_INT *result)
    {
      SIMD8_FractalPixelColor ((__m256 *)c_real    , (__m256 *)c_imag    , (__m256i *)result    );
      SIMD8_FractalPixelColor ((__m256 *)c_real + 1, (__m256 *)c_imag + 1, (__m256i *)result + 1);
    }
    #else
      #if SUPPORTED_IMPLEMENTATION >= FRACTAL_FLOAT_SSE1_ASM
      static INLINE void SIMD16_FractalPixelColor (SIMD_FLT *c_real, SIMD_FLT *c_imag, SIMD_INT *result)
      {
        SIMD4_FractalPixelColor ((__m128 *)c_real    , (__m128 *)c_imag    , (__m128i *)result    );
        SIMD4_FractalPixelColor ((__m128 *)c_real + 1, (__m128 *)c_imag + 1, (__m128i *)result + 1);
        SIMD4_FractalPixelColor ((__m128 *)c_real + 2, (__m128 *)c_imag + 2, (__m128i *)result + 2);
        SIMD4_FractalPixelColor ((__m128 *)c_real + 3, (__m128 *)c_imag + 3, (__m128i *)result + 3);
      }
      #else
      static INLINE void SIMD16_FractalPixelColor (SIMD_FLT *c_real, SIMD_FLT *c_imag, SIMD_INT *result)
      {
        FractalPixelColor (c_real->_f[ 0], c_imag->_f[ 0], result->_ui[ 0], FALSE);
        FractalPixelColor (c_real->_f[ 1], c_imag->_f[ 1], result->_ui[ 1], FALSE);
        FractalPixelColor (c_real->_f[ 2], c_imag->_f[ 2], result->_ui[ 2], FALSE);
        FractalPixelColor (c_real->_f[ 3], c_imag->_f[ 3], result->_ui[ 3], FALSE);
        FractalPixelColor (c_real->_f[ 4], c_imag->_f[ 4], result->_ui[ 4], FALSE);
        FractalPixelColor (c_real->_f[ 5], c_imag->_f[ 5], result->_ui[ 5], FALSE);
        FractalPixelColor (c_real->_f[ 6], c_imag->_f[ 6], result->_ui[ 6], FALSE);
        FractalPixelColor (c_real->_f[ 7], c_imag->_f[ 7], result->_ui[ 7], FALSE);
        FractalPixelColor (c_real->_f[ 8], c_imag->_f[ 8], result->_ui[ 8], FALSE);
        FractalPixelColor (c_real->_f[ 9], c_imag->_f[ 9], result->_ui[ 9], FALSE);
        FractalPixelColor (c_real->_f[10], c_imag->_f[10], result->_ui[10], FALSE);
        FractalPixelColor (c_real->_f[11], c_imag->_f[11], result->_ui[11], FALSE);
        FractalPixelColor (c_real->_f[12], c_imag->_f[12], result->_ui[12], FALSE);
        FractalPixelColor (c_real->_f[13], c_imag->_f[13], result->_ui[13], FALSE);
        FractalPixelColor (c_real->_f[14], c_imag->_f[14], result->_ui[14], FALSE);
        FractalPixelColor (c_real->_f[15], c_imag->_f[15], result->_ui[15], FALSE);
      }
      #endif
    #endif
  #endif


  #if SUPPORTED_IMPLEMENTATION == FRACTAL_FLOAT_AVX1024_ASM
  /* MandelbrotPixelColor32AVX1024[Intrin] (x, y, result) */
  #define SIMD32_FractalPixelColor(c_real,c_imag,result) FractalPixelColor((__m1024 *)(c_real), (__m1024 *)(c_imag), (__m1024i *)(result), FALSE)
  #else
    #if SUPPORTED_IMPLEMENTATION == FRACTAL_FLOAT_AVX512_ASM
    static INLINE void SIMD32_FractalPixelColor (SIMD_FLT *c_real, SIMD_FLT *c_imag, SIMD_INT *result)
    {
      SIMD16_FractalPixelColor ((__m512 *)c_real    , (__m512 *)c_imag    , (__m512i *)result    );
      SIMD16_FractalPixelColor ((__m512 *)c_real + 1, (__m512 *)c_imag + 1, (__m512i *)result + 1);
    }
    #elif SUPPORTED_IMPLEMENTATION >= FRACTAL_FLOAT_AVX1_ASM
    static INLINE void SIMD32_FractalPixelColor (SIMD_FLT *c_real, SIMD_FLT *c_imag, SIMD_INT *result)
    {
      SIMD8_FractalPixelColor ((__m256 *)c_real    , (__m256 *)c_imag    , (__m256i *)result    );
      SIMD8_FractalPixelColor ((__m256 *)c_real + 1, (__m256 *)c_imag + 1, (__m256i *)result + 1);
      SIMD8_FractalPixelColor ((__m256 *)c_real + 2, (__m256 *)c_imag + 2, (__m256i *)result + 2);
      SIMD8_FractalPixelColor ((__m256 *)c_real + 3, (__m256 *)c_imag + 3, (__m256i *)result + 3);
    }
    #else
      #if SUPPORTED_IMPLEMENTATION >= FRACTAL_FLOAT_SSE1_ASM
      static INLINE void SIMD32_FractalPixelColor (SIMD_FLT *c_real, SIMD_FLT *c_imag, SIMD_INT *result)
      {
        SIMD4_FractalPixelColor ((__m128 *)c_real    , (__m128 *)c_imag    , (__m128i *)result    );
        SIMD4_FractalPixelColor ((__m128 *)c_real + 1, (__m128 *)c_imag + 1, (__m128i *)result + 1);
        SIMD4_FractalPixelColor ((__m128 *)c_real + 2, (__m128 *)c_imag + 2, (__m128i *)result + 2);
        SIMD4_FractalPixelColor ((__m128 *)c_real + 3, (__m128 *)c_imag + 3, (__m128i *)result + 3);
        SIMD4_FractalPixelColor ((__m128 *)c_real + 4, (__m128 *)c_imag + 4, (__m128i *)result + 4);
        SIMD4_FractalPixelColor ((__m128 *)c_real + 5, (__m128 *)c_imag + 5, (__m128i *)result + 5);
        SIMD4_FractalPixelColor ((__m128 *)c_real + 6, (__m128 *)c_imag + 6, (__m128i *)result + 6);
        SIMD4_FractalPixelColor ((__m128 *)c_real + 7, (__m128 *)c_imag + 7, (__m128i *)result + 7);
      }
      #else
      static INLINE void SIMD32_FractalPixelColor (SIMD_FLT *c_real, SIMD_FLT *c_imag, SIMD_INT *result)
      {
        FractalPixelColor (c_real->_f[ 0], c_imag->_f[ 0], result->_ui[ 0], FALSE);
        FractalPixelColor (c_real->_f[ 1], c_imag->_f[ 1], result->_ui[ 1], FALSE);
        FractalPixelColor (c_real->_f[ 2], c_imag->_f[ 2], result->_ui[ 2], FALSE);
        FractalPixelColor (c_real->_f[ 3], c_imag->_f[ 3], result->_ui[ 3], FALSE);
        FractalPixelColor (c_real->_f[ 4], c_imag->_f[ 4], result->_ui[ 4], FALSE);
        FractalPixelColor (c_real->_f[ 5], c_imag->_f[ 5], result->_ui[ 5], FALSE);
        FractalPixelColor (c_real->_f[ 6], c_imag->_f[ 6], result->_ui[ 6], FALSE);
        FractalPixelColor (c_real->_f[ 7], c_imag->_f[ 7], result->_ui[ 7], FALSE);
        FractalPixelColor (c_real->_f[ 8], c_imag->_f[ 8], result->_ui[ 8], FALSE);
        FractalPixelColor (c_real->_f[ 9], c_imag->_f[ 9], result->_ui[ 9], FALSE);
        FractalPixelColor (c_real->_f[10], c_imag->_f[10], result->_ui[10], FALSE);
        FractalPixelColor (c_real->_f[11], c_imag->_f[11], result->_ui[11], FALSE);
        FractalPixelColor (c_real->_f[12], c_imag->_f[12], result->_ui[12], FALSE);
        FractalPixelColor (c_real->_f[13], c_imag->_f[13], result->_ui[13], FALSE);
        FractalPixelColor (c_real->_f[14], c_imag->_f[14], result->_ui[14], FALSE);
        FractalPixelColor (c_real->_f[15], c_imag->_f[15], result->_ui[15], FALSE);

        FractalPixelColor (c_real->_f[16], c_imag->_f[16], result->_ui[16], FALSE);
        FractalPixelColor (c_real->_f[17], c_imag->_f[17], result->_ui[17], FALSE);
        FractalPixelColor (c_real->_f[18], c_imag->_f[18], result->_ui[18], FALSE);
        FractalPixelColor (c_real->_f[19], c_imag->_f[19], result->_ui[19], FALSE);
        FractalPixelColor (c_real->_f[20], c_imag->_f[20], result->_ui[20], FALSE);
        FractalPixelColor (c_real->_f[21], c_imag->_f[21], result->_ui[21], FALSE);
        FractalPixelColor (c_real->_f[22], c_imag->_f[22], result->_ui[22], FALSE);
        FractalPixelColor (c_real->_f[23], c_imag->_f[23], result->_ui[23], FALSE);
        FractalPixelColor (c_real->_f[24], c_imag->_f[24], result->_ui[24], FALSE);
        FractalPixelColor (c_real->_f[25], c_imag->_f[25], result->_ui[25], FALSE);
        FractalPixelColor (c_real->_f[26], c_imag->_f[26], result->_ui[26], FALSE);
        FractalPixelColor (c_real->_f[27], c_imag->_f[27], result->_ui[27], FALSE);
        FractalPixelColor (c_real->_f[28], c_imag->_f[28], result->_ui[28], FALSE);
        FractalPixelColor (c_real->_f[29], c_imag->_f[29], result->_ui[29], FALSE);
        FractalPixelColor (c_real->_f[30], c_imag->_f[30], result->_ui[30], FALSE);
        FractalPixelColor (c_real->_f[31], c_imag->_f[31], result->_ui[31], FALSE);
      }
      #endif
    #endif
  #endif


  #if TARGETED_IMPLEMENTATION >= FRACTAL_FLOAT_AVX1024_ASM
  #define SIMD_FractalPixelColor(c_real,c_imag,result) SIMD32_FractalPixelColor(c_real, c_imag, result)
  #elif TARGETED_IMPLEMENTATION >= FRACTAL_FLOAT_AVX512_ASM
  #define SIMD_FractalPixelColor(c_real,c_imag,result) SIMD16_FractalPixelColor(c_real, c_imag, result)
  #elif TARGETED_IMPLEMENTATION >= FRACTAL_FLOAT_AVX1_ASM  /* AVX1 <or> AVX2? */
  #define SIMD_FractalPixelColor(c_real,c_imag,result) SIMD8_FractalPixelColor(c_real, c_imag, result)
  #else /*elif TARGETED_IMPLEMENTATION >= FRACTAL_FLOAT_SSE1_ASM */
  #define SIMD_FractalPixelColor(c_real,c_imag,result) SIMD4_FractalPixelColor(c_real, c_imag, result)
  #endif
#endif


/* Function definitions: */
static INLINE unsigned int RoundPower2 (unsigned int v)
{
  /* Round up to the next highest power of 2 – see http://graphics.stanford.edu/~seander/bithacks.html: */
  v--;
  v |= v >> 1;
  v |= v >> 2;
  v |= v >> 4;
  v |= v >> 8;
  v |= v >> 16;
  v++;

  return (v);
}


extern BOOL (FractalDraw) (int l, int u, int r, int b)
{ int
    ret;

  /* Only draw Mariani-Silver subdivision? ⇒ Clear screen: */
  if (not drawFractal)
    DrawFilledRectangleLS (l, u, r, b, 0);

  /* Create TLS key ‘tsd_key’: */
  #if defined(MANDEL_MARIANI_SILVER_SUBDIVISION)
  if ((ret=pthread_once (&run_once, InitializeTLS)) != 0)
  {
    fprintf (stderr, "pthread_once failed: %s\n", strerror (ret));
    exit (251);
  }
  #endif

  /* Initialize FractalDrawThread crew: */
  if (crew.member == NULL)
  {
    if ((crew.member=(struct THREAD_ARG *)calloc (maxThreads, sizeof(*crew.member))) == NULL)
    {
      fprintf (stderr, "calloc(crew.member) failed: %s\n", strerror (errno));
      exit (250);
    }

    #if !defined(MANDEL_RECREATE_THREADS_PER_ITER)
    if ((ret=pthread_mutex_init (&crew.mutex, NULL)) != 0)
    {
      fprintf (stderr, "pthread_mutex_init(crew.mutex) failed: %s\n", strerror (ret));
      exit (250);
    }

    if ((ret=sem_init (&crew.redraw, 0, 0)) != 0)
    {
      fprintf (stderr, "sem_init(crew.redraw) failed: %s\n", strerror (ret));
      exit (250);
    }
    #endif
  }

  /* Assign screen tiles to threads? */
  if (nrThreads > 1)
  { int
      i;

    if (crew.size != nrThreads)
    {
      /* Reinitialize thread barrier? */
      #if !defined(MANDEL_RECREATE_THREADS_PER_ITER)
      if (crew.barrier != NULL)
      {
        pthread_barrier_destroy (crew.barrier);
        memset (crew.barrier, 0, sizeof(*crew.barrier));
      }
      else
      {
        if ((crew.barrier=(pthread_barrier_t *)calloc (1, sizeof(*crew.barrier))) == NULL)
        {
          fprintf (stderr, "calloc(crew.barrier) failed: %s\n", strerror (errno));
          exit (250);
        }
      }

      if ((ret=pthread_barrier_init (crew.barrier, NULL, 1 + nrThreads))!=0)
      {
        fprintf (stderr, "pthread_barrier_init(crew.barrier) failed: %s\n", strerror (ret));
        exit (250);
      }
      #endif

      crew.size = nrThreads;

      /* Calculate screen tile size (w.r.t. the number of selected threads): */
      crew.tiles.x = 1; crew.tiles.w = (Uint16)screen_width;
      crew.tiles.y = 1; crew.tiles.h = (Uint16)screen_height;

/*    for (i=(int)RoundPower2(nrThreads); crew.tiles.x*crew.tiles.y < i; ) */
      while (crew.tiles.x*crew.tiles.y < nrThreads)
      {
        if (crew.tiles.w > crew.tiles.h)
        {
          crew.tiles.x++;
          crew.tiles.w = (Uint16)(screen_width / crew.tiles.x);
        }
        else
        {
          crew.tiles.y++;
          crew.tiles.h = (Uint16)(screen_height / crew.tiles.y);
        }
      }
    }

    /* Create threads: */
    for (i=0; i < nrThreads; i++)
    {
      crew.member[i].rect.x = (Sint16)((i % crew.tiles.x) * crew.tiles.w);
      if (i==nrThreads-1 || ((i + 1) % crew.tiles.x) == 0)
        crew.member[i].rect.w = (Uint16)(screen_width - crew.member[i].rect.x);
      else
        crew.member[i].rect.w = crew.tiles.w;

      crew.member[i].rect.y = (Sint16)((i / crew.tiles.x) * crew.tiles.h);
      if (i==nrThreads-1 || (i / crew.tiles.x) == crew.tiles.y-1)
        crew.member[i].rect.h = (Uint16)(screen_height - crew.member[i].rect.y);
      else
        crew.member[i].rect.h = crew.tiles.h;

      if (crew.member[i].tid==0)
      {
        crew.member[i].tid = 1 + i;
        crew.member[i].crew = &crew;

        /* Pass the threads screen tile in its thread argument; if the Mariani-Silver
           optimization is enabled, then FractalDrawThread() will store its coordinates
           in ‘thread-local storage’, thereby allowing all functions in its call tree
           to get access to this vital piece of information: */
        if ((ret=pthread_create (&crew.member[i].thread, NULL, FractalDrawThread, crew.member + i)) != 0)
        {
          fprintf (stderr, "pthread_create failed: %s\n", strerror (ret));
          exit (249);
        }
      }
    }

    /* Start calculation: */
    for (i=0; i < nrThreads; i++)
    {
    #if defined(MANDEL_RECREATE_THREADS_PER_ITER)
      if ((ret=pthread_join (crew.member[i].thread, NULL)) != 0)
      {
        fprintf (stderr, "pthread_join failed: %s\n", strerror (ret));
        exit (248);
      }
    }

    memset (crew.member, 0, maxThreads*sizeof(*crew.member));
    #else
      if (sem_post(&crew.redraw) < 0)
      {
        fprintf (stderr, "sem_post(crew.redraw) failed: %s\n", strerror (errno));
        exit (247);
      }
    }

    if ((ret=pthread_barrier_wait (crew.barrier))!=0 && ret!=PTHREAD_BARRIER_SERIAL_THREAD)
    {
      fprintf (stderr, "pthread_barrier_wait(crew.barrier) failed: %s\n", strerror (ret));
      exit (247);
    }
    #endif

    ret = TRUE;
  }
  else
  {
    /* If only one thread is active, then all DrawFunctionXxx() functions get called
       directly. To this end its necessary its threadCoord[0] values immediately: */
    #if defined(MANDEL_MARIANI_SILVER_SUBDIVISION)
    if ((ret=pthread_setspecific (tsd_key, threadCoord + 0)) != 0)
    {
      fprintf (stderr, "pthread_setspecific failed: %s\n", strerror (ret));
      exit (251);
    }
    #endif

    /* Update statistics? */
    #if defined(CALCULATE_OP_STATISTICS)
    add_count = 0;
    mul_count = 0;
    #endif

    /* Call FractalDrawXxx() functions directly: */
    #if defined(MANDEL_SINGLE_PIXEL) || defined(MANDEL_SINGLE_LINE)
    ret = FractalDrawPixelLine (l, u, r, b);
    #elif defined(MANDEL_MARIANI_SILVER_SUBDIVISION)
    FractalDrawPartition (l, u, r, b);
    ret = TRUE;
    #else
    ret = FractalDrawScreen (l, u, r, b);
    #endif
  }

  /* Free thread resources: */
  if (quit && crew.member!=NULL)
  {
    #if !defined(MANDEL_RECREATE_THREADS_PER_ITER)
    pthread_barrier_destroy (crew.barrier);
    free (crew.barrier); crew.barrier = NULL;

    sem_destroy (&crew.redraw);
    pthread_mutex_destroy (&crew.mutex);
    #endif

    free (crew.member);
    crew.member = NULL;
  }

  return (ret);
}


extern void *FractalDrawThread (void *arg)
{ struct THREAD_ARG
   *work = (struct THREAD_ARG *)arg;
  int
    l, u, r, b;
  int
    frameCount = 0;
  int
    ret;

  /* Initialize the pthreads („POSIX Threads“) library? */
  #if defined(_WIN32) && !defined(_DLL)
  pthread_win32_thread_attach_np ();
  #endif

  /* Floating Point Control Word: set single precision, round towards zero: */
  /* ATTN: needs to be set for every thread (even with pthreads) */
  FPU_set_control_word (FPU_PC_SINGLE_PREC | FPU_RC_ROUND_ZERO,
                        FPU_PC_MASK        | FPU_RC_MASK);
  SSE_set_control_word (MXCSR_RC_ROUND_ZERO, MXCSR_RC_MASK);

  /* Set thread detach state: */
  #if !defined(MANDEL_RECREATE_THREADS_PER_ITER)
  { pthread_t
      self = pthread_self();

    if ((ret=pthread_detach (self)) != 0)
    {
      fprintf (stderr, "pthread_detach(self) failed: %s\n", strerror (ret));
      exit (247);
    }
  }

  do {
    if (sem_wait(&work->crew->redraw) < 0)
    {
      fprintf (stderr, "sem_wait(crew->redraw) failed: %s\n", strerror (errno));
      exit (247);
    }
  #endif

    /* Update statistics? */
    #if defined(CALCULATE_OP_STATISTICS)
    add_count = 0;
    mul_count = 0;
    #endif

    /* Get current threads' arguments: */
    #if defined(MANDEL_MARIANI_SILVER_SUBDIVISION)
    if (frameCount == 0)
    {
      if ((ret=pthread_setspecific (tsd_key, threadCoord + (work->tid-1))) != 0)
      {
        fprintf (stderr, "pthread_setspecific failed: %s\n", strerror (ret));
        exit (254);
      }
    }

    l = work->rect.x;
    u = work->rect.y;
    r = work->rect.x + work->rect.w - 1;
    b = work->rect.y + work->rect.h - 1;

    FractalDrawPartition (l, u, r, b);
    #elif defined(MANDEL_SINGLE_SCREEN)
    l = work->rect.x;
    u = work->rect.y;
    r = work->rect.x + work->rect.w - 1;
    b = work->rect.y + work->rect.h - 1;

    while (not FractalDrawScreen (l, u, r, b));
    #else
    (void)l;
    (void)u;
    (void)r;
    (void)b;
    #endif

    /* Wait until all threads have processed their screen tile: */
    frameCount++;

  #if !defined(MANDEL_RECREATE_THREADS_PER_ITER)
    if ((ret=pthread_mutex_lock (&work->crew->mutex)) != 0)
    {
      fprintf (stderr, "pthread_mutex_lock(crew->mutex) failed: %s\n", strerror (ret));
      exit (247);
    }

    if (nrThreads < work->tid)
      work->tid = 0;

    if ((ret=pthread_mutex_unlock (&work->crew->mutex)) != 0)
    {
      fprintf (stderr, "pthread_mutex_unlock(crew->mutex) failed: %s\n", strerror (ret));
      exit (247);
    }

    if ((ret=pthread_barrier_wait (work->crew->barrier))!=0 && ret!=PTHREAD_BARRIER_SERIAL_THREAD)
    {
      fprintf (stderr, "pthread_barrier_wait(crew->barrier) failed: %s\n", strerror (ret));
      exit (247);
    }
  } while (work->tid > 0);
  #endif

  /* Uninitialize pthreads: */
  #if defined(_WIN32) && !defined(_DLL)
  pthread_win32_thread_detach_np ();
  #endif

  return ((void *)(intptr_t)ret);
}


/***********************   Mariani/Silver subdivision  ***********************/
#if defined(MANDEL_MARIANI_SILVER_SUBDIVISION)
static void InitializeTLS (void)
{ int
    ret;

  if ((ret=pthread_key_create (&tsd_key, NULL)) != 0)
  {
    fprintf (stderr, "pthread_key_create failed: %s\n", strerror (ret));
    exit (253);
  }

  #if defined(_WIN32)
  if ((threadCoord=(struct FRACTAL_COORDS *)_aligned_malloc (maxThreads*sizeof(*threadCoord), sizeof(__m512))) == NULL)
  #else
  if (posix_memalign ((void **)&threadCoord, sizeof(__m512), maxThreads*sizeof(*threadCoord)) != 0)
  #endif
  {
    fprintf (stderr, "malloc(tsd) failed: %s\n", strerror (errno));
    exit (252);
  }
}

#define TEST_PARTITION (48 + (tsd - threadCoord) * 24)  /* 192==‘red’ (max. 8 threads) */

/* Recursion level: */
static int
  level = 0;

static void FractalDrawPartition (int l, int u, int r, int b)
{ /* TLS: */
  struct FRACTAL_COORDS
   *tsd;

  /* Local: */
  int
    w, h;
  int
    i;

  if ((tsd=(struct FRACTAL_COORDS *)pthread_getspecific (tsd_key)) == NULL)
  {
    fprintf (stderr, "pthread_getspecific failed: %s\n", strerror (errno));
    exit (255);
  }

  _c_real = crealf (leftUpper [0]);
  _c_imag = cimagf (leftUpper [0]);

  /* Subdivide further? */
  w = r - l + 1;
  h = b - u + 1;

  if (/*(w < 2*SIMD_MINSIZE() && h < 2*SIMD_MINSIZE()) ||  // 2*minsz-1 */
      (w < 10 && h < 10)                                   /* Tested minimum tile sizes from 64x64 to 4x4! */
     )
  {
    int x = l;
    int y = u;
    int j = w * h;

    w = x;
    h = y;

    _c_real += l * real_factor; _c_curr = _c_real;
    _c_imag += u * imag_factor;

    for (i=0; j > 0; j--)
    {
      _pt_real._f[i] = _c_curr;
      _pt_imag._f[i] = _c_imag;

      /* Enough (SIMD) coordinates? */
      if (++i==SIMD_PARALLEL() || j==1)
      {
      /*int k = i;*/

        SIMD_FractalPixelColor (&_pt_real, &_pt_imag, &_pt_clr);

        if (drawFractal)
        {
        /*for (i=0; i < k; i++)*/
          for (i=0; i<SIMD_PARALLEL() && (h<y || (w<=x && h<=y)); i++)
          {
            PutPixelLS (w, h, _pt_clr._ui[i]);

            if (++w > r)
            {
              w = l;
              h++;
            }
          }
        }
        else
        {
          w = x;
          h = y;
        }

        i = 0;
      }

      /* Reinitialize column index? */
      if (++x <= r)
        _c_curr += real_factor;
      else
      {
        x = l;
        y++;

        _c_curr = _c_real;
        _c_imag += imag_factor;
      }
    }

    return;
  }

  /* Level 0 always gets subdivided! */
  if (level == 0)
    goto Partition;

  /* Do all border pixels of the current partition exhibit the same color value? */
  {
    int stepsx = (w + (SIMD_PARALLEL() / 4) - 2) / (SIMD_PARALLEL() / 4);
    int stepsy = (h + (SIMD_PARALLEL() / 4) - 2) / (SIMD_PARALLEL() / 4);

    #if TARGETED_IMPLEMENTATION == FRACTAL_FLOAT_AVX1024_ASM
    int w2 = (w >>= 1) >> 1;
    int h2 = (h >>= 1) >> 1;
    int w4 = w2 >> 1;
    int h4 = h2 >> 1;

    #elif TARGETED_IMPLEMENTATION == FRACTAL_FLOAT_AVX512_ASM
    int w2 = (w >>= 1) >> 1;
    int h2 = (h >>= 1) >> 1;

    #elif TARGETED_IMPLEMENTATION >= FRACTAL_FLOAT_AVX1_ASM  /* AVX1 <or> AVX2? */
    w >>= 1;
    h >>= 1;
    #endif

/*   0-> 1-> 2-> 3-> 4-> 5-> 6-> 7-> 16 */
/*   ^                                v */
/*  31                               17 */
/*   ^                                v */
/*  30       0-> 1-> 2-> 3-> 8       18 */
/*   ^        ^ 0 -> 1 -> 4  v        v */
/*  29       15 ^         v  9       19 */
/*   ^        ^ ^  0-> 2  v  v        v */
/*  28       14 7  ^   v  5 10       20 */
/*   ^        ^ ^  3 <-1  v  v        v */
/*  27       13 ^         v 11       21 */
/*   ^        ^ 6 <- 3 <- 2  v        v */
/*  26       12<-7 <-6 <-5 <-4       22 */
/*   ^                                v */
/*  25                               23 */
/*   ^                                v */
/*  24<-15<-14<-13<-12 <-11<-10 <-9 <-8 */

    /* real_factor is positive, imag_factor is negative: */

    /* 0 / 0 /  0 /  0 */
    _pt_real._f[0                    ] = _c_real + l * real_factor;
    _pt_imag._f[0                    ] = _c_imag + u * imag_factor;

    /* 2 / 4 /  8 / 16 */
    _pt_real._f[SIMD_PARALLEL()/2    ] = _c_real + r * real_factor;
    _pt_imag._f[SIMD_PARALLEL()/2    ] = _pt_imag._f[0                    ];
    /* 1 / 2 /  4 /  8 */
    _pt_real._f[SIMD_PARALLEL()/4    ] = _pt_real._f[SIMD_PARALLEL()/2    ];
    _pt_imag._f[SIMD_PARALLEL()/4    ] = _c_imag + b * imag_factor;
    /* 3 / 6 / 12 / 24 */
    _pt_real._f[3*SIMD_PARALLEL()/4  ] = _pt_real._f[0                    ];
    _pt_imag._f[3*SIMD_PARALLEL()/4  ] = _pt_imag._f[SIMD_PARALLEL()/4    ];

    #if TARGETED_IMPLEMENTATION >= FRACTAL_FLOAT_AVX1_ASM  /* AVX1 <or> AVX2? */
    /*   / 1 /  2 /  4 */
    _pt_real._f[SIMD_PARALLEL()/8    ] = _pt_real._f[0                    ] + w * real_factor;
    _pt_imag._f[SIMD_PARALLEL()/8    ] = _pt_imag._f[0                    ];
    /*   / 5 / 10 / 20 */
    _pt_real._f[5*SIMD_PARALLEL()/8  ] = _pt_real._f[SIMD_PARALLEL()/2    ];
    _pt_imag._f[5*SIMD_PARALLEL()/8  ] = _pt_imag._f[SIMD_PARALLEL()/2    ] + h * imag_factor;
    /*   / 3 /  6 / 12 */
    _pt_real._f[3*SIMD_PARALLEL()/8  ] = _pt_real._f[SIMD_PARALLEL()/4    ] - w * real_factor;
    _pt_imag._f[3*SIMD_PARALLEL()/8  ] = _pt_imag._f[SIMD_PARALLEL()/4    ];
    /*   / 7 / 14 / 28 */
    _pt_real._f[7*SIMD_PARALLEL()/8  ] = _pt_real._f[3*SIMD_PARALLEL()/4  ];
    _pt_imag._f[7*SIMD_PARALLEL()/8  ] = _pt_imag._f[3*SIMD_PARALLEL()/4  ] - h * imag_factor;
    #endif

    #if TARGETED_IMPLEMENTATION >= FRACTAL_FLOAT_AVX512_ASM  /* AVX512? */
    /*   /   /  1 /  2 */
    _pt_real._f[SIMD_PARALLEL()/16   ] = _pt_real._f[0                    ] + w2 * real_factor;
    _pt_imag._f[SIMD_PARALLEL()/16   ] = _pt_imag._f[0                    ];
    /*   /   /  3 /  6 */
    _pt_real._f[3*SIMD_PARALLEL()/16 ] = _pt_real._f[SIMD_PARALLEL()/2    ] - w2 * real_factor;
    _pt_imag._f[3*SIMD_PARALLEL()/16 ] = _pt_imag._f[SIMD_PARALLEL()/2    ];
    /*   /   /  9 / 18 */
    _pt_real._f[9*SIMD_PARALLEL()/16 ] = _pt_real._f[SIMD_PARALLEL()/2    ];
    _pt_imag._f[9*SIMD_PARALLEL()/16 ] = _pt_imag._f[SIMD_PARALLEL()/2    ] + h2 * imag_factor;
    /*   /   / 11 / 22 */
    _pt_real._f[11*SIMD_PARALLEL()/16] = _pt_real._f[SIMD_PARALLEL()/4    ];
    _pt_imag._f[11*SIMD_PARALLEL()/16] = _pt_imag._f[SIMD_PARALLEL()/4    ] - h2 * imag_factor;
    /*   /   /  5 / 10 */
    _pt_real._f[5*SIMD_PARALLEL()/16 ] = _pt_real._f[SIMD_PARALLEL()/4    ] - w2 * real_factor;
    _pt_imag._f[5*SIMD_PARALLEL()/16 ] = _pt_imag._f[SIMD_PARALLEL()/4    ];
    /*   /   /  7 / 14 */
    _pt_real._f[7*SIMD_PARALLEL()/16 ] = _pt_real._f[3*SIMD_PARALLEL()/4  ] + w2 * real_factor;
    _pt_imag._f[7*SIMD_PARALLEL()/16 ] = _pt_imag._f[3*SIMD_PARALLEL()/4  ];
    /*   /   / 13 / 26 */
    _pt_real._f[13*SIMD_PARALLEL()/16] = _pt_real._f[3*SIMD_PARALLEL()/4  ];
    _pt_imag._f[13*SIMD_PARALLEL()/16] = _pt_imag._f[3*SIMD_PARALLEL()/4  ] - h2 * imag_factor;
    /*   /   / 15 / 30 */
    _pt_real._f[15*SIMD_PARALLEL()/16] = _pt_real._f[0                    ];
    _pt_imag._f[15*SIMD_PARALLEL()/16] = _pt_imag._f[0                    ] + h2 * imag_factor;
    #endif

    #if TARGETED_IMPLEMENTATION >= FRACTAL_FLOAT_AVX1024_ASM  /* AVX1024? */
    /*   /   /    /  1 */
    _pt_real._f[SIMD_PARALLEL()/32   ] = _pt_real._f[0                    ] + w4 * real_factor;
    _pt_imag._f[SIMD_PARALLEL()/32   ] = _pt_imag._f[0                    ];
    /*   /   /    /  3 */
    _pt_real._f[3*SIMD_PARALLEL()/32 ] = _pt_real._f[SIMD_PARALLEL()/8    ] - w4 * real_factor;
    _pt_imag._f[3*SIMD_PARALLEL()/32 ] = _pt_imag._f[SIMD_PARALLEL()/8    ];
    /*   /   /    /  5 */
    _pt_real._f[5*SIMD_PARALLEL()/32 ] = _pt_real._f[SIMD_PARALLEL()/8    ] + w4 * real_factor;
    _pt_imag._f[5*SIMD_PARALLEL()/32 ] = _pt_imag._f[SIMD_PARALLEL()/8    ];
    /*   /   /    /  7 */
    _pt_real._f[7*SIMD_PARALLEL()/32 ] = _pt_real._f[SIMD_PARALLEL()/2    ] - w4 * real_factor;
    _pt_imag._f[7*SIMD_PARALLEL()/32 ] = _pt_imag._f[SIMD_PARALLEL()/2    ];
    /*   /   /    / 17 */
    _pt_real._f[17*SIMD_PARALLEL()/32] = _pt_real._f[SIMD_PARALLEL()/2    ];
    _pt_imag._f[17*SIMD_PARALLEL()/32] = _pt_imag._f[SIMD_PARALLEL()/2    ] + h4 * imag_factor;
    /*   /   /    / 19 */
    _pt_real._f[19*SIMD_PARALLEL()/32] = _pt_real._f[5*SIMD_PARALLEL()/8  ];
    _pt_imag._f[19*SIMD_PARALLEL()/32] = _pt_imag._f[5*SIMD_PARALLEL()/8  ] - h4 * imag_factor;
    /*   /   /    / 21 */
    _pt_real._f[21*SIMD_PARALLEL()/32] = _pt_real._f[5*SIMD_PARALLEL()/8  ];
    _pt_imag._f[21*SIMD_PARALLEL()/32] = _pt_imag._f[5*SIMD_PARALLEL()/8  ] + h4 * imag_factor;
    /*   /   /    / 23 */
    _pt_real._f[23*SIMD_PARALLEL()/32] = _pt_real._f[SIMD_PARALLEL()/4    ];
    _pt_imag._f[23*SIMD_PARALLEL()/32] = _pt_imag._f[SIMD_PARALLEL()/4    ] - h4 * imag_factor;
    /*   /   /    /  9 */
    _pt_real._f[9*SIMD_PARALLEL()/32 ] = _pt_real._f[SIMD_PARALLEL()/4    ] - w4 * real_factor;
    _pt_imag._f[9*SIMD_PARALLEL()/32 ] = _pt_imag._f[SIMD_PARALLEL()/4    ];
    /*   /   /    / 11 */
    _pt_real._f[11*SIMD_PARALLEL()/32] = _pt_real._f[3*SIMD_PARALLEL()/8  ] + w4 * real_factor;
    _pt_imag._f[11*SIMD_PARALLEL()/32] = _pt_imag._f[3*SIMD_PARALLEL()/8  ];
    /*   /   /    / 13 */
    _pt_real._f[13*SIMD_PARALLEL()/32] = _pt_real._f[3*SIMD_PARALLEL()/8  ] - w4 * real_factor;
    _pt_imag._f[13*SIMD_PARALLEL()/32] = _pt_imag._f[3*SIMD_PARALLEL()/8  ];
    /*   /   /    / 15 */
    _pt_real._f[15*SIMD_PARALLEL()/32] = _pt_real._f[3*SIMD_PARALLEL()/4  ] + w4 * real_factor;
    _pt_imag._f[15*SIMD_PARALLEL()/32] = _pt_imag._f[3*SIMD_PARALLEL()/4  ];
    /*   /   /    / 25 */
    _pt_real._f[25*SIMD_PARALLEL()/32] = _pt_real._f[3*SIMD_PARALLEL()/4  ];
    _pt_imag._f[25*SIMD_PARALLEL()/32] = _pt_imag._f[3*SIMD_PARALLEL()/4  ] - h4 * imag_factor;
    /*   /   /    / 27 */
    _pt_real._f[25*SIMD_PARALLEL()/32] = _pt_real._f[7*SIMD_PARALLEL()/8  ];
    _pt_imag._f[25*SIMD_PARALLEL()/32] = _pt_imag._f[7*SIMD_PARALLEL()/8  ] + h4 * imag_factor;
    /*   /   /    / 29 */
    _pt_real._f[29*SIMD_PARALLEL()/32] = _pt_real._f[7*SIMD_PARALLEL()/8  ];
    _pt_imag._f[29*SIMD_PARALLEL()/32] = _pt_imag._f[7*SIMD_PARALLEL()/8  ] - h4 * imag_factor;
    /*   /   /    / 31 */
    _pt_real._f[31*SIMD_PARALLEL()/32] = _pt_real._f[0                    ];
    _pt_imag._f[31*SIMD_PARALLEL()/32] = _pt_imag._f[0                    ] + h4 * imag_factor;
    #endif

    SIMD_FractalPixelColor (&_pt_real, &_pt_imag, &_rect_clr);

    /* Are all _rect_clr.ui [] values equal? */
    #if defined(BORDER_COLOR_MEMCMP)
    { __m128i *border_clr = (__m128i *)&_rect_clr;

      if (_rect_clr._ui[0] != _rect_clr._ui[1]
       || _rect_clr._ul[0] != _rect_clr._ul[1]
       #if TARGETED_IMPLEMENTATION >= FRACTAL_FLOAT_AVX1_ASM
       || memcmp (border_clr, border_clr + 1, sizeof(*border_clr)) != 0
       #if TARGETED_IMPLEMENTATION >= FRACTAL_FLOAT_AVX512_ASM
       || memcmp (border_clr, border_clr + 2, 2*sizeof(*border_clr)) != 0
       #if TARGETED_IMPLEMENTATION >= FRACTAL_FLOAT_AVX1024_ASM
       || memcmp (border_clr, border_clr + 4, 4*sizeof(*border_clr)) != 0
       #endif
       #endif
       #endif
         )
       goto Partition;
    }
    #else
  for (i=0; i < SIMD_PARALLEL()-1; ++i)
      if (_rect_clr._ui [i] != _rect_clr._ui [i+1])
        goto Partition;
    #endif

/*  SIMD4: */
/*    x x                                   2 = 1 */
/*    x * x                                 3 = 2 */
/*    x * * x                               4 = 3 */
/*    x * * * x                             5 = 4 */
/* ⇒ steps = (r - l + 1) - 1 = (r - l + 0) / 1; */
/* ⇒ minsz = 2 = 2^0 + 1 */

/*SIMD8: */
/*    x x x                                 3 = 1 */
/*    x * x x                               4 = 2 */
/*    x * x * x                             5 = 2 */
/*    x * * x * x                           6 = 3 */
/*    x * * x * * x                         7 = 3 */
/*    x * * * x * * x                       8 = 4 */
/*    x * * * x * * * x                     9 = 4 */
/* ⇒ steps = (r - l + 1) / 2 = (r - l + 1) / 2 */
/* ⇒ minsz = 3 = 2^1 + 1 */

/*SIMD16: */
/*    x x x x x                             5 = 1 */
/*    x x * x x x                           6 = 2 */
/*    x x * x * x x                         7 = 2 */
/*    x * x * x x * x                       8 = 2 */
/*    x * x * x * x * x                     9 = 2 */
/*    x * x * * x * x * x                  10 = 3 */
/*    x * x * * x * * x * x                11 = 3 */
/*    x * * x * * x * x * * x              12 = 3 */
/*    x * * x * * x * * x * * x            13 = 3 */
/*    x * * x * * * x * * x * * x          14 = 4 */
/* ⇒ steps = ((r - l + 1) + 2) / 4 = (r - l + 3) / 4 */
/* ⇒ minsz = 5 = 2^2 + 1 */

/*  SIMD32: */
/*    x x x x x x x x x                     9 = 1 */
/*    x x x * x x x x x x                  10 = 2 */
/*    x * x * x * x * x * x * x * x * x    17 = 2 */
/*    x * x * x * * x * x * x * x * x * x  18 = 3 */
/* ⇒ steps = ((r - l + 1) + 6) / 4 = (r - l + 7) / 8 */
/* ⇒ minsz = 9 = 2^3 + 1 */

/*  SIMDn: */
/* ⇒ steps = (w + (n/4) - 2) / (n/4) */
/* ⇒ minsz = 2^[ld(n)-2] + 1 = (n/4) + 1 */
    { int j;

      for (i=max(stepsx, stepsy)-1; i > 0; i--, stepsx--, stepsy--)
      {
        if (stepsx > 1)
          for (j=0; j < SIMD_PARALLEL()/4; j++)
          {
            _pt_real._f[j                      ] += real_factor;
            _pt_real._f[j +   SIMD_PARALLEL()/4] -= real_factor;
          }

        if (stepsy > 1)
          for (j=0; j < SIMD_PARALLEL()/4; j++)
          {
            _pt_imag._f[j +   SIMD_PARALLEL()/2] += imag_factor;
            _pt_imag._f[j + 3*SIMD_PARALLEL()/4] -= imag_factor;
          }

        SIMD_FractalPixelColor (&_pt_real, &_pt_imag, &_pt_clr);

        if (memcmp (&_pt_clr, &_rect_clr, sizeof(_pt_clr)) != 0)
          goto Partition;
      }
    }

    if (drawFractal)
      DrawFilledRectangleLS (l, u, r, b, _rect_clr._ui[0]);

    if (drawPartitions)
    {
      #define FRAME_INSET 0
      DrawLineLS (l+FRAME_INSET, u+FRAME_INSET, r-FRAME_INSET, u+FRAME_INSET, (unsigned char)TEST_PARTITION);
      DrawLineLS (r-FRAME_INSET, u+FRAME_INSET, r-FRAME_INSET, b-FRAME_INSET, (unsigned char)TEST_PARTITION);
      DrawLineLS (l+FRAME_INSET, b-FRAME_INSET, r-FRAME_INSET, b-FRAME_INSET, (unsigned char)TEST_PARTITION);
      DrawLineLS (l+FRAME_INSET, u+FRAME_INSET, l+FRAME_INSET, b-FRAME_INSET, (unsigned char)TEST_PARTITION);
      #undef FRAME_INSET
    }
    return;

Partition:;
    level++;

    if (w > h)
    {
      i = (l + r) >> 1;
      FractalDrawPartition (l,   u, i, b);
      FractalDrawPartition (i+1, u, r, b);
    }
    else
    {
      i = (u + b) >> 1;
      FractalDrawPartition (l, u,   r, i);
      FractalDrawPartition (l, i+1, r, b);
    }
  }

  level--;
}


/***************************   Draw single screen  ***************************/
#elif defined(MANDEL_SINGLE_SCREEN)
static BOOL FractalDrawScreen (int l, int u, int r, int b)
{ /* Local: */
  #if SIMD_FLT_COUNT > 1
  SIMD_FLT
    pt_real,
    pt_imag;
  SIMD_INT
    pt_clr;
  #else
  #endif
  unsigned long
    c;
  SCREEN_MAPPING
    c_real,
    c_curr,
    c_imag;
  int
    w;

  c_imag = cimagf (leftUpper [0]) + u * imag_factor;
  c_real = crealf (leftUpper [0]) + l * real_factor;
  w      = r-l+1;

  for (; u <= b; u++)
  {
    #if SIMD_FLT_COUNT > 1
    for (c=0; c < SIMD_PARALLEL(); c++)
      pt_imag._f[c] = c_imag;
    #endif
    c_curr = c_real;
    (void)c_curr;  /*error: variable 'c_curr' set but not used */

    while (l <= r)
    {
    #if defined(FRACTAL_SCREEN_COORDS) && !FRACTAL_SIMD_OPTIMIZATION
      FractalPixelColor ((COMPLEX_BASE_T)l, (COMPLEX_BASE_T)u, c, TRUE);
      PutPixelLS (l, u, c);
      l++;
    #else
      #if SIMD_FLT_COUNT > 1
      for (c=0; c < SIMD_PARALLEL(); c++)
      {
        pt_real._f[c] = c_curr;
        c_curr += real_factor;
      }
      SIMD_FractalPixelColor (&pt_real, &pt_imag, &pt_clr);
      for (c=0; c < SIMD_PARALLEL(); c++)
        PutPixelLS (l + c, u, pt_clr._ui[c]);
      l += SIMD_PARALLEL();
      #else
      FractalPixelColor (c_curr, c_imag, c, FALSE);
      c_curr += real_factor;
      PutPixelLS (l, u, c);
      l++;
      #endif
    #endif
    }

    c_imag += imag_factor;
    l -= w;
  }

  return (TRUE);
}


/***************************   Draw line-by-line  ****************************/
#elif defined(MANDEL_SINGLE_PIXEL) || defined(MANDEL_SINGLE_LINE)
static BOOL FractalDrawPixelLine (int l, int u, int r, int b)
{ static int
    y = INT_MAX,
    x;
  static float
    c_real,
    c_imag;
  unsigned long
    c;

  if (y > b)
  {
    y = u-1; c_imag = leftUpper [0].imag + y*imag_factor;
    x = INT_MAX;
  }

  #if defined(MANDEL_SINGLE_PIXEL)
  if (x >= r)
  {
    x = l; c_real = leftUpper [0].real + l*real_factor;
    y++; c_imag += imag_factor;
    if (y > b) return (TRUE);
  }

  FractalPixelColor (c_real, c_imag, c, FALSE);
  x++; c_real += real_factor;
  PutPixelLS (x, y, c);
  #else
  y++; c_imag += imag_factor;
  if (y > b) return (TRUE);

  for (x = l, c_real = leftUpper [0].real + l*real_factor; x <= r; x++)
  {
    FractalPixelColor (c_real, c_imag, c, FALSE);
    c_real += real_factor;
    PutPixelLS (x, y, c);
  }
  #endif

  return (FALSE);
}
#endif
