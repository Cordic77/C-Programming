/*#define _POSIX_C_SOURCE  199309L*/  /* struct timespec, clock_gettime() */
#define _POSIX_C_SOURCE  200112L  /* pthread_win32_process_attach_np(), pthread_win32_process_detach_np() */

#if defined(_WIN32)
#include <windows.h>              /* Win32 API: QueryPerformanceXxx() */
#elif defined(__MACH__)
#include <sys/param.h>
#include <sys/sysctl.h>           /* Mac OS X: sysctl() */
#else
#include <unistd.h>               /* POSIX API: sysconf() */
#endif

#include <stdio.h>                /* fprintf() */
#include <limits.h>               /* LONG_MAX */
#include <errno.h>                /* errno */
#include <float.h>                /* FLT_MIN, FLT_MAX */
#include <time.h>                 /* time(), Linux: struct timespec, clock_getres(), clock_gettime() */
#include <iso646.h>               /* not */

#include <getopt.h>               /* getopt_long() */
#include <pthread.h>              /* pthread_create() */

#include "stdinc.h"

#if SDL_VER > 1                   /* Simple DirectMedia Layer */
  WARNING_PUSH()
  MSCWARN_DISABLE(4127)           /* _MSC: warning C4127: conditional expression is constant */
  #include <SDL2/SDL.h>
  WARNING_POP()
#else
  #include <SDL/SDL.h>
#endif

#include "Complex_ADT.h"              /* Complex */
#include "FloatPoint.h"           /* FPU_set_control_word(), SSE_set_control_word() */
#include "Graphics.h"             /* InitSDL() */
#include "Fractal.h"              /* FractalSetCoordinates() */
#include "DrawFractal.h"          /* FractalDraw() */

#include "/SDK/stdlib/OSVersion.c"


/* Preprocessor macros: */
#ifndef NANOSECONDS_PER_SECOND
#define NANOSECONDS_PER_SECOND  1000000000ull  /* 1s = 1.000.000.000 ns */
#endif


/* Imports: */
extern BOOL
  drawFractal;                    /* d  draw fractal? */
extern BOOL
  drawPartitions;                 /* p  draw partitioning grid? */

/* Exports: */
int
  logCores;                       /* Number of available (logical) cores */
int
  nrThreads,                      /* Number of parallel threads? */
  maxThreads;                     /* Maximum number of possible threads? */
BOOL
  quit = FALSE;                   /* Quit application? */

/* Globals: */
static BOOL
  appname_printed = FALSE;        /* Application name already printed? */


/* Function declarations: */
static void
  PrintAppName (void);
static void
  DisplayUsageAndExit (void);
static void
  ParseCommandLine (int argc, char *argv[]);

static void
  MandelbrotViewer (void);
static int
  NumberOfPhysicalCores (void);
static int
  NumberOfLogicalCores (void);


/* Program entry point: */
int main (int argc, char *argv[])
{ struct FpControlWord old_cw;

  #include "/SDK/stdlib/Win32UTF8.c"

  /* Initialization: */
  ParseCommandLine (argc, argv);

  if (InitSDL () != 0)
  {
    fprintf (stderr, "%s", SDL_GetError ());
    exit (1);
  }

  /* Floating Point Control Word: set single precision, round towards zero: */
  /* ATTN: needs to be set for every thread (even with pthreads) */
  old_cw.fpu = FPU_set_control_word (FPU_PC_SINGLE_PREC | FPU_RC_ROUND_ZERO,
                                     FPU_PC_MASK        | FPU_RC_MASK);
  old_cw.sse = SSE_set_control_word (MXCSR_RC_ROUND_ZERO, MXCSR_RC_MASK);

  /* Pthreads: */
  #if defined(_WIN32) && !defined(_DLL)
  pthread_win32_process_attach_np ();
  #endif
  logCores = NumberOfLogicalCores ();
  nrThreads = 0;
  maxThreads = 4*logCores;

  /* Fractal viewer: */
  MandelbrotViewer ();

  /* Cleanup: */
  #if defined(_WIN32) && !defined(_DLL)
  pthread_win32_process_detach_np ();
  #endif

  /*_control87 (_CW_DEFAULT, 0xFFFF);  // _MSC_VER: Set FPU back to default state */
  FPU_set_control_word (old_cw.fpu, FPU_ALL_MASK);
  SSE_set_control_word (old_cw.sse, MXCSR_ALL_MASK);

  SDL_Quit ();
  return (0);
}


/* Function definitions: */
static void PrintAppName (void)
{
  if (not appname_printed)
  {
    fprintf (stdout, "Mandelbrot 1.4"
                     #ifdef _WIN64
                     " x64"
                     #else
                     " x86"
                     #endif
                     ", (c) 2019 MiSt\n\n");
    appname_printed = TRUE;
  }
}


static void DisplayUsageAndExit (void)
{
  fprintf (stdout,
"Arguments: [OPTION]\n\n"
"--width=<pixel>, -w <pixel>\n"
"    Horizontal resolution (default: %d)\n"
"--height=<pixel>, -h <pixel>\n"
"    Vertical resolution (default: %d)\n"
"--fullscreen (default), --window\n"
"    Full-screen or window mode?\n"
"--alternate-palette, -p\n"
"    Use alternate 256 color palette?\n"
"-?, --help\n"
"    Display this help and exit\n\n"
"Keyboard shortcuts:\n"
"b ... enable benchmarking mode\n"
"d ... enable/disable drawing code\n"
#if defined(MANDEL_MARIANI_SILVER_SUBDIVISION)
"p ... draw Mariani/Silver subdivision?\n"
#endif
"t ... enable Multithreading\n"
"+ ... increase number of threads\n"
"- ... decrease number of threads\n"
"q ... quit application (ESC also works!)\n", GetResolutionX(), GetResolutionY());
  exit (1);
}


static void ParseCommandLine (int argc, char *argv[])
{ struct option long_options[] = {
      /* These options set a flag: */
/*    {"verbose",             no_argument,        &verbose_flag,  1 }, */

      /* These options don't set a flag. We distinguish them by their indices: */
      {"width",               required_argument,  NULL,          'w'},
      {"height",              required_argument,  NULL,          'h'},
      {"fullscreen",          no_argument,        NULL,           1 },
      {"window",              no_argument,        NULL,           2 },
      {"alternate-palette",   no_argument,        NULL,          'p'},
      {"help",                no_argument,        NULL,          '?'},

      {0,0,0,0}
    };

  for (;;)
  {
    /* getopt_long() stores the option index here: */
    int option_character;   /* last option character */
    int option_index = 0;   /* last option index */

    option_character = getopt_long (argc, argv, "whp?", long_options, &option_index);

    if (option_character == -1)
    {
      if (argv[optind] != NULL && _tcsicmp(argv[optind], TEXT("/?")) == 0)
        option_character = '?';
      else
        break;
    }

    switch (option_character)
    {
    case 'w':
    case 'h':
      if (optarg == NULL)
        optarg = argv [optind];

      if (optarg != NULL)
      { char *endptr = NULL;
        long  resolution = 0;

        while (*optarg != '\0' && not isdigit (*optarg))
          optarg++;

        if (isdigit (*optarg))
        {
          errno = 0;
          resolution = strtol (optarg, &endptr, 10);
        }

        if ((resolution == 0 && errno != 0)
          || ((resolution == LONG_MAX || resolution == LONG_MIN) && errno == ERANGE)
          || (endptr == NULL || endptr == optarg))
        {
          PrintAppName ();
          fprintf (stderr, "Invalid resolution '%s'!\n\n", optarg);
          DisplayUsageAndExit ();
        }

        if (option_character == 'w')
          screen_width = resolution;
        else
          screen_height = resolution;
      }
      else
      {
        PrintAppName ();
        DisplayUsageAndExit ();
      }
      break;

    case 1:
      fullscreen = TRUE;
      break;

    case 2:
      fullscreen = FALSE;
      break;

    case 'p':
      alt_palette = TRUE;
      break;

    case '?':
      PrintAppName ();
      DisplayUsageAndExit ();
      break;

    default:
      /* We won't actually get here! */
      break;
    }
  }

  if ((screen_width > 0) != (screen_height > 0))
  {
    PrintAppName ();
    fprintf (stderr, "Please specify both the width and the height!\n\n");
    DisplayUsageAndExit ();
  }
}


static void MandelbrotViewer (void)
{ /* Timing Code: */
  BOOL
    benchmark = FALSE;            /* Benchmark fractal calculation? */
  BOOL
    threading = FALSE;            /* Use multithreading? */
  BOOL
    resetime  = FALSE;            /* Reset starting time? */
  int
    iter  = 0;                    /* How many iterations do we have so far? */

  /* Genaue Zeitmessung: */
  #if defined(_WIN32)
  #define HPET_AVAILABLE
  long long
    ts_f  = 0ll,                  /* HPET frequency (High Performance Timer) */
    ts_s  = 0ll,                  /* Starting time [rdtsc]. */
    ts_c;                         /* Current time [rdtsc]. */
  #elif defined(__USE_POSIX199309)
  #define HPET_AVAILABLE
  struct timespec
    ts_f  = {(time_t)0, (long)0}, /* HPET frequency (High Performance Timer). */
    ts_s  = {(time_t)0, (long)0}, /* Starting time [rdtsc]. */
    ts_c;                         /* Current time [rdtsc]. */
  #endif

  /* Grobe Zeitmessung: */
  #define SDL_GETTICKS 1          /* SDL_GetTicks(): number of milliseconds since the SDL library initialization */
  long int
    #ifndef HPET_AVAILABLE
    start = 0,                    /* Starting time [ms]. */
    #endif
    last  = 0,                    /* Last (saved) timestamp [ms]. */
    curr;                         /* Current timestamp [ms]. */

  /* SDL: */
  SDL_Event
    event;                        /* Last polled event */
  #if SDL_VER > 1
  Uint32
  #else
  Uint8
  #endif
    mouseb;                       /* Any mouse keys pressed? */
  int
    mousex, mousey;               /* Mouse position x/y */
  int
    zoomdir;                      /* Zoom direction (<0: zoom in, >0: zoom out). */
  float
    zoomfact = 1.0;               /* Current zoom factor */

  /* Statistik: */
  char
    mpos [64 + 1] = " ",          /* Mouse position string */
    fps [127 + 1] = " ",          /* FPS string */
    info [63 + 1] = "Zoom:  1.00000%";/* Zoom settings string */

  /* Inital fractal view: */
  FractalSetView (1, TRUE);

  /* */
  curr = 0;

  for (;;)
  {
    /* Draw fractal: */
    #if !defined(MANDEL_SINGLE_PIXEL) && !defined(MANDEL_SINGLE_LINE)
    BeginDraw ();
    while (not FractalDraw (0, 0, screen_width-1, screen_height-1));
    EndDraw ();
    iter++;
    #if SDL_GETTICKS == 1
    curr = SDL_GetTicks();
    #endif
    if (not benchmark)
    #else
    BeginDraw ();
    if (FractalDraw (0, 0, screen_width-1, screen_height-1))
      iter++;
    EndDraw ();
    curr = SDL_GetTicks();
    if (not benchmark || (curr-last) < 1000)
    #endif
    {
      DrawStringULS (0, 0, mpos);
      FlipScreen ();
    }

    /* Quit program? */
    if (quit)
      break;

    /* Perform HPET measurement once every second: */
    if (benchmark && (curr-last) >= 1000)
    { double diff;
      int    y;

      #ifdef HPET_AVAILABLE
        #if defined(_WIN32)
        QueryPerformanceCounter ((LARGE_INTEGER *)&ts_c);
        diff = (ts_c - ts_s) / (double)ts_f;
        #else
        clock_gettime (CLOCK_PROCESS_CPUTIME_ID, &ts_c);
        diff = NANOSECONDS_PER_SECOND*(ts_c.tv_sec - ts_s.tv_sec)
             + ts_c.tv_nsec - ts_s.tv_nsec;
        diff /= NANOSECONDS_PER_SECOND;
        #endif
      #else
        diff = (curr - start) / (double)1000;
      #endif

      #if defined(UPDATE_SET_DIMENSIONS)
      if (FIXED2FLOAT(crealf (minInSet)) == FLT_MAX || FIXED2FLOAT(cimagf (minInSet)) == FLT_MAX ||
          FIXED2FLOAT(crealf (maxInSet)) == FLT_MIN || FIXED2FLOAT(cimagf (maxInSet)) == FLT_MIN)
      #endif
        snprintf (fps, sizeof(fps), "%2d:  %6.2lf fps "
                                    #if defined(CALCULATE_OP_STATISTICS)
                                    " %llu add, %llu mul "
                                    #endif
            , iter, iter / diff
            #if defined(CALCULATE_OP_STATISTICS)
            , add_count, mul_count
            #endif
          );
      #if defined(UPDATE_SET_DIMENSIONS)
      else
        snprintf (fps, sizeof(fps), "%2d:  %6.2lf fps "
                                    #if defined(CALCULATE_OP_STATISTICS)
                                    " %llu add, %llu mul "
                                    #endif
                                    #if defined(UPDATE_SET_DIMENSIONS)
                                    " [min: %4.3f + %4.3fi, max: %4.3f + %4.3fi]  "
                                    #endif
            , iter, iter / diff
            #if defined(CALCULATE_OP_STATISTICS)
            , add_count, mul_count
            #endif
            #if defined(UPDATE_SET_DIMENSIONS)
            , FIXED2FLOAT(crealf (minInSet)), FIXED2FLOAT(cimagf (minInSet))
            , FIXED2FLOAT(crealf (maxInSet)), FIXED2FLOAT(cimagf (maxInSet))
            #endif
          );
      #endif

      y = 0;
      DrawStringULS (0, y, mpos);
      y += 19;
      #if !defined(MANDEL_SINGLE_PIXEL) && !defined(MANDEL_SINGLE_LINE)
      DrawStringULS (0, y, info);
      y += 19;
      #endif
      DrawStringULS (0, y, fps);
      FlipScreen ();

      last = curr;
    }

    /* Keyboard/mouse events? */
    while (SDL_PollEvent (&event))
    {
      switch (event.type)
      {
      /* Main window closed by user? */
      case SDL_QUIT:
        quit = TRUE;
        break;

      /* Key release event? */
      case SDL_KEYUP:
        switch (event.key.keysym.sym)
        {
        case SDLK_b:
          benchmark = TRUE;

          /* Reset starting time: */
          #ifdef HPET_AVAILABLE
            #if defined(_WIN32)
            QueryPerformanceFrequency ((LARGE_INTEGER *)&ts_f);
            #else
            clock_getres (CLOCK_PROCESS_CPUTIME_ID, &ts_f);
            #endif
          #endif
          resetime = TRUE;
          break;

        case SDLK_d:
          drawFractal = !drawFractal;
          if (not drawFractal && not drawPartitions)
            drawPartitions = TRUE;
          break;

        case SDLK_p:
          drawPartitions = !drawPartitions;
          if (not drawFractal && not drawPartitions)
            drawFractal = TRUE;
          break;

        case SDLK_t:
          threading = TRUE;

          /* Increase nrThreads by one: */
          event.type = SDL_KEYUP;
          event.key.keysym.sym = SDLK_PLUS;
          SDL_PushEvent (&event);
          break;

        case SDLK_EQUALS:
          if (SDL_GetModState()!=KMOD_LSHIFT && SDL_GetModState()!=KMOD_RSHIFT)
            break;
          event.key.keysym.sym = SDLK_KP_PLUS;
          /* see https://stackoverflow.com/questions/45129741/gcc-7-wimplicit-fallthrough-warnings-and-portable-way-to-clear-them#answer-45137452 */
          /* fall through */
        case SDLK_PLUS:
        case SDLK_MINUS:
        case SDLK_KP_PLUS:
        case SDLK_KP_MINUS:
          if (benchmark)
          {
            #if !defined(MANDEL_SINGLE_PIXEL) && !defined(MANDEL_SINGLE_LINE)
            int newCount;

            if (event.key.keysym.sym == SDLK_KP_PLUS || event.key.keysym.sym == SDLK_PLUS)
              newCount = min (nrThreads+1, maxThreads);
            else
              newCount = max (1, nrThreads-1);

            if (newCount != nrThreads)
            {
              if (threading)
              {
                nrThreads = newCount;
                snprintf (info, sizeof(info), "Threads:  %d, Zoom:  %6.5f", nrThreads, zoomfact);
                resetime = TRUE;
              }
            }
            #endif
          }
          break;

        case SDLK_ESCAPE:
        case SDLK_q:
          quit = TRUE;
        }
        break;

      /* Mouse movement? */
      case SDL_MOUSEMOTION:
        { float
            real, imag;

          FractalGetMousePos (event.motion.x, event.motion.y, &real, &imag);
          snprintf (mpos, sizeof(mpos), "Position:  %4.3f + %4.3f i  ", real, imag);
        }
        break;
      }
    }

    /* Mouse button event? */
    zoomdir = 0;
    mouseb = SDL_GetMouseState (&mousex, &mousey);
    if (mouseb & SDL_BUTTON(SDL_BUTTON_LEFT))  zoomdir = -1; else
    if (mouseb & SDL_BUTTON(SDL_BUTTON_RIGHT)) zoomdir = +1;

    if (zoomdir != 0)
    {
      zoomfact = FractalZoom (mousex, mousey, zoomdir);
      if (threading)
        snprintf (info, sizeof(info), "Threads:  %d, Zoom:  %6.5f", nrThreads, zoomfact);
      else
        snprintf (info, sizeof(info), "Zoom:  %6.5f", zoomfact);
      resetime = TRUE;
      FlipScreen ();
    }

    if (resetime)
    {
      resetime = FALSE;
      iter = 0;
      #ifdef HPET_AVAILABLE
        #if defined(_WIN32)
        QueryPerformanceCounter ((LARGE_INTEGER *)&ts_s);
        #else
        clock_gettime (CLOCK_PROCESS_CPUTIME_ID, &ts_s);
        #endif
      #else
        start = SDL_GetTicks ();
      #endif
    }
  }
}


#if defined(_WIN32)
typedef BOOL (WINAPI *IsWow64ProcessPtr) ( /* WinBase.h */
          HANDLE  hProcess,
          PBOOL   Wow64Process
        );

typedef BOOL (WINAPI *GetLogicalProcessorInformationPtr) (
          PSYSTEM_LOGICAL_PROCESSOR_INFORMATION Buffer,
          PDWORD ReturnedLength
        );

static BOOL IsWow64 (void)
{ BOOL
    isWow64   = FALSE;
  HMODULE
    hKernel32 = NULL;
  IsWow64ProcessPtr
    IsWow64ProcessFunc = NULL;

  if ((hKernel32=GetModuleHandle("kernel32"))!=NULL)
  {
    IsWow64ProcessFunc = (IsWow64ProcessPtr)
      GetProcAddress (hKernel32, TEXT("IsWow64Process"));
  }

  if (IsWow64ProcessFunc != NULL)
  {
    if (not IsWow64ProcessFunc(GetCurrentProcess(), &isWow64))
      isWow64 = FALSE;
  }

  return (isWow64);
}
#endif


static int NumberOfPhysicalCores (void)
{
  #if defined(_WIN32)
  SYSTEM_INFO
    si;

  /* Windows XP pre SP2 does not make any distinction between logical and
       physical processors. To maintain compatibility, XP SP2/SP3 continues
       to report logical processors. */
  /* On Windows Vista and Windows 7 this function reports the number of
       physical processors. */
  if (IsWow64 ())
    GetNativeSystemInfo (&si);
  else
    GetSystemInfo (&si);

  return (si.dwNumberOfProcessors);
  #elif defined(__MACH__)
  int
    nm [2];
  size_t
    len = 4;
  uint32_t
    count;

  nm[0] = CTL_HW;
  nm[1] = HW_AVAILCPU;
  sysctl (nm, 2, &count, &len, NULL, 0);

  if (count < 1)
  {
    nm[1] = HW_NCPU;
    sysctl(nm, 2, &count, &len, NULL, 0);
    if (count < 1)
      count = 1;
  }

  return (count);
  #else
  return (sysconf (_SC_NPROCESSORS_ONLN));
  #endif
}


/* Fast sideways addition  see http://graphics.stanford.edu/~seander/bithacks.html: */
static INLINE int BitCount32 (int v)
{
  v = v - ((v >> 1) & 0x55555555);
  v = (v & 0x33333333) + ((v >> 2) & 0x33333333);

  return ((((v + (v >> 4)) & 0xF0F0F0F) * 0x1010101) >> 24);
}


/* See http://fluid-particles.googlecode.com/svn/trunk/CoreDetection/CpuTopology.cpp: */
static int NumberOfLogicalCores (void)
{
  #if defined(_WIN32)
  HMODULE
    hKernel32 = NULL;
  GetLogicalProcessorInformationPtr
    GetLogicalProcessorInformationFunc = NULL;
  int
    detLogCores = 0;

  if ((hKernel32=GetModuleHandle("kernel32"))!=NULL)
  {
    GetLogicalProcessorInformationFunc = (GetLogicalProcessorInformationPtr)
      GetProcAddress (hKernel32, TEXT("GetLogicalProcessorInformation"));
  }

  if (GetLogicalProcessorInformationFunc != NULL)
  { DWORD
      bufferSize = 0;
    SYSTEM_LOGICAL_PROCESSOR_INFORMATION
     *slpi = NULL;

    GetLogicalProcessorInformationFunc (NULL, &bufferSize);

    if ((slpi=(SYSTEM_LOGICAL_PROCESSOR_INFORMATION *)malloc (bufferSize)) != NULL)
    { SYSTEM_LOGICAL_PROCESSOR_INFORMATION
       *curr = slpi;
      DWORD
        byteOffset = 0;

      if (!GetLogicalProcessorInformationFunc (slpi, &bufferSize))
        free (slpi);
      else
      {
        while (byteOffset < bufferSize)
        {
          switch (curr->Relationship)
          {
          case RelationProcessorCore:
            if (curr->ProcessorCore.Flags)
              detLogCores++;
            else
              detLogCores += BitCount32 ((int)curr->ProcessorMask);
            break;
          }

          curr++;
          byteOffset += sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
        }

        free (slpi);
        return (detLogCores);
      }
    }
  }
  #endif

  /* Simply assume that there are twice as many logical cores: */
  return (2 * NumberOfPhysicalCores());
}
