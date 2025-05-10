#ifdef _MSC_VER
  #pragma once
#endif
#ifndef STDINCLUDE_H_
#define STDINCLUDE_H_

#include <stdio.h>                    /* fprintf() */
#include <stdlib.h>                   /* exit() */
#include <string.h>                   /* strlcpy() */
#include <iso646.h>                   /* and, not, or, xor */


/* Pointer size, library type: */
#if defined(_WIN32)
  #if defined(_WIN64) || defined(_M_X64)
    #define BUILD_X64
  #else
    #define BUILD_X86
  #endif

  #if !defined(DYNAMIC_LIB) && !defined(STATIC_LIB)
    #if defined(_DLL) || defined(_USRDLL) || defined(_AFXDLL)
      #define DYNAMIC_LIB
    #else
      #define STATIC_LIB
    #endif
  #endif

#elif defined(__linux__) || defined(__MACH__)
  #if defined(__LP64__)
    #define BUILD_X64
  #else
    #define BUILD_X86
  #endif

  #if !defined(DYNAMIC_LIB) && !defined(STATIC_LIB)
    #define DYNAMIC_LIB
  #endif
#else
  #error "Unsupported Platform!"
#endif


/* Boolean type: typedef enum { FALSE, TRUE } BOOL; */
typedef int BOOL;
#if !defined(FALSE)
  #define FALSE                   0
#endif
#if !defined(TRUE)
  #define TRUE                    1
#endif
#define UNDEF                    -1         /* Tristate (kind of) */


/* Byte, Word, Dword, Qword: */
#if !defined(LOBYTE)
  #define LOBYTE(w)               (unsigned char)((w) & 0xFF)
#endif
#if !defined(HIBYTE)
  #define HIBYTE(w)               (unsigned char)((w) >> 8)
#endif
#if !defined(MAKEWORD)
  #define MAKEWORD(lo,hi)         (unsigned short)((LOBYTE(hi) << 8) | LOBYTE(lo))
#endif

#if !defined(LOWORD)
  #define LOWORD(l)               (unsigned short)((l) & 0xFFFFU)
#endif
#if !defined(HIWORD)
  #define HIWORD(l)               (unsigned short)((l) >> 16)
#endif
#if !defined(MAKELONG)
  #define MAKELONG(lo,hi)         (unsigned long)((LOWORD(hi) << 16) | LOWORD(lo))
#endif


/* ASCII / UNICODE: */
#if !defined(_countof)
  #define _countof(arr)           (sizeof(arr)/sizeof(arr[0]))
#endif

#if defined(UNICODE) || defined(_UNICODE)
  #define TCHAR                             wchar_t
  #if !defined(TEXT)
  #define TEXT(quote)                       L##quote
  #endif

  #if defined(__linux__)
  #define _tmain                            main
  #else
  #define _tmain                            wmain
  #endif

  /* String functions: */
  #define _ftprintf                         fwprintf
  #define _sntprintf                        _snwprintf
  #define _tcserr_r(errnum,buf,buflen)      _wcserror_s (buf,buflen,errnum)
  #define _tcschr                           wcschr
  #define _tcsicmp                          wcsicmp
  #define _tcslcat(dst,src,dstlen)          wcscat_s (dst,dstlen,src)
  #define _tcslcpy(dst,src,dstlen)          wcscpy_s (dst,dstlen,src)
  #define _tcslen                           wcslen
  #define _tprintf                          wprintf
  #define _vsntprintf                       _vsnwprintf

  /* Character classification: */
  #define _istprint                         iswprint

#else
  #define TCHAR                             char
  #if !defined(TEXT)
  #define TEXT(quote)                       quote
  #endif

  #define _tmain                            main

  /* String functions: */
  #if defined(__linux__)
  #define _sntprintf                        snprintf
  #define _tcserr_r                         strerror_r
  #define _tcsicmp                          strcasecmp
  #define _tcslcat                          strlcat
  #define _tcslcpy                          strlcpy
  #define _vsntprintf                       vsnprintf
  #else
  #define _sntprintf                        _snprintf
  #define _tcserr_r(errnum,buf,buflen)      strerror_s (buf,buflen,errnum)
  #define _tcsicmp                          stricmp
  #define _tcslcat(dst,src,dstlen)          strcat_s (dst,dstlen,src)
  #define _tcslcpy(dst,src,dstlen)          strcpy_s (dst,dstlen,src)
  #define _vsntprintf                       _vsnprintf
  #endif

  #define _ftprintf                         fprintf
  #define _tcschr                           strchr
  #define _tprintf                          printf
  #define _tcslen                           strlen

  /* Character classification: */
  #if defined(__linux__)
  #define _istprint                         isprint
  #else
  #define _istprint                         _ismbcprint
  #endif
#endif

#define _TCHAR                              TCHAR

#define NTS                                 TEXT('\0')
#define CR                                  TEXT('\r')
#define LF                                  TEXT('\n')


/* Structures: padding, alignment, visibility: */
#if defined(_MSC_VER)
  #define PACK(DECLARATION)                                                  \
__pragma(pack (push, 1))                                                     \
DECLARATION;                                                                 \
__pragma(pack (pop))

  #define PACKALIGN(ALIGN,DECLARATION)                                       \
__pragma(pack (push, 1))                                                     \
_declspec(align (ALIGN))                                                     \
DECLARATION;                                                                 \
__pragma(pack (pop))

  #define WARNING_PUSH()                                                     \
__pragma(warning (push))
  #define GCCWARN_DISABLE(warning_str)
  #define MSCWARN_DISABLE(number)                                            \
__pragma(warning (disable : number))
  #define WARNING_POP()                                                      \
__pragma(warning (pop))

#else
  #define PACK(DECLARATION)                                                  \
DECLARATION                                                                  \
__attribute__ ((packed));

  #define PACKALIGN(ALIGN,DECLARATION)                                       \
DECLARATION                                                                  \
__attribute__ ((aligned (ALIGN), packed));

  #define WARNING_PUSH()                                                     \
_Pragma("GCC diagnostic push")
  #define GCCWARN_DISABLE(warning_str)                                       \
_Pragma("GCC diagnostic ignored \"" warning_str "\"")
  #define MSCWARN_DISABLE(number)
  #define WARNING_POP()                                                      \
_Pragma("GCC diagnostic pop")
#endif

#if defined(BUILD_X64)
  #define OffsetOf(s,m)           (size_t)( (ptrdiff_t)&(((struct s *)0)->m) )
#else
  #define OffsetOf(s,m)           (size_t)&( ((struct s *)0)->m )
#endif

#define PRIVATE()
#define PROTECTED()
#define PUBLIC()


/* Variable alignment: */
#if defined(_MSC_VER)
  #define ALIGN(ALIGN,DECLARATION)                                           \
_declspec(align (ALIGN))                                                     \
DECLARATION
#else
  #define ALIGN(ALIGN,DECLARATION)                                           \
DECLARATION;                                                                 \
__attribute__ ((aligned (ALIGN)));
#endif


/* Functions: visibility, inlining, arguments: */
/*
// INLINE EXTERN LIB_API <rtype> CALLCONV <fname> (IN args, OUT res);
*/
#if defined(__cplusplus)
  #define INLINE inline
#else
  #define INLINE __inline
#endif

#if defined(__cplusplus)
  #define EXTERN   extern
  #define EXTERN_C extern "C"
#else
  #define EXTERN   extern
  #define EXTERN_C extern
#endif

#if defined(__GNUC__)
  #define CDECL   __attribute__((cdecl))
#else
  #if !defined(CDECL)
  #define CDECL   __cdecl
  #endif
#endif

#define IN
#define OUT
#define REF


/* Extensions: math.h */
#if !defined(min)
  #define min(a,b)  (((a) < (b)) ? (a) : (b))
#endif
#if !defined(max)
  #define max(a,b)  (((a) > (b)) ? (a) : (b))
#endif


/* Windows-specific: */
#if !defined(PACKVERSION)
  #define PACKVERSION(major,minor)  MAKELONG(minor,major)
#endif
#if !defined(MAJOR_VERSION)
  #define MAJOR_VERSION(packvers)   HIWORD(packvers)
#endif
#if !defined(MINOR_VERSION)
  #define MINOR_VERSION(packvers)   LOWORD(packvers)
#endif


/* Various Preprocessor macros: */
#if !defined(UNREFERENCED_PARAMETER)
  #define UNREFERENCED_PARAMETER(x) (void)x
#endif


/* Static assert (-> http://stackoverflow.com/questions/174356/ways-to-assert-expressions-at-build-time-in-c): */
#ifdef __GNUC__
#define STATIC_ASSERT_HELPER(expr, msg) \
  (!!sizeof (struct { unsigned int STATIC_ASSERTION__##msg: (expr) ? 1 : -1; }))
#define STATIC_ASSERT(expr, msg) \
  extern int (*assert_function__(void)) [STATIC_ASSERT_HELPER(expr, msg)]
#else
#define STATIC_ASSERT(expr, msg)   \
  extern char STATIC_ASSERTION__##msg[1]; \
  extern char STATIC_ASSERTION__##msg[(expr)?1:2]
#endif /* #ifdef __GNUC__ */


/* Default includes: */
#include "Config/Warnings.h"
#include "Config/Settings.h"

#endif
