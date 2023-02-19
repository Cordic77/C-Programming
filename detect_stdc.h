#ifndef DETECT_STDC_H_
#define DETECT_STDC_H_

#if defined(_MSC_VER)  /* Microsoft Visual C++ Compiler? */
  #if !defined(__STDC_VERSION__) || __STDC_VERSION__ <= 199409L
    #if _MSC_VER >= 1928    /* Visual Studio 2019, Visual C++ v16.8 (or higher) */
      /* Support for _Pragma, restrict, _Noreturn, _Alignas, _Alignof, _Generic, _Static_assert:
         See https://devblogs.microsoft.com/cppblog/c11-and-c17-standard-support-arriving-in-msvc/ */
      #define STD__C_VERSION 201112L  /* (C17/18 is just a “bug fix release” and basically the same as C11.) */

      /* Visual Studio 2019 version 16.8 still defaults to ‘Default ("Legacy MSVC")’, but finally
         adds compiler switches to select specific (C/C++) language standards: /std:c11, /std:c17
         and sets __STDC_VERSION__ accordingly (otherwise set to 199409L by default). */
    #elif _MSC_VER >= 1900    /* Visual Studio 2015, Visual C++ v14 (or higher) */
      /* Most “C99” language features and good library support; e.g. adds "uchar.h" and snprintf (among others):
         See https://devblogs.microsoft.com/cppblog/c11-and-c17-standard-support-arriving-in-msvc/ */
      #define STD__C_VERSION 199901L
    #endif

    #if _MSC_VER >= 1100    /* Visual Studio 97, Visual C++ 5.0 (or higher) */
      #if _MSC_VER >= 1600    /* Visual Studio 2010 v10 (or higher) */
        /* While <inttypes.h> was still missing in VS2010, it did ship whith a <stdint.h> header: */
        #define HAVE_STDINT_H 1

        #if _MSC_VER >= 1800    /* Visual Studio 2013, Visual C++ v12 (or higher) */
          /* _Bool type, Compound literals, Designated initializers, Mixing declarations/code, __func__ macro:
             https://blogs.msdn.microsoft.com/vcblog/2013/07/19/c99-library-support-in-visual-studio-2013/ */
          #ifndef STD__C_VERSION
          #define STD__C_VERSION 199901L
          #endif
          #define HAVE_INTTYPES_H 1
          #define HAVE_STDBOOL_H 1
        #endif
      #else
        /* Visual C++ 5.0 already shipped with a "VCPP-5.00\DEVSTUDIO\VC\CRT\SRC\ISO646.H": */
        #ifndef STD__C_VERSION
        #define STD__C_VERSION 199409L
        #endif
      #endif
    #endif

    #if defined(STD__C_VERSION) && !defined(__STDC_VERSION__)
      #define __STDC_VERSION__ 1  /* (Big “no-no” in general; works, however, if not set by MSC.) */
    #endif
  #endif
#endif

/* ISO/IEC 9899 „C“ standard conformance? */
#if defined(__STDC_VERSION__)
  #ifndef STD__C_VERSION  /* Set equal to __STDC_VERSION__ for compilers other than MSC. */
  #define STD__C_VERSION __STDC_VERSION__
  #endif
  #if STD__C_VERSION >= 201710L       /* C17/C18 */ /* ISO/IEC 9899:2018 */
    #define C_STD    2017
  #elif STD__C_VERSION >= 201112L     /* C11 */     /* ISO/IEC 9899:2011 */
    #define C_STD    2011
  #elif STD__C_VERSION >= 199901L     /* C99 */     /* ISO/IEC 9899:1999 */
    #define C_STD    1999
  #elif STD__C_VERSION >= 199409L     /* C94/C95 */ /* ISO/IEC 9899/AMD1:1995, Normative Addendum 1 (NA1) */
    #define C_STD    1995
  #endif
#else  /* __STDC_VERSION__ was added in “C95”, Normative Addendum 1 (NA1) */
  #if defined(__GNUC__)               /* Note: __GNUC__ is also set by other (GCC compatible) compilers. */
    #define C_STD    1989
  #elif defined(_MSC_VER)
    #if _MSC_VER >= 600               /* MSC5.1 stuck closely to the ANSI standard; MSC6 is just about on the money. */
    #define C_STD    1989             /* http://www.drdobbs.com/windows/optimizing-with-microsoft-c-60/184408398 */
    #else
    #define C_STD    1978             /* K&R C: “The C Programming Language”, 1st Edition (1978) */
    #endif
  #endif
#endif

#endif  /* DETECT_STDC_H_ */
