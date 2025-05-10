#ifndef M_DETECT_COMPILER_H_
#define M_DETECT_COMPILER_H_

/* In this header file, we try to detect the currently running compiler, and to set the following
   preprocessor macros which indicate conformance to various versions of the C and C++ language
   standards:

   C_STD is set to one of the following values:    1978 | 1989 | 1999 | 2011 | 2017 ( | 202X )
   CPP_STD is set to one of the following values:  1985 | 1998 | 2003 | 2011 | 2014 | 2017 ( | 2020 )

   #if __cplusplus is undefined (or zero), the compiler was invoked without C++ support. Other-
   wise, the value of this preprocessor macro should indicate the supported language standard:

   __cplusplus == 1:        C++ compiler with pre-C++98 support
   __cplusplus == 199711L:  C++98 support
  (__cplusplus == 200310L:  C++03 compilers usually don't differentiate, but we try to define it anyway.)
   __cplusplus == 201103L:  C++11 support
   __cplusplus == 201402L:  C++14 support
   __cplusplus == 201703L:  C++17 support
   __cplusplus == 202002L:  C++20 support

   Unfortunately, however, some compiler vendors haven't properly updated this preprocessor
   macro until quite recently:

   ⇒ GNU Compiler Collection: Bug 1773 - __cplusplus defined to 1, should be 199711L
                              https://gcc.gnu.org/bugzilla/show_bug.cgi?id=1773
                              ... this was fixed with the gcc 4.7.0 release!

   ⇒ Visual C++ Compiler:    MSVC now correctly reports __cplusplus
                              https://devblogs.microsoft.com/cppblog/msvc-now-correctly-reports-__cplusplus/
                              ... from v15.7.0 onwards, Visual Studio 2017 now correctly sets this
                              macro if the "/Zc:__cplusplus" command line option is specified!

   We try to circumvent this by directly checking for the various compiler versions:
   https://en.cppreference.com/w/cpp/compiler_support#C.2B.2B14_features

   N.b.: C++20 includes standardized preprocessor macros to test for various C++ features:
   https://en.cppreference.com/w/cpp/feature_test
*/

/* Intel C++ Compiler? */
#if defined(__INTEL_COMPILER)
  #define ICC_VER __INTEL_COMPILER
#elif defined(__ICL)
  #define ICC_VER __ICL       /* On Linux and OS X */
#elif defined(__ICC)
  #define ICC_VER __ICC       /* On Windows */
#elif defined(__ECC)
  #define ICC_VER __ECC       /* Of historic interest only: Itanium! */
#endif

#if defined(ICC_VER)  /* Intel C++ Compiler? */
  #define compiler_prefix() icc

  /* Fix for “Intel Composer XE 2011 Update 6” (v12.10), which reported an incorrect value for __INTEL_COMPILER: */
  #if ICC_VER == 9999 && __INTEL_COMPILER_BUILD_DATE == 20110811
     #undef ICC_VER
     #define ICC_VER 1210
  #endif

  /* C++17 Features Supported by Intel C++ Compiler
     https://software.intel.com/en-us/articles/c17-features-supported-by-intel-c-compiler */

  /* C++14 Features Supported by Intel C++ Compiler
     https://software.intel.com/en-us/articles/c14-features-supported-by-intel-c-compiler */

  /* C++11 Features Supported by Intel C++ Compiler
     https://software.intel.com/en-us/articles/c0x-features-supported-by-intel-c-compiler */

/* http://clang.llvm.org/docs/LanguageExtensions.html */
#elif defined(__clang__)  /* LLVM / Clang Compiler? */
  #define compiler_prefix() clang
  #define CLANG_VER     (__clang_major__*10000 + __clang_minor__*100 + __clang_patchlevel__)

  /* Initial C17 Language Support Lands In LLVM Clang 6.0 SVN
     https://www.phoronix.com/scan.php?page=news_item&px=LLVM-Clang-C17-Support */

  /* C++ Support in Clang
     http://clang.llvm.org/cxx_status.html
     ⇒ Clang 5 and later implement all the features of the ISO C++ 2017 standard
     ⇒ Clang 3.4 and later implement all of the ISO C++ 2014 standard
     ⇒ Clang 3.3 and later implement all of the ISO C++ 2011 standard */

  /* LLVM's Clang 3.1 Compiler Betters C11, C++11
     https://www.phoronix.com/scan.php?page=news_item&px=MTA5NzU */

  /* Clang had its initial release on 2007-09-26. As such it has always had support for C++03! */
  #if __cplusplus <= 199711L
    #define std__cplusplus 200310L
  #endif

/* http://gcc.gnu.org/onlinedocs/cpp/Predefined-Macros.html */
#elif defined(__GNUC__)  /* GNU Compiler Collection? */
/*#if !defined(ICC_VER) && !defined(__clang__)*/
    #define compiler_prefix() gcc
    #define GCC_VER     (__GNUC__*10000 + __GNUC_MINOR__*100 + __GNUC_PATCHLEVEL__)

    #if GCC_VER >= 40700
      /* GCC now correctly reports __cplusplus! */
      #if __cplusplus <= 199711L
        #define std__cplusplus 200310L
      #endif
    #else
      #if GCC_VER >= 40600
        /* GCC 4.6.0 shipped with “reasonable” support for C++11
           https://gcc.gnu.org/gcc-4.6/cxx0x_status.html */
        #define std__cplusplus 201103L

      #elif GCC_VER >= 30400
        /* Formally, with versions starting from 3.4, GCC/libstdc++ only guarantees C++98/C++03 ABI stability and not more.
           https://wiki.gentoo.org/wiki/Upgrading_GCC#The_special_case_C.2B.2B11_.28and_C.2B.2B14.29 */
        #define std__cplusplus 200310L
     #endif
    #endif
/*#endif*/
#endif

/* Microsoft Visual C++ Compiler? */
#if defined(_MSC_VER)
  #define compiler_prefix() msvc

  /* What is the default version of a header file?
     http://blogs.msdn.com/b/oldnewthing/archive/2007/04/11/2079137.aspx

     #define WINVER          0x0400   // Symbol that 16-bit Windows used to control the versioning of its
                                      // header files, and its use carried forward into the 32-bit headers
     #define _WIN32_WINNT    0x0400   // Invented by the Windows NT team to guard Windows NT [3.1] specific
                                      // functions (for example, both NT4 and Win95 had a major version of 4)
     #define _WIN32_WINDOWS  0x0400   // Get access to functions which were first introduced in Windows 95
                                      // and did not exist in the original version of Windows NT 4
     #define _WIN32_IE       0x0400   // What version of Internet Explorer is required to be installed on
                                      // on the users system?

     In an attempt to make some sense out of this disaster, the SDK and DDK teams came up with a new plan
     for Windows Vista header files: sdkddkver.h. There's now just one symbol you define to specify your
     minimum target operating system:

     http://msdn.microsoft.com/en-us/library/aa383745(VS.85).aspx
     #define NTDDI_VERSION 0x05010000 // Minimum value (targets Windows XP RTM)

     Once you set that, all the other symbols are set automatically to the appropriate values for your
     target operating system:

     http://wishmesh.com/2009/09/microsoft-visual-c-supported-target-windows-versions/
  */
  #if _MSC_VER >= 1930
    #define VS17_2022       _MSC_VER    /* 2021-11-08: MFC 14.30(.30704.0), .NET 6.0(.100) */
    #define MSC_SUFFIX      "vs2022"
    #if _MSC_FULL_VER < 193030705
      #define VS17_2022_BETA _MSC_FULL_VER  /* Beta */
    #endif

  #elif _MSC_VER >= 1920
    #define VS16_2019       _MSC_VER    /* 2019-04-02: MFC 14.20(.27508.1), .NET 4.8 */
    #define MSC_SUFFIX      "vs2019"
    #if _MSC_FULL_VER < 192027508
      #define VS15_2019_BETA _MSC_FULL_VER  /* Beta */
    #endif

  #elif _MSC_VER >= 1910
    #define VS15_2017       _MSC_VER    /* 2017-03-07: MFC 14.10(.25008.0), .NET 4.7 */
    #define MSC_SUFFIX      "vs2017"
    #if _MSC_FULL_VER < 191025017
      #define VS15_2017_BETA _MSC_FULL_VER  /* Beta */
    #endif

    #ifndef NTDDI_VERSION
    #define NTDDI_VERSION   0x06030000  /* Visual Studio 2017 requires Windows 8.1 or later */
    #endif
    #ifndef _WIN32_WINNT
    #define _WIN32_WINNT    0x0602      /* Windows 8 / Windows 8.1 */
    #endif
  #elif _MSC_VER >= 1900
    #define VS14_2015       _MSC_VER    /* 2015-07-20: MFC 14.0(.23026.0) , .NET 4.6 */
    #define MSC_SUFFIX      "vs2015"
    #if _MSC_FULL_VER < 190023026
      #define VS14_2015_BETA _MSC_FULL_VER  /* Beta */
    #endif

    #ifndef NTDDI_VERSION
    #define NTDDI_VERSION   0x06010000  /* Visual Studio 2015 requires Windows 7 SP 1 or later */
    #endif
    #ifndef _WIN32_WINNT
    #define _WIN32_WINNT    0x0601      /* Windows 7 / Windows 7 SP1 */
    #endif
  #elif _MSC_VER >= 1800
    #define VS12_2013       _MSC_VER    /* 2013-10-17: MFC 12.0(.21005.1), .NET 4.5.1 */
    #define MSC_SUFFIX      "vs2013"
    #if _MSC_FULL_VER < 180021005
      #define VS12_2013_BETA _MSC_FULL_VER  /* Beta */
    #endif

    #ifndef NTDDI_VERSION
    #define NTDDI_VERSION   0x06010000  /* Visual Studio 2013 requires Windows 7 SP 1 or later */
    #endif
    #ifndef _WIN32_WINNT
    #define _WIN32_WINNT    0x0601      /* Windows 7 / Windows 7 SP1 */
    #endif
  #elif _MSC_VER >= 1700
    #define VS11_2012       _MSC_VER    /* 2012-09-12: MFC 11.0(.50727.1), .NET 4.5 */
    #define MSC_SUFFIX      "vs2012"
    #if _MSC_FULL_VER < 170050727
      #define VS11_2012_BETA _MSC_FULL_VER  /* Beta */
    #endif

    #ifndef NTDDI_VERSION
    #define NTDDI_VERSION   0x06010000  /* Visual Studio 2012 requires Windows 7 RTM or later */
    #endif
    #ifndef _WIN32_WINNT
    #define _WIN32_WINNT    0x0601      /* Windows 7 */
    #endif
  #elif _MSC_VER >= 1600
    #define VS10_2010       1           /* 2010-04-12: MFC 10.0(.30319.1), .NET 4.0 */
    #define MSC_SUFFIX      "vs2010"
    #if _MSC_FULL_VER < 160030319
      #define VS10_2010_BETA _MSC_FULL_VER  /* Beta */
    #endif

    #ifndef NTDDI_VERSION
    #define NTDDI_VERSION   0x05010200  /* Visual Studio 2010 requires Windows XP SP2 or later */
    #endif
    #ifndef _WIN32_WINNT
    #define _WIN32_WINNT    0x0501      /* Windows XP */
    #endif
  #elif _MSC_VER >= 1500
    #define VS9_2008        1           /* 2007-11-19: MFC 9.0(.21022.8), .NET 3.5 */
    #define MSC_SUFFIX      "vs2008"
    #if _MSC_FULL_VER < 150021022
      #define VS9_2008_BETA _MSC_FULL_VER  /* Beta */
    #endif

    #ifndef WINVER
    #define WINVER          0x0501      /* Windows XP */
    #endif
    #ifndef _WIN32_WINNT
    #define _WIN32_WINNT    0x0501      /* Windows XP */
    #endif
    #ifndef _WIN32_WINDOWS
    #define _WIN32_WINDOWS  0x0501      /* Visual Studio 2008 requires Windows XP later */
    #endif
    #ifndef _WIN32_IE
    #define _WIN32_IE       0x0501      /* Internet Explorer 5.01 or later */
    #endif
  #elif _MSC_VER >= 1400
    #define VS8_2005        1           /* 2005-11-07: MFC 8.0(.50727.42), .NET 2.0 */
    #define MSC_SUFFIX      "vs2005"
    #if _MSC_FULL_VER < 140050320
      #define VS8_2005_BETA _MSC_FULL_VER  /* Beta */
    #endif

    #ifndef WINVER
    #define WINVER          0x0400      /* Windows NT4 */
    #endif
    #ifndef _WIN32_WINNT
    #define _WIN32_WINNT    0x0400      /* Windows NT4 */
    #endif
    #ifndef _WIN32_WINDOWS
    #define _WIN32_WINDOWS  0x0410      /* Visual Studio 2005 requires Windows 98 or later */
    #endif
    #ifndef _WIN32_IE
    #define _WIN32_IE       0x0401      /* Internet Explorer 4.01 or later */
    #endif

    /* There is a hack for Windows 95 tough:
       http://ma.wishmesh.com/2009/09/run-exe-dll-compiled-with-microsoft-visual-c-2005-under-windows-95/
       Open your .EXE file with Hex editor. Find string ‘IsDebuggerPresent’ and replace it
       with ‘GetCurrentProcess’ (or any other Import from kernel32.dll, that has the same
       length and does not take any parameters) */
  #elif _MSC_VER >= 1310
    #define VS71_2003       1           /* 2003-04-24: MFC 7.1, .NET 1.1 */
    #define MSC_SUFFIX      "vs2003"
    #if _MSC_FULL_VER < 13103077
      #define VS71_2003_BETA _MSC_FULL_VER  /* Beta */
    #endif

    #ifndef WINVER
    #define WINVER          0x0400      /* Windows NT4 */
    #endif
    #ifndef _WIN32_WINNT
    #define _WIN32_WINNT    0x0400      /* Windows NT4 */
    #endif
    #ifndef _WIN32_WINDOWS
    #define _WIN32_WINDOWS  0x0400      /* Visual Studio 2003 requires Windows 95 or later */
    #endif
    #ifndef _WIN32_IE
    #define _WIN32_IE       0x0200      /* Internet Explorer 2.0 or later */
    #endif
  #elif _MSC_VER >= 1300
    #define VS7_2002        1           /* 2002-02-13: MFC 7.0, .NET 1.0 */
    #define MSC_SUFFIX      "vs2002"
    #if _MSC_FULL_VER < 13009466
      #define VS7_2002_BETA _MSC_FULL_VER  /* Beta */
    #endif

    #ifndef WINVER
    #define WINVER          0x0400      /* Windows NT4 */
    #endif
    #ifndef _WIN32_WINNT
    #define _WIN32_WINNT    0x0400      /* Windows NT4 */
    #endif
    #ifndef _WIN32_WINDOWS
    #define _WIN32_WINDOWS  0x0400      /* Visual Studio 2002 requires Windows 95 or later */
    #endif
    #ifndef _WIN32_IE
    #define _WIN32_IE       0x0200      /* Internet Explorer 2.0 or later */
    #endif
  #elif _MSC_VER >= 1200
    #define VS6_98          1           /* 1998-06-xx: MFC 6.0, Visual Studio 98 (6.0): RTM, SP1-SP5 */
    #define MSC_SUFFIX      "vs60"

    #ifndef WINVER
    #define WINVER          0x0400      /* Windows NT4 */
    #endif
    #ifndef _WIN32_WINNT
    #define _WIN32_WINNT    0x0400      /* Windows NT4 */
    #endif
    #ifndef _WIN32_WINDOWS
    #define _WIN32_WINDOWS  0x0400      /* Visual Studio 1998 requires Windows 95 or later */
    #endif
    #ifndef _WIN32_IE
    #define _WIN32_IE       0x0200      /* Internet Explorer 2.0 or later */
    #endif
  #elif _MSC_VER >= 1100
    #define VS5_97          1           /* 1997-xx-xx: MFC 4.21, Visual Studio 97 (5.0) */
    #define MSC_SUFFIX      "vs50"
  #elif _MSC_VER >= 1020
    #define VC_42           1           /* 1995-03-xx: MFC 4.2, Visual C++ 4.2 */
    #define MSC_SUFFIX      "vs42"
  #elif _MSC_VER >= 1010
    #define VC_41           1           /* 1995-03-xx: MFC 4.1, Visual C++ 4.1 */
    #define MSC_SUFFIX      "vs41"
  #elif _MSC_VER >= 1000
    #define VC_40           1           /* 1995-03-xx: MFC 4.0, Visual C++ 4.0 */
    #define MSC_SUFFIX      "vs40"
  #elif _MSC_VER >= 900
    #define VC_20           1           /* 1994-10-xx: MFC 3.0, Visual C++ 2.0: 32-bit compiler */
    #define MSC_SUFFIX      "vs20"
  #elif _MSC_VER >= 800
    #define VC_10           1           /* 1993-xx-xx: MFC 2.0, Visual C++ 1.0: 16/32-bit compiler */
    #define MSC_SUFFIX      "vs10"
  #elif _MSC_VER >= 700
    #define VC_70           1           /* 1992-xx-xx: MFC 1.0, Microsoft C/C++ 7.0 */
    #define MSC_SUFFIX      "msc70"
  #elif _MSC_VER >= 600
    #define MSC_6X          1           /* 1990-xx-xx: C-Compiler 6.x (DOS) */
    #define MSC_SUFFIX      "msc6x"
  #elif _MSC_VER >= 510
    #define MSC_51          1           /* 1988-xx-xx: C-Compiler 5.1 (DOS) */
    #define MSC_SUFFIX      "msc51"
  #endif

  /* For a full list see: https://dev.to/yumetodo/list-of-mscver-and-mscfullver-8nd
                          https://github.com/rizonesoft/Notepad3/blob/master/src/Version.h */

/*#if _MSC_VER >= 1930*/  /* Visual C++ 2022 (or higher)? */
    #if _MSC_VER >= 1943
      #define VS17_2022_U13     _MSC_FULL_VER /* v17.13.x */
    #elif _MSC_VER >= 1942
      #define VS17_2022_U12     _MSC_FULL_VER /* v17.12.x */
    #elif _MSC_VER >= 1941
      #define VS17_2022_U11     _MSC_FULL_VER /* v17.11.x */
    #elif _MSC_VER >= 1940
      #define VS17_2022_U10     _MSC_FULL_VER /* v17.10.x */
    #elif _MSC_VER >= 1939
      #define VS17_2022_U9      _MSC_FULL_VER /* v17.9.x */
    #elif _MSC_VER >= 1938
      #define VS17_2022_U8      _MSC_FULL_VER /* v17.8.x */
    #elif _MSC_VER >= 1937
      #define VS17_2022_U7      _MSC_FULL_VER /* v17.7.x */
    #elif _MSC_VER >= 1936
      #define VS17_2022_U6      _MSC_FULL_VER /* v17.6.x */
    #elif _MSC_VER >= 1935
      #define VS17_2022_U5      _MSC_FULL_VER /* v17.5.x */
    #elif _MSC_VER >= 1934
      #define VS17_2022_U4      _MSC_FULL_VER /* v17.4.x */
    #elif _MSC_VER >= 1933
      #define VS17_2022_U3      _MSC_FULL_VER /* v17.3.x */
    #elif _MSC_VER >= 1932
      #define VS17_2022_U2      _MSC_FULL_VER /* v17.2.x */
    #elif _MSC_VER >= 1931
      #define VS17_2022_U1      _MSC_FULL_VER /* v17.1.x */
    #elif _MSC_VER >= 1930
      #define VS17_2022_RTM     _MSC_FULL_VER /* v17.0.x */
    #endif

    #define VS17_2022_U13_6     194334810     /* v17.13.6 */
    #define VS17_2022_U13_4_5   194334809     /* v17.13.[45] */
    #define VS17_2022_U13_3     194334808     /* v17.13.3  //TODO: welche _MSC_FULL_VER hat diese Version? */
    #define VS17_2022_U13_0_2   194334808     /* v17.13.[0-2] */

    #define VS17_2022_U12_4     194234436     /* v17.12.4 */
    #define VS17_2022_U12_2_3   194234435     /* v17.12.[23] */
    #define VS17_2022_U12_0_1   194234433     /* v17.12.[01] */

    #define VS17_2022_U11_5     194134123     /* v17.11.5 */
    #define VS17_2022_U11_0_4   194134120     /* v17.11.[0-4] */

    #define VS17_2022_U10_6     194033814     /* v17.10.6 */
    #define VS17_2022_U10_5     194033813     /* v17.10.5 */
    #define VS17_2022_U10_4     194033812     /* v17.10.4 */
    #define VS17_2022_U10_1_3   194033811     /* v17.10.[1-3] */
    #define VS17_2022_U10_0     194033808     /* v17.10.0 */

    #define VS17_2022_U9_4_7    193933523     /* v17.9.[4-7] */
    #define VS17_2022_U9_3      193933522     /* v17.9.3 */
    #define VS17_2022_U9_2      193933521     /* v17.9.2 */
    #define VS17_2022_U9_1      193933520     /* v17.9.1 */
    #define VS17_2022_U9_0      193933519     /* v17.9.0 */

    #define VS17_2022_U8_6_7    193833135     /* v17.8.[67] */
    #define VS17_2022_U8_4_5    193833134     /* v17.8.[45] */
    #define VS17_2022_U8_3      193833133     /* v17.8.3 */
    #define VS17_2022_U8_0_2    193833130     /* v17.8.[0-2] */

    #define VS17_2022_U7_5_6    193732825     /* v17.7.[56] */
    #define VS17_2022_U7_4      193732824     /* v17.7.4 */
    #define VS17_2022_U7_0_3    193732822     /* v17.7.[0-3] */

    #define VS17_2022_U6_5      193632537     /* v17.6.5 */
    #define VS17_2022_U6_4      193632535     /* v17.6.4 */
    #define VS17_2022_U6_3      193632534     /* v17.6.3 */
    #define VS17_2022_U6_0_2    193632532     /* v17.6.[0-2] */

    #define VS17_2022_U5_4_5    193532217     /* v17.5.[45] */
    #define VS17_2022_U5_3      193532216     /* v17.5.3 */
    #define VS17_2022_U5_0_2    193532215     /* v17.5.[012] */

    #define VS17_2022_U4_5      193431942     /* v17.4.5 */
    #define VS17_2022_U4_3_4    193431937     /* v17.4.[34] */
    #define VS17_2022_U4_2      193431935     /* v17.4.2 */
    #define VS17_2022_U4_1      193431933     /* v17.4.1 */

    #define VS17_2022_U3_4_6    193331630     /* v17.3.[4-6] */
    #define VS17_2022_U3_3      193331629     /* v17.3.3 */

/* v17.3.0 - v17.3.2: missing due to stolen Laptop */
    #define VS17_2022_U2_2_2    193231329     /* v17.2.2 */
/* v17.2.[01], v17.2.[3-7]  missing due to stolen Laptop */
/* v17.1.5 - v17.1.7: missing due to stolen Laptop */

    #define VS17_2022_U1_4      193131106     /* v17.1.4 */
    #define VS17_2022_U1_2_3    193131105     /* v17.1.[23] */
    #define VS17_2022_U1_0_1    193131104     /* v17.1.[01] */
    #define VS17_2022_U0_5_6    193030709     /* v17.0.[56] */
    #define VS17_2022_U0_2_4    193030706     /* v17.0.[2-4] */
    #define VS16_2019_U0_0_1    193030705     /* v16.0.[01] */
    #define VS17_2022_RTM       193030705     /* RTM      [2021-11-08] */

/*#elif _MSC_VER >= 1920*/  /* Visual C++ 2019 (or higher)? */
    #if _MSC_VER >= 1929
      #if _MSC_FULL_VER >= 192930133  /* Visual Studio 2019 Version 16.10 + 16.11 */
        #define VS16_2019_U11   _MSC_FULL_VER /* v16.11.x */
      #else
        #define VS16_2019_U10   _MSC_FULL_VER /* v16.10.x */
      #endif
    #elif _MSC_VER >= 1928
      #if _MSC_FULL_VER >= 192829910  /* Visual Studio 2019 Version 16.8 + 16.9 */
        #define VS16_2019_U9    _MSC_FULL_VER /* v16.9.x */
      #else
        #define VS16_2019_U8    _MSC_FULL_VER /* v16.8.x */
      #endif
    #elif _MSC_VER >= 1927
      #define VS16_2019_U7      _MSC_FULL_VER /* v16.7.x */
    #elif _MSC_VER >= 1926
      #define VS16_2019_U6      _MSC_FULL_VER /* v16.6.x */
    #elif _MSC_VER >= 1925
      #define VS16_2019_U5      _MSC_FULL_VER /* v16.5.x */
    #elif _MSC_VER >= 1924
      #define VS16_2019_U4      _MSC_FULL_VER /* v16.4.x */
    #elif _MSC_VER >= 1923
      #define VS16_2019_U3      _MSC_FULL_VER /* v16.3.x */
    #elif _MSC_VER >= 1922
      #define VS16_2019_U2      _MSC_FULL_VER /* v16.2.x */
    #elif _MSC_VER >= 1921
      #define VS16_2019_U1      _MSC_FULL_VER /* v16.1.x */
    #elif _MSC_VER >= 1920
      #define VS16_2019_RTM     _MSC_FULL_VER /* v16.0.x */
    #endif

    #define VS16_2019_U11_10    192930140     /* v16.11.10 */
    #define VS16_2019_U11_9     192930139     /* v16.11.9 */
    #define VS16_2019_U11_8     192930138     /* v16.11.8 */
    #define VS16_2019_U11_6_7   192930137     /* v16.11.[67] */
    #define VS16_2019_U11_4_5   192930136     /* v16.11.[45] */
    #define VS16_2019_U11_0_3   192930133     /* v16.11.[0123] */

    #define VS16_2019_U10_4     192930040     /* v16.10.4 */
    #define VS16_2019_U10_2_3   192930038     /* v16.10.[23] */
    #define VS16_2019_U10_0_1   192930037     /* v16.10.[01] */

    #define VS16_2019_U9_5_6    192829915     /* v16.9.[56] */
    #define VS16_2019_U9_4      192829914     /* v16.9.4 */
    #define VS16_2019_U9_2_3    192829913     /* v16.9.[23] */
    #define VS16_2019_U9_1      192829912     /* v16.9.1 */
    #define VS16_2019_U9_0      192829910     /* v16.9.0 */

    #define VS16_2019_U8_5_6    192829337     /* v16.8.[56] */
    #define VS16_2019_U8_4      192829336     /* v16.8.4 */
    #define VS16_2019_U8_3      192829335     /* v16.8.3 */
    #define VS16_2019_U8_2      192829334     /* v16.8.2 */
    #define VS16_2019_U8_0_1    192829333     /* v16.8.[01] */

    #define VS16_2019_U7_7      192729112     /* v16.7.[5-7] */
    #define VS16_2019_U7_1_4    192729111     /* v16.7.[1-4] */
    #define VS16_2019_U7_0      192729110     /* v16.7.0 */

    #define VS16_2019_U6_1_5    192628806     /* v16.6.[1-5] */
    #define VS16_2019_U6_0      192628805     /* v16.6.0 */

    #define VS16_2019_U5_4_5    192528614     /* v16.5.[45] */
    #define VS16_2019_U5_2_3    192528612     /* v16.5.[23] */
    #define VS16_2019_U5_1      192528611     /* v16.5.1 */
    #define VS16_2019_U5_0      192528610     /* v16.5.0 */

    #define VS16_2019_U4_4_6    192428319     /* v16.4.[6] */
    #define VS16_2019_U4_4_5    192428316     /* v16.4.[45] */
    #define VS16_2019_U4_3      192428315     /* v16.4.3 */
    #define VS16_2019_U4_0_2    192428314     /* v16.4.[012] */

    #define VS16_2019_U3_9_10   192328107     /* v16.3.[9-10] */
    #define VS16_2019_U3_3_8    192328106     /* v16.3.[3-8] */
    #define VS16_2019_U3_0      192328105     /* v16.3.[0-2] */

    #define VS16_2019_U2_0_5    192227905     /* v16.2.[0-5] */

    #define VS16_2019_U1_2_6    192127702     /* v16.1.[2-6] */
    #define VS16_2019_U1_0_1    /* ??? */     /* v16.1.[01] */

    #define VS16_2019_U0_0_3    192027508     /* v16.0.[0-3] */
    #define VS16_2019_RTM       192027508     /* RTM      [2019-04-02] */

    #if _MSC_VER >= 1926
      #define msvcattr_likely  201803L
    #endif

/*#elif _MSC_VER >= 1910*/  /* Visual C++ 2017 (or higher)? */
    #if _MSC_VER >= 1916
      #define VS15_2017_U9      _MSC_FULL_VER /* v15.9.x */
    #elif _MSC_VER >= 1915
      #define VS15_2017_U8      _MSC_FULL_VER /* v15.8.x */
    #elif _MSC_VER >= 1914
      #define VS15_2017_U7      _MSC_FULL_VER /* v15.7.x */
    #elif _MSC_VER >= 1913
      #define VS15_2017_U6      _MSC_FULL _VER /* v15.7.x */
    #elif _MSC_VER >= 1912
      #define VS15_2017_U5      _MSC_FULL_VER /* v15.5.x */
    #elif _MSC_VER >= 1911
      #define VS15_2017_U34     _MSC_FULL_VER /* v15.[34].x */
    #elif _MSC_VER >= 1910
      #define VS15_2017_U12     _MSC_FULL_VER /* v15.[012].x */
    #endif

    #define VS15_2017_U9_11     191627030     /* v15.9.11 */
    #define VS15_2017_U9_7_10   191627027     /* v15.9.([789]|10) */
    #define VS15_2017_U9_5_6    191627026     /* v15.9.[56] */
    #define VS15_2017_U9_4      191627025     /* v15.9.4 */
    #define VS15_2017_U9_2_3    191627024     /* v15.9.[23] */
    #define VS15_2017_U9_0_1    191627023     /* v15.9.[01] */

    #define VS15_2017_U8_8_9    191526732     /* v15.8.[89] */
    #define VS15_2017_U8_5_7    191526730     /* v15.8.[5-7] */
    #define VS15_2017_U8_4      191526729     /* v15.8.4 */
    #define VS15_2017_U8_0_3    191526726     /* v15.8.[0-3] */

    #define VS15_2017_U7_5_6    191426433     /* v15.7.[56] */
    #define VS15_2017_U7_4      191426431     /* v15.7.4 */
    #define VS15_2017_U7_3      191426430     /* v15.7.3 */
    #define VS15_2017_U7_2      191426429     /* v15.7.2 */
    #define VS15_2017_U7_0_1    191426428     /* v15.7.[01] */

    #define VS15_2017_U6_7      191326132     /* v15.6.7 */
    #define VS15_2017_U6_6      191326131     /* v15.6.6 */
    #define VS15_2017_U6_3_5    191326129     /* v15.6.[3-5] */
    #define VS15_2017_U6_0_2    191326128     /* v15.6.[0-2] */

    #define VS15_2017_U5_5_7    191225835     /* v15.5.[5-7] */
    #define VS15_2017_U5_3_4    191225834     /* v15.5.[34] */
    #define VS15_2017_U5_2      191225831     /* v15.5.2 */
    #define VS15_2017_U5_0_1    191225830     /* v15.5.[01] */

    #define VS15_2017_U4_5      191125547     /* v15.4.5 */
    #define VS15_2017_U4_4      191125542     /* v15.4.4 */
    #define VS15_2017_U3_3      191125507     /* v15.3.3 */
    #define VS15_2017_U3_0      VS15_2017_U3_3/* v15.3.0 – FIXME // assume to be the same for now! */

    /*https://www.reddit.com/r/cpp/comments/6tnt8t/visual_studio_2017_version_153_released#siteTable_t1_dlmkv7u
      STL: VS 2017 RTM (15.0), 15.1, and 15.2 all have the same C++ toolset,
            and therefore the same feature support. Only the IDE was improved. */
    #define VS15_2017_U0_2      191025017     /* v15.[012] */
    #define VS15_2017_RTM       191025017     /* RTM      [2017-03-08] */

    #if _MSC_VER >= 1910
      #define msvcattr_fallthrough  201603L
    #endif

/*#elif _MSC_VER >= 1900*/  /* Visual C++ 2015? */
    #define VS14_2015_U3      190024210       /* Update 3 [2016-07-27] */
    #define VS14_2015_U2      190023918       /* Update 2 [2016-03-30] */
    #define VS14_2015_U1      190023506       /* Update 1 [2015-11-30] */
    #define VS14_2015_RTM     190023026       /* RTM      [2015-07-20] */

    #if _MSC_VER >= 1911
      #define msvcattr_maybe_unused  201603L
    #elif _MSC_VER >= 1900
      #define msvcattr_carries_dependency  200809L
      #define msvcattr_deprecated  201309L
    #endif

/*#elif _MSC_VER >= 1800*/  /* Visual C++ 2013? */
    #define VS12_2013_U5      180040629       /* Update 5 [2015-07-20] */
    #define VS12_2013_U4      180031101       /* Update 4 [2014-11-06] */
    #define VS12_2013_U3      180030723       /* Update 3 [2014-08-04] */
    #define VS12_2013_U2      180030501       /* Update 2 [2014-05-12] */
    #define VS12_2013_U1      180021005       /* Update 1 [2014-02-20]: Microsoft apparently neglected to update _MSC_FULL_VER for update 1 - Doh! */
    #define VS12_2013_RTM     180021005       /* RTM      [2013-10-17] */

/*#elif _MSC_VER >= 1700*/  /* Visual C++ 2012? */
    #define VS11_2012_U4      170061030       /* Update 4 [2013-11-13] */
    #define VS11_2012_U3      170060610       /* Update 3 [2013-06-26] */
    #define VS11_2012_U2      170060315       /* Update 2 [2013-04-04] */
    #define VS11_2012_U1      170051106       /* Update 1 [2012-11-26] */
    #define VS11_2012_RTM     170050727       /* RTM      [2012-09-12] */

/*#elif _MSC_VER >= 1600*/  /* Visual C++ 2010? */
    #define VS10_2010_SP1     160040219       /* SP1      [2011-03-03] */
    #define VS10_2010_RTM     160030319       /* RTM      [2010-04-12] */

/*#elif _MSC_VER >= 1500*/  /* Visual C++ 2008? */
    #define VS9_2008_SP1      150030729       /* SP1      [2008-08-11] */
    #define VS9_2008_RTM      150021022       /* RTM      [2007-11-19] */

/*#elif _MSC_VER >= 1400*/  /* Visual C++ 2005? */
    #define VS8_2005_SP1      140050727       /* SP1      [2006-12-14] */
    #define VS8_2005_RTM      140050320       /* RTM      [2005-11-07] */

/*#elif _MSC_VER >= 1310*/  /* Visual C++ 2003? */
    #define VS71_2003_SP1      13106030       /* SP1      [2006-07-17] */
    #define VS71_2003_RTM      13103077       /* RTM      [2003-04-24] */

/*#elif _MSC_VER >= 1300*/  /* Visual C++ 2002? */
    #define VS7_2002_RTM       13009466       /* RTM      [2002-02-13] */

/*#elif _MSC_VER >= 1200*/  /* Visual C++ 6? */
    #define VS6_98             12008804       /* 1998-06-xx: MFC 6.0, Visual Studio 98 (6.0): SP1-SP5 */

  /*N.b.: My tests under Windows XP suggest that Visual C++ 6.0 RTM had no
          support for _MSC_FULL_VER – the compiler aborts with a build error.

          From SP4 onwards (and possibly even earlier), however, the Visual
          C++ compiler returned a value of 12008804 for _MSC_FULL_VER.

          Sadly, this value didn't change for Visual C++ SP5 (or even SP6),
          making this new preprocessor macro somewhat pointless in VS98. */
/*#endif*/

  /* “C” standard? */
  #ifndef __STDC_VERSION__
    #if _MSC_VER >= 1928
      /* C11 and C17 Standard Support Arriving in MSVC
         https://devblogs.microsoft.com/cppblog/c11-and-c17-standard-support-arriving-in-msvc/

         All the required features of C11 and C17 are supported:     What’s not:
         * _Pragma                                                   * Atomics and threading support
         * restrict                                                  * Support for Complex numbers
         * _Noreturn and <stdnoreturn.h>                             * aligned_alloc support is missing — the
         * _Alignas, _Alignof and <stdalign.h>                         alternative is to use _aligned_malloc
         * _Generic and <tgmath.h> support                           * Variable Length Arrays */
      #define STD__C_VERSION 201112L

    #elif _MSC_VER >= 1914
      /* Is there any option to switch between C99 and C11 C standards in Visual Studio?
         https://stackoverflow.com/questions/48981823/is-there-any-option-to-switch-between-c99-and-c11-c-standards-in-visual-studio#answer-48983723 */
      #define STD__C_VERSION 201112L          /* ... well, at least they had to implement the C11
                                                 library for full conformance with the C++17 standard! */
    #elif _MSC_VER >= 1900
      /* Visual Studio 2015 fully implements the C99 Standard Library
         https://msdn.microsoft.com/en-us/library/hh409293.aspx#c-runtime-library */
      #define STD__C_VERSION 199901L

    #elif _MSC_VER >= 1800
      /* Visual Studio 2013 shipped with some support for C99:
         _Bool type, Compound literals, Designated initializers,
         Mixing declarations with code, __func__ predefined macro

         https://blogs.msdn.microsoft.com/vcblog/2013/07/19/c99-library-support-in-visual-studio-2013/

         We know that this is not complete support for the C99 library functions.
         To the best of our understanding, the missing pieces are these:
         * The tgmath.h header is missing. C compiler support is needed for this
           header. Note that the ctgmath header was addedthis is possible because
           that header does not require the tgmath.h headeronly the ccomplex and
           cmath headers.
         * The uchar.h header is missing. This is from the C Unicode TR. Several
           format specifiers in the printf family are not yet supported.
         * The snprintf and snwprintf functions are missing from stdio.h and wchar.h */
      #define STD__C_VERSION 199901L

    #elif _MSC_VER >= 1100
      /* As it turns out, the "Microsoft Visual C++ 5.0.iso" already
         contained a "VCPP-5.00\DEVSTUDIO\VC\CRT\SRC\ISO646.H"! */
      #define STD__C_VERSION 199409L
    #endif

    #if defined(STD__C_VERSION) && !defined(__STDC_VERSION__)
      #define __STDC_VERSION__ 1
    #endif
  #endif

  /* “C++” standard? */
  #if defined(__cplusplus) /*&& __cplusplus == 1*/ /* Commented out, because VS2008 already sets 199711L */
    /* If the /std:c++XY compiler option is set, the _MSVC_LANG macro specifies the C++ language
       standard targeted by the compiler (from Visual Studio 2015 Update 3 onwards): */
    #if defined(_MSVC_LANG) /*&& _MSC_FULL_VER >= 190024210*/
      #define std__cplusplus _MSVC_LANG
    #else
      #if _MSC_VER >= 1914
        /* Visual Studio 2017 15.7 Brings Full C++17 Compliance
           https://blogs.msdn.microsoft.com/vcblog/2018/05/07/announcing-msvc-conforms-to-the-c-standard/ */
        #define std__cplusplus 201703L

      #elif _MSC_VER >= 1900  /* Visual C++ 2015? */
        /* Visual Studio 2015 shipped with “reasonable” support for C++14 */
        #define std__cplusplus 201402L

      #elif _MSC_VER >= 1800  /* Visual C++ 2013? */
        /* Visual Studio 2013 shipped with “reasonable” support for C++11
           https://blogs.msdn.microsoft.com/vcblog/2013/12/02/c1114-core-language-features-in-vs-2013-and-the-nov-2013-ctp/ */
        #define std__cplusplus 201103L

      #elif _MSC_VER >= 1400  /* Visual C++ 2005? */
        /* Value initialization and Non POD types
           https://stackoverflow.com/questions/3931312/value-initialization-and-non-pod-types#answer-3931589
           ... we set C++03 despite this issue (→ “All of the above issues have been fixed in VS 2015”) */
        #define std__cplusplus 200310L

      #elif _MSC_VER >= 1310 /* Visual C++ 2003? */
        /* Standard C++ Meets Managed C++
           http://www.gotw.ca/publications/standard_c++_meets_managed_c++.htm
           The final column shows that VC++ 6.0 only passed 83.43 percent of the test cases,
           but VC++ 7.1 passed 98.22 percent of the test cases.
           http://www.drdobbs.com/cpp/c-compilers-iso-conformance/184405483 */
        #define std__cplusplus 199711L
      #endif
    #endif
  #endif
#endif

/* ISO/IEC 9899 “C” standard conformance? */
#if defined(__STDC_VERSION__)
  #ifndef STD__C_VERSION  /* Necessary because of »macro name '__STDC_VERSION__' is reserved, '#define' ignored! */
  #define STD__C_VERSION __STDC_VERSION__
  #endif
  #if STD__C_VERSION >= 202311L       /* C23 */     /* ISO/IEC 9899:2023 */
    #define C_STD    2023
  #elif STD__C_VERSION >= 202000L     /* (__STDC_VERSION__ as reported by GCC for ‘-std=c2x’.) */
    #define C_STD    2020
  #elif STD__C_VERSION >= 201710L     /* C17/C18 */ /* ISO/IEC 9899:2018 */
    #define C_STD    2017
  #elif STD__C_VERSION >= 201112L     /* C11 */     /* ISO/IEC 9899:2011 */
    #define C_STD    2011
  #elif STD__C_VERSION >= 199901L     /* C99 */     /* ISO/IEC 9899:1999 */
    #define C_STD    1999
  #elif STD__C_VERSION >= 199409L     /* C94/C95 */ /* ISO/IEC 9899/AMD1:1995, Normative Addendum 1 (NA1) */
    #define C_STD    1995
  #endif
#else  /* (The standard macro __STDC_VERSION__ was introduced with NA1.) */
  #if defined(__GNUC__)               /* Seems to have supported C89 like “forever” */
    #define C_STD    1989
  #elif defined(__clang__)            /* Clang had its initial release on 2007-09-26 */
    #define C_STD    1989
  #elif defined(_MSC_VER)
    #if _MSC_VER >= 600               /* MSC5.1 stuck closely to the ANSI standard; MSC6 is just about on the money. */
    #define C_STD    1989             /* http://www.drdobbs.com/windows/optimizing-with-microsoft-c-60/184408398 */
    #else
    #define C_STD    1978             /* K&R C: “The C Programming Language”, 1st Edition (1978) */
    #endif
  #endif
#endif

/* ISO/IEC 14882 “C++” standard conformance? */
#if defined(__cplusplus)
  #ifndef std__cplusplus  /* Necessary because of »macro name '__cplusplus' is reserved, '#define' ignored«! */
  #define std__cplusplus __cplusplus
  #endif
  #if std__cplusplus >= 202305L       /* C++23 */   /* ISO/IEC 14882:2023 */
    #define C_STD    2023
  #elif std__cplusplus >= 202002L     /* C++20 */   /* ISO/IEC 14882:2020 */
    #define CPP_STD  2020
  #elif std__cplusplus >= 201703L     /* C++17 */   /* ISO/IEC 14882:2017 */
    #define CPP_STD  2017
  #elif std__cplusplus >= 201402L     /* C++14 */   /* ISO/IEC 14882:2014 */
    #define CPP_STD  2014
  #elif std__cplusplus >= 201103L     /* C++11 */   /* ISO/IEC 14882:2011 */
    #define CPP_STD  2011
  #elif std__cplusplus >= 200310L     /* C++03 */   /* ISO/IEC 14882:2003 */
    #define CPP_STD  2003
  #elif std__cplusplus >= 199711L     /* C++98 */   /* ISO/IEC 14882:1998 */
    #define CPP_STD  1998
  #else
    #define CPP_STD  1985             /* Inofficial standard: “The C++ Programming Language”, 1st Edition (1985) */
  #endif
#else
  #define CPP_STD 0                   /* No C++ support */
#endif

/* C++ compiler support – https://en.cppreference.com/w/cpp/compiler_support
                          https://github.com/MicrosoftDocs/cpp-docs/blob/main/docs/overview/visual-cpp-language-conformance.md
*/

/* https://en.cppreference.com/w/cpp/feature_test */
#if !defined(__has_cpp_attribute)
#define __has_cpp_attribute(attribute) compiler_prefix() ## attr_ ## attribute
#endif

#endif /* M_DETECT_COMPILER_H_ */
