/* config.h  */

#ifndef  __PTW32_CONFIG_H
#define  __PTW32_CONFIG_H

/*********************************************************************
 * Defaults: see target specific redefinitions below.
 *********************************************************************/

/* We're building the pthreads-win32 library */
#define  __PTW32_BUILD

/* CPU affinity */
#define HAVE_CPU_AFFINITY

/* Do we know about the C type sigset_t? */
#undef HAVE_SIGSET_T

/* Define if you have the <signal.h> header file.  */
#undef HAVE_SIGNAL_H

/* Define if you have the Borland TASM32 or compatible assembler.  */
#undef HAVE_TASM32

/* Define if you don't have Win32 DuplicateHandle. (eg. WinCE) */
#undef NEED_DUPLICATEHANDLE

/* Define if you don't have Win32 _beginthreadex. (eg. WinCE) */
#undef NEED_CREATETHREAD

/* Define if you don't have Win32 errno. (eg. WinCE) */
#undef NEED_ERRNO

/* Define if you don't have Win32 calloc. (eg. WinCE)  */
#undef NEED_CALLOC

/* Define if you don't have Win32 semaphores. (eg. WinCE 2.1 or earlier)  */
#undef NEED_SEM

/* Define if you need to convert string parameters to unicode. (eg. WinCE)  */
#undef NEED_UNICODE_CONSTS

/* Define if your C (not C++) compiler supports "inline" functions. */
#if defined(__cplusplus)
#undef HAVE_C_INLINE
#else
#define HAVE_C_INLINE 1
#endif

/* Do we know about type mode_t? */
#undef HAVE_MODE_T

/*
 * Define if GCC has atomic builtins, i.e. __sync_* intrinsics
 * __sync_lock_* is implemented in mingw32 gcc 4.5.2 at least
 * so this define does not turn those on or off. If you get an
 * error from __sync_lock* then consider upgrading your gcc.
 */
#undef HAVE_GCC_ATOMIC_BUILTINS

/* Define if you have the timespec struct */
#undef HAVE_STRUCT_TIMESPEC

/* Define if you don't have the GetProcessAffinityMask() */
#undef NEED_PROCESS_AFFINITY_MASK

/* Define if your version of Windows TLSGetValue() clears WSALastError
 * and calling SetLastError() isn't enough restore it. You'll also need to
 * link against wsock32.lib (or libwsock32.a for MinGW).
 */
#undef RETAIN_WSALASTERROR

/*MiSt { // no longer included*/
/* These defines determine the style of cleanup (see pthread.h) and,
 * most importantly, the way that cancelation and thread exit (via
 * pthread_exit) is performed (see the routine ptw32_throw() in private.c).
 *
 * In short, the exceptions versions of the library throw an exception
 * when a thread is canceled or exits (via pthread_exit()), which is
 * caught by a handler in the thread startup routine, so that the
 * the correct stack unwinding occurs regardless of where the thread
 * is when it's canceled or exits via pthread_exit().
 */
#if !(defined(__PTW32_CLEANUP_CXX) || defined(__PTW32_CLEANUP_SEH) || defined(__PTW32_CLEANUP_C))
  #if defined(__cplusplus)
    #define __PTW32_CLEANUP_CXX   1   // C++, including MSVC++, GNU G++
  #elif defined(_WIN32) && defined(_MSC_VER)
    #define __PTW32_CLEANUP_SEH   1   // MSVC only
  #else
    #define __PTW32_CLEANUP_C     1   // C, including GNU GCC, not MSVC
  #endif
#else
  /* ==> see "C/C++ > Preprocessor" */
#endif

/* Define to 1 if the necessary calls to on_process_init() and
 * on_process_exit() should get called automatically, even for static
 * library builds.
 */
#define __PTW32_PROCESS_STATIC_AUTO_INIT 1
/*MiSt }*/

/*
# ----------------------------------------------------------------------
# The library can be built with some alternative behaviour to better
# facilitate development of applications on Win32 that will be ported
# to other POSIX systems.
#
# Nothing described here will make the library non-compliant and strictly
# compliant applications will not be affected in any way, but
# applications that make assumptions that POSIX does not guarantee are
# not strictly compliant and may fail or misbehave with some settings.
#
#  __PTW32_THREAD_ID_REUSE_INCREMENT
# Purpose:
# POSIX says that applications should assume that thread IDs can be
# recycled. However, Solaris (and some other systems) use a [very large]
# sequence number as the thread ID, which provides virtual uniqueness.
# This provides a very high but finite level of safety for applications
# that are not meticulous in tracking thread lifecycles e.g. applications
# that call functions which target detached threads without some form of
# thread exit synchronisation.
#
# Usage:
# Set to any value in the range: 0 <= value < 2^wordsize.
# Set to 0 to emulate reusable thread ID behaviour like Linux or *BSD.
# Set to 1 for unique thread IDs like Solaris (this is the default).
# Set to some factor of 2^wordsize to emulate smaller word size types
# (i.e. will wrap sooner). This might be useful to emulate some embedded
# systems.
#
# define  __PTW32_THREAD_ID_REUSE_INCREMENT 0
#
# ----------------------------------------------------------------------
 */
#undef  __PTW32_THREAD_ID_REUSE_INCREMENT


/*********************************************************************
 * Target specific groups
 *
 * If you find that these are incorrect or incomplete please report it
 * to the pthreads-win32 maintainer. Thanks.
 *********************************************************************/
#if defined(WINCE)
#  undef  HAVE_CPU_AFFINITY
#  define NEED_DUPLICATEHANDLE
#  define NEED_CREATETHREAD
#  define NEED_ERRNO
#  define NEED_CALLOC
#  define NEED_FTIME
/* #  define NEED_SEM */
#  define NEED_UNICODE_CONSTS
#  define NEED_PROCESS_AFFINITY_MASK
/* This may not be needed */
#  define RETAIN_WSALASTERROR
#endif

#if defined(_UWIN)
#  define HAVE_MODE_T
#  define HAVE_STRUCT_TIMESPEC
#  define HAVE_SIGNAL_H
#endif

#if defined(__GNUC__)
#  define HAVE_C_INLINE
#endif

#if defined(__BORLANDC__)
#endif

#if defined(__WATCOMC__)
#endif

#if defined(__DMC__)
#define HAVE_SIGNAL_H
#define HAVE_C_INLINE
#endif

#if defined(_MSC_VER) && _MSC_VER >= 1900
#define HAVE_STRUCT_TIMESPEC
#endif

#endif /*  __PTW32_CONFIG_H */
