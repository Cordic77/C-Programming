#ifdef _WIN32
  WINVERSION winver = GetVersionPacked ();

  #if (WDK_NTDDI_VERSION >= NTDDI_WIN10_RS4)
  /* Windows 1803+ (Windows 10.0.17134+): Universal C Runtime supports using a UTF-8 code page */
  #ifdef _INC_LOCALE  /* #include <locale.h> */
  if (winver.major_ver > 10 || (winver.major_ver == 10 && winver.build_num >= 17134))
  {
    errno = 0;
    if (!setlocale(LC_ALL, ".UTF8") && errno != 0)
    {
      #ifdef _CONSOLE
      fprintf (stderr, "Failed to set the program's code page to UTF-8, aborting: %d.\n", errno);
      #else
      MessageBox (NULL, TEXT("Failed to set the program's code page to UTF-8, aborting."),
                  TEXT("Program locale information"), MB_ICONERROR);
      #endif
      exit (255);
    }
  }
  #endif

  /* Windows 1903+ (Windows SDK 10.0.18362.0+): Force process to use UTF-8 as the process code page */
  if (winver.major_ver > 10 || (winver.major_ver == 10 && winver.build_num >= 18362))
  {
    if (!SetConsoleCP (CP_UTF8) || !SetConsoleOutputCP (CP_UTF8))
    {
      #ifdef _CONSOLE
      fprintf (stderr, "Failed to set the console code page to CP_UTF8, aborting.\n");
      exit (255);
      #else
      if (GetLastError() != ERROR_INVALID_HANDLE)
      {
        MessageBox (NULL, TEXT("Failed to set the console code page to CP_UTF8, aborting."),
                    TEXT("Setting process code page to UTF-8"), MB_ICONERROR);
        exit (255);
      }
      #endif
    }
  }
  else
  {
    #ifdef _CONSOLE
    fprintf (stderr, "UTF-8 process code page support requires Windows 1903+, aborting!\n");
    #else
    MessageBox (NULL, TEXT("UTF-8 process code page support requires Windows 1903+, aborting!"),
                TEXT("Setting process code page to UTF-8"), MB_ICONERROR);
    #endif
    exit(255);
  }
  #else
  (void)winver;
  #endif
#endif
