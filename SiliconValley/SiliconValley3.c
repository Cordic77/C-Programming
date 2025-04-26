#include <stdio.h>
#include <ctype.h>     /* toupper() */

/* Results! — The Big Array Size Survey for C. “C26” (?) will have a countof() operator: */
#if (!defined(__STDC_VERSION__) || __STDC_VERSION__ <= 202311L) && !defined(countof)
  #define countof(arr)  (sizeof(arr)/sizeof((arr)[0]))
#endif

__int128_t encode_message (char const str [], size_t *char_count);
void _ctx_iface (__int128_t s, int i);

int main (void)
{ __int128_t pentad_chars;
  char message [25+1];

  printf ("Enter up to 25 characters in the ASCII range 65…96:\n");

  if (fgets (message, countof(message), stdin) != NULL)
  { size_t valid_chars, i;

    /* Encode message: */
    pentad_chars = encode_message (message, &valid_chars);

    printf ("\nContains %zu valid char in the range 0 ≤ (character-'A') ≤ 31\n", valid_chars);
    printf ("With 5 bits/character, a total of %zu bits will be required to encode them\n\n", 5*valid_chars);

    printf ("... lower 64-bits: 0x%llX\n", (unsigned long long)pentad_chars);
    if (5*valid_chars >= 64)
      printf ("... upper 64-bits: 0x%llX\n", (unsigned long long)(pentad_chars >> 64));

    /* Decode message: */
    printf ("\nDecoded message:\n");
    for (i = 0; i < valid_chars; ++i)
      _ctx_iface (pentad_chars, (int)i);
    fputc ('\n', stdout);
  }

  return (0);
}

__int128_t encode_message (char const str [], size_t *char_count)
{ __int128_t encoded = 0;

  /* 25 chars á 5 bits… after that only 3 bits would remain, which is not
     enough space left to encode additional characters into a 128-bit integer: */
  { size_t valid_chars, i;
    int pentad_char;

    /* How many valid chars (in the range 0..31) are there? */
    for (i = valid_chars = 0; str[i] != '\0' && valid_chars < 25; ++i)
    {
      pentad_char = toupper(str [i]) - 'A';
      if (pentad_char >= 0 && pentad_char <= 31)
        valid_chars++;
    }

    /* Iterate over all valid characters, encode them using 5-bits each: */
    while (i-- > 0)
    {
      pentad_char = toupper(str [i]) - 'A';
      if ((unsigned)pentad_char < 32)  /* 0 ≤ pentad_char ≤ 31? */
        encoded = (encoded << 5) | pentad_char;
    }

    if (char_count)
     *char_count = valid_chars;
  }

  return (encoded);
}

void _ctx_iface (__int128_t s, int i)
{
/*int c = (((s & ((__int128_t)0x1FULL << i * 5)) >> i * 5 ) + 65);*/
  int c = 'A' + ((s >> (i * 5)) & 31);  /* A bit simpler… same effect */
  printf ("%c",c);
}
