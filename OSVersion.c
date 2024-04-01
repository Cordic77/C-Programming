#if defined(_WIN32)
/*
  How to get the OS version in Win8.1 as GetVersion/GetVersionEx are deprecated?
  https://stackoverflow.com/questions/32115255/c-how-to-detect-windows-10#answer-39173324

  Starting in Windows 8.1, GetVersion[Ex]() are subject to application manifestation:

    ———————————————————————————————————————————————————————————————————————————
    With the release of Windows 8.1, the behavior of the GetVersionEx() API has
    changed in the value it will return for the operating system version. The
    value returned by the GetVersionEx() function now depends on how the
    application is manifested.

    Applications not manifested for Windows 8.1 or Windows 10 will return the
    Windows 8 OS version value (6.2). Once an application is manifested for a
    given operating system vers., GetVersionEx() will always return the version
    that the application is manifested for in future releases. To manifest your
    appl. for Windows 8.1+ refer to Targeting your application for Windows.
    ———————————————————————————————————————————————————————————————————————————

  The newer Version Helper functions are simply wrappers for VerifyVersionInfo().
  Starting in Windows 10, it is now subject to  manifestation as well:

    Windows 10: VerifyVersionInfo() returns false when called by applications
    that do not have a compatibility manifest for Windows 8.1 or Windows 10 if
    the ‘lpVersionInfo’ parameter is set so that it specifies Windows 8.1 or
    Windows 10, even when the current operating system version is Windows 8.1
    or Windows 10. Specifically, VerifyVersionInfo() has the following behavior:

    ———————————————————————————————————————————————————————————————————————————
    • If the application has no manifest, VerifyVersionInfo behaves as if the
      operation system version is Windows 8 (6.2).

    • If the application has a manifest that contains the GUID that corresponds
      to Windows 8.1, VerifyVersionInfo behaves as if the operation system ver=
      sion is Windows 8.1 (6.3).

    • If the application has a manifest that contains the GUID that corresponds
      to Windows 10, VerifyVersionInfo behaves as if the operation system version
      is Windows 10 (10.0).

    The Version Helper functions use the VerifyVersionInfo() function, so the
    behavior IsWindows8Point1OrGreater() and IsWindows10OrGreater() are similarly
    affected by the presence and content of the manifest.
    ———————————————————————————————————————————————————————————————————————————

  To get the true OS version regardless of manifestation use RtlGetVersion(),
  NetServerGetInfo(), or NetWkstaGetInfo() instead. They all report an accurate
  OS version and are not subject to manifestation (yet?).
*/

/* Windows headers: */
#define WIN32_LEAN_AND_MEAN       /* Exclude APIs such as Cryptography, DDE, RPC, Shell, and Windows Sockets */
#define NOSERVICE                 /* Additional NOservice definitions: NOMCX, NOIME, NOSOUND, NOCOMM, NOKANJI, NORPC, ... */
#define _WINSOCKAPI_              /* Prevent windows.h from including winsock.h */
#define _WIN32_DCOM               /* Include all DCOM (Distributed Component Object Model) centric definitions */
#include <windows.h>              /* ... pull in Windows headers! */

/*#include <ntdef.h>*/
#ifndef _NTDEF_
typedef __success(return >= 0) LONG NTSTATUS;
#endif

/*#include <ntstatus.h>*/
#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS ((NTSTATUS)0x00000000L)
#endif


/* Pragma: “structure padding” */
#ifndef PACK
  /* warning C4201: nonstandard extension used : nameless struct/union */
  #define PACK(DECLARATION)             \
    __pragma(pack (push, 1))            \
    __pragma(warning (push))            \
    __pragma(warning (disable : 4201))  \
    DECLARATION;                        \
    __pragma(warning (pop))             \
    __pragma(pack (pop))
#endif


/* C.f. https://docs.microsoft.com/en-us/windows/win32/api/sysinfoapi/nf-sysinfoapi-getversion#code-try-1 */
#ifndef WINVERSION_DEFINED
  #pragma warning (push)
  #pragma warning (disable: 4201)  /* _MSC: nonstandard extension used: nameless struct/union */
  PACK(union WINVERSION {  /* (Assumes "Little Endian".) */
    LONG    version_number;
    struct {
      BYTE  major_ver;
      BYTE  minor_ver;
      WORD  build_num;
    };
  })
  #pragma warning (pop)
  #define WINVERSION_DEFINED
#endif
#ifdef WINVERSION_DEFINED
typedef union WINVERSION WINVERSION;
#endif

/* Unions and type-punning?
   https://stackoverflow.com/questions/25664848/unions-and-type-punning#answer-25672839

   Type-punning through unions is perfectly fine in C (but not in C++):

   C11 section 6.5.2.3 §3:

     A postfix expression followed by the . operator and an identifier designates
     a member of a structure or union object. The value is that of the named member

   with the following footnote 95:

     If the member used to read the contents of a union object is not the same as
     the member last used to store a value in the object, the appropriate part of
     the object representation of the value is reinterpreted as an object repre=
     sentation in the new type as described in 6.2.6 (a process sometimes called
     “type punning”). This might be a trap representation.
*/
/* #define WINVERSION_TYPE_PUNNING 1 */
/* → Prefer MAKEWORD()/MAKELONG() instead! */


/* Function declarations: */
#ifdef  __cplusplus
extern "C" {
#endif

static WINVERSION GetVersionPacked (void);
static LONG GetVersionLong (void);

#ifdef  __cplusplus
}
#endif


/* Need to get real OS Version:
   https://docs.microsoft.com/en-us/answers/questions/595325/need-to-get-real-os-version.html#answer-595392 */
typedef NTSTATUS (WINAPI *PFN_RTLGETVERSIONEX) (LPOSVERSIONINFOEX lpVersionInformation);

static WINVERSION GetVersionPacked (void)
{ WINVERSION
    winver;
  winver.version_number = -1;

  { PFN_RTLGETVERSIONEX
      RtlGetVersionPtr;

    HMODULE hNtdll = LoadLibrary(TEXT("ntdll.dll"));

    if (!hNtdll)
      goto LoadLibraryFailed;

    RtlGetVersionPtr = (PFN_RTLGETVERSIONEX)GetProcAddress (hNtdll, "RtlGetVersion");

    if (!RtlGetVersionPtr)
      goto GetProcFailed;

    { OSVERSIONINFOEX
        os_info;

      os_info.dwOSVersionInfoSize = sizeof(os_info);
      if ((*RtlGetVersionPtr)(&os_info) != STATUS_SUCCESS)
        goto GetVersionFailed;

      #if WINVERSION_TYPE_PUNNING
      winver.major_ver      = (BYTE)os_info.dwMajorVersion;
      winver.minor_ver      = (BYTE)os_info.dwMinorVersion;
      winver.build_num      = (WORD)os_info.dwBuildNumber;
      #else
      winver.version_number = MAKELONG(MAKEWORD(os_info.dwMajorVersion, os_info.dwMinorVersion), os_info.dwBuildNumber);
      #endif
    }

GetVersionFailed:
GetProcFailed:
    FreeLibrary(hNtdll);
  }

LoadLibraryFailed:
  return (winver);
}


static LONG GetVersionLong (void)
{
  WINVERSION winver = GetVersionPacked ();

  return (winver.version_number);
}
#endif
