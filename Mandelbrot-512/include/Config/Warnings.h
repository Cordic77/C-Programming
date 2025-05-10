#ifndef M_WARNINGS_H_
#define M_WARNINGS_H_

/* Disable Warnings: */
#if defined(__linux__) || defined(__MACH__)
  #if defined(__GNUC__)
    /* error: suggest braces around empty body in an 'if' statement */
    #pragma GCC diagnostic ignored "-Wempty-body"

    /* Missing braces around initializer */
    #pragma GCC diagnostic ignored "-Wmissing-braces"

    /* struct Structure s = {0};  // error: missing initializers */
    #pragma GCC diagnostic ignored "-Wmissing-field-initializers"

    /* ISO C90 does not support ‘long long’ */
    #pragma GCC diagnostic ignored "-Wlong-long"

    /* error: string length ‘%d’ is greater than the length ‘509’ ISO C90 compilers are required to support */
    #pragma GCC diagnostic ignored "-Woverlength-strings"

    /* ISO C90 does not support the ‘hh’ gnu_printf length modifier: */
    #pragma GCC diagnostic ignored "-Wformat"

    /* enumeration value '...' not handled in switch: */
    #pragma GCC diagnostic ignored "-Wswitch"
  #endif
#endif

#endif
