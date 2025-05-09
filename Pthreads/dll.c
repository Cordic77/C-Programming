/*
 * dll.c
 *
 * Description:
 * This translation unit implements DLL initialisation.
 *
 * --------------------------------------------------------------------------
 *
 *      Pthreads4w - POSIX Threads for Windows
 *      Copyright 1998 John E. Bossom
 *      Copyright 1999-2018, Pthreads4w contributors
 *
 *      Homepage: https://sourceforge.net/projects/pthreads4w/
 *
 *      The current list of contributors is contained
 *      in the file CONTRIBUTORS included with the source
 *      code distribution. The list can also be seen at the
 *      following World Wide Web location:
 *
 *      https://sourceforge.net/p/pthreads4w/wiki/Contributors/
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#if defined(__PTW32_STATIC_LIB) && defined(_MSC_VER) && _MSC_VER >= 1400
#  undef __PTW32_STATIC_LIB
#  define __PTW32_STATIC_TLSLIB
#endif

#include "pthread.h"
#include "implement.h"

#if !defined (__PTW32_STATIC_LIB)

#if defined(_MSC_VER)
/*
 * lpvReserved yields an unreferenced formal parameter;
 * ignore it
 */
#pragma warning( disable : 4100 )
#endif

#if defined(__cplusplus)
/*
 * Dear c++: Please don't mangle this name. -thanks
 */
extern "C"
#endif				/* __cplusplus */
BOOL WINAPI
#if defined(__PTW32_STATIC_TLSLIB)
PTW32_StaticLibMain (HINSTANCE hinstDll, DWORD fdwReason, LPVOID lpvReserved)
#else
DllMain (HINSTANCE hinstDll, DWORD fdwReason, LPVOID lpvReserved)
#endif
{
  BOOL result =  __PTW32_TRUE;

  switch (fdwReason)
    {

    case DLL_PROCESS_ATTACH:
      result = pthread_win32_process_attach_np ();
      break;

    case DLL_THREAD_ATTACH:
      /*
       * A thread is being created
       */
      result = pthread_win32_thread_attach_np ();
      break;

    case DLL_THREAD_DETACH:
      /*
       * A thread is exiting cleanly
       */
      result = pthread_win32_thread_detach_np ();
      break;

    case DLL_PROCESS_DETACH:
      (void) pthread_win32_thread_detach_np ();
      result = pthread_win32_process_detach_np ();
      break;
    }

  return (result);

}				/* DllMain */

#endif /* !PTW32_STATIC_LIB */

#if ! defined (__PTW32_BUILD_INLINED)
/*
 * Avoid "translation unit is empty" warnings
 */
typedef int foo;
#endif

/* Visual Studio 8+ can leverage PIMAGE_TLS_CALLBACK CRT segments, which
 * give a static lib its very own DllMain.
 */
#ifdef __PTW32_STATIC_TLSLIB

static void WINAPI
TlsMain(PVOID h, DWORD r, PVOID u)
{
  (void)PTW32_StaticLibMain((HINSTANCE)h, r, u);
}

#ifdef _M_X64
# pragma comment (linker, "/INCLUDE:_tls_used")
# pragma comment (linker, "/INCLUDE:_xl_b")
# pragma const_seg(".CRT$XLB")
EXTERN_C const PIMAGE_TLS_CALLBACK _xl_b = TlsMain;
# pragma const_seg()
#else
# pragma comment (linker, "/INCLUDE:__tls_used")
# pragma comment (linker, "/INCLUDE:__xl_b")
# pragma data_seg(".CRT$XLB")
EXTERN_C PIMAGE_TLS_CALLBACK _xl_b = TlsMain;
# pragma data_seg()
#endif /* _M_X64 */

#endif /* __PTW32_STATIC_TLSLIB */

#if defined(__PTW32_STATIC_LIB)

/*
 * Note: MSVC 8 and higher use code in dll.c, which enables TLS cleanup
 * on thread exit. Code here can only do process init and exit functions.
 */

#if defined(__MINGW32__) || defined(_MSC_VER)

/* For an explanation of this code (at least the MSVC parts), refer to
 *
 * http://www.codeguru.com/cpp/misc/misc/threadsprocesses/article.php/c6945/
 * ("Running Code Before and After Main")
 *
 * Compatibility with MSVC8 was cribbed from Boost:
 *
 * http://svn.boost.org/svn/boost/trunk/libs/thread/src/win32/tss_pe.cpp
 *
 * In addition to that, because we are in a static library, and the linker
 * can't tell that the constructor/destructor functions are actually
 * needed, we need a way to prevent the linker from optimizing away this
 * module. The pthread_win32_autostatic_anchor() hack below (and in
 * implement.h) does the job in a portable manner.
 */

static int on_process_init(void)
{
    pthread_win32_process_attach_np ();
    return 0;
}

static int on_process_exit(void)
{
    pthread_win32_thread_detach_np  ();
    pthread_win32_process_detach_np ();
    return 0;
}

#if defined(__GNUC__)
__attribute__((section(".ctors"), used)) static int (*gcc_ctor)(void) = on_process_init;
__attribute__((section(".dtors"), used)) static int (*gcc_dtor)(void) = on_process_exit;
#elif defined(_MSC_VER)
#  if _MSC_VER >= 1400 /* MSVC8+ */
#    pragma section(".CRT$XCU", long, read)
#    pragma section(".CRT$XPU", long, read)
__declspec(allocate(".CRT$XCU")) static int (*msc_ctor)(void) = on_process_init;
__declspec(allocate(".CRT$XPU")) static int (*msc_dtor)(void) = on_process_exit;
#  else
#    pragma data_seg(".CRT$XCU")
static int (*msc_ctor)(void) = on_process_init;
#    pragma data_seg(".CRT$XPU")
static int (*msc_dtor)(void) = on_process_exit;
#    pragma data_seg() /* reset data segment */
#  endif
#endif

#endif /* defined(__MINGW32__) || defined(_MSC_VER) */

/* This dummy function exists solely to be referenced by other modules
 * (specifically, in implement.h), so that the linker can't optimize away
 * this module. Don't call it.
 */
void __ptw32_autostatic_anchor(void) { abort(); }

#endif /*  __PTW32_STATIC_LIB */

