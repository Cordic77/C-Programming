#include <stdio.h>                /* stderr */
#include <stdlib.h>               /* atexit(), exit() */
#include <math.h>                 /* log() */

#include "stdinc.h"

#if SDL_VER > 1                   /* Simple DirectMedia Layer */
  WARNING_PUSH()
  MSCWARN_DISABLE(4127)           /* _MSC: warning C4127: conditional expression is constant */
  #include <SDL2/SDL.h>
  WARNING_POP()
#else
  #include <SDL/SDL.h>
#endif

#if SDL_VER > 1
  /* As of version 2.0, SDL_Draw has been integrated into SDL itself! */
#else
  #if SDL_DRAW_SUPPORTED          /* Drawing primitives */
  #include <SDL/SDL_Draw.h>
  #endif
#endif

#if SDL_TTF_SUPPORTED
  #if SDL_VER > 1                 /* TrueType font support */
  #include <SDL2/SDL_ttf.h>
  #else
  #include <SDL/SDL_ttf.h>
  #endif
#endif

#include "Graphics.h"


/* Preprocessor macros (graphics defaults): */
#define WIDTH           1920      /*1920 // [Fallback: 1024] */
#define HEIGHT          1080      /*1080 // [Fallback:  768] */
#define COLORS          256


/* Exports: */
int
  screen_width  = -1,             /* Window width */
  screen_height = -1,             /* Window height */
  vmode_bpp     = -1,             /* Video mode bit depth */
  vmode_colors  = -1;             /* Video mode colors */
SDL_Color
  color_palette [256] = {{0,0,0,0}};/* Custom color palette */
BOOL
  alt_palette = FALSE,            /* Load alternate color palette? */
  fullscreen =                    /* Switch to fullscreen mode? */
    #if defined(_DEBUG)
    FALSE;
    #else
    TRUE;
    #endif

#if SDL_VER > 1 && defined(__linux__) && !defined(_DEBUG)
  #define RENDER_FLICKER_DISABLE_SDL2  1 /* SDL2 && __linux__ && ‘fullscreen’ → rely on Bresenham to draw lines ourselves. */
#else
  #define RENDER_FLICKER_DISABLE_SDL2  0 /* SDL1 || _WIN32 || _DEBUG build → SDL_RenderDrawLine() should work just fine. */
#endif

unsigned char *
  pixel_data = NULL;              /* (Array of) screen pixel color values */
#if SDL_TTF_SUPPORTED
TTF_Font *
  screen_font = NULL;             /* Loaded TrueType font */
#endif

/* Globals: */
static SDL_Surface *
  vmode_surface = NULL;           /* Drawing surface */
#if SDL_VER > 1
static SDL_Surface *
  screen_surface = NULL;          /* Video surface */
static SDL_Texture *
  screen_texture = NULL;          /* Video texture */
static SDL_Window *
  sdl_window = NULL;              /* Window */
static SDL_Renderer *
  sdl_renderer = NULL;            /* Renderer */
#endif

/* Predefined colors: */
static SDL_Color
  black = {0x00, 0x00, 0x00, 0xFF};  /* R=G=B=0 (opaque) */
static SDL_Color
  white = {0xFF, 0xFF, 0xFF, 0xFF};  /* R=G=B=255 (opaque) */


/* Function declarations: */
static int
  LoadFractalPalette (void);
static void
  FreeSDL (void);


/* Function definitions: */
extern int InitSDL (void)
{
  /* Initialize the SDL (“Simple DirectMedia Layer”): */
  if (SDL_Init (SDL_INIT_VIDEO) == -1)
  {
    SDL_SetError ("Can't init SDL: %s\n",SDL_GetError ());
    return (-1);
  }
  atexit (FreeSDL);

  /* [SDL1] Set video mode / [SDL2] Create window: */
  { int
      requested_width, requested_height;
    Uint32
      flags;

    requested_width = screen_width>0? screen_width : WIDTH;
    requested_height = screen_height>0? screen_height : HEIGHT;

    #if SDL_VER > 1
    flags = 0;
    #if !defined(_DEBUG)
    if (fullscreen)
      flags |= SDL_WINDOW_FULLSCREEN | SDL_WINDOW_BORDERLESS;
    #endif

    /* C.f. Benʼs Website – Intro to Software Rendering with SDL2,
            https://benedicthenshaw.com/soft_render_sdl2.html */
    if ((sdl_window=SDL_CreateWindow ("Fractal renderer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                                          requested_width, requested_height, flags)) == NULL)
    {
      /* (Very) simplistic fallback handling: */
      requested_width = 1024;
      requested_width =  768;

      if ((sdl_window = SDL_CreateWindow ("SDL sample", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                                        requested_width, requested_width, flags)) == NULL)
      {
        SDL_SetError ("Can't set video mode: %s\n", SDL_GetError ());
        return (-2);
      }
    }

    /* Set hints before creating the renderer! */
    if (SDL_SetHint (SDL_HINT_RENDER_SCALE_QUALITY, "0") == SDL_FALSE       /* Nearest pixel sampling ("1"=linear filtering, "2"=anisotropic filtering) */
     || SDL_SetHint (SDL_HINT_FRAMEBUFFER_ACCELERATION, "1") == SDL_FALSE)  /* Enable 3D acceleration, using the default renderer: */
    {
      SDL_SetError ("SDL_SetHint() failed: %s\n", SDL_GetError ());
      return (-3);
    }

    if ((sdl_renderer=SDL_CreateRenderer (sdl_window, -1, SDL_RENDERER_ACCELERATED /* | SDL_RENDERER_PRESENTVSYNC */)) == NULL)
    {
      SDL_SetError ("Unable to create renderer: %s\n", SDL_GetError ());
      return (-4);
    }

    SDL_SetRenderDrawBlendMode (sdl_renderer, SDL_BLENDMODE_NONE);
    #else
    flags = SDL_SWSURFACE | SDL_HWACCEL | SDL_HWPALETTE; /* | SDL_DOUBLEBUF */
    #if !defined(_DEBUG)
    if (fullscreen)
      flags |= SDL_FULLSCREEN;
    #endif

    vmode_colors = 256;  /* Video mode: indexed 8-bit (256) palette colors */
    vmode_bpp    = (int)(log(vmode_colors) / log(2.0));

    if ((vmode_surface=SDL_SetVideoMode (requested_width, requested_height, vmode_bpp, flags)) == NULL)
    {
      /* (Very) simplistic fallback handling: */
      requested_width = 1024;
      requested_width =  768;

      if ((vmode_surface=SDL_SetVideoMode (requested_width, requested_height, vmode_bpp, flags)) == NULL)
      {
        SDL_SetError ("Can't set video mode: %s\n", SDL_GetError ());
        return (-2);
      }
    }
    #endif

    screen_width = requested_width;
    screen_height = requested_height;
  }

  #if SDL_VER > 1
  /* Allocate an 8-bit color surface from the provided pixel data: */
  { Uint8 *
      vmode_data;
    SDL_PixelFormatEnum
      screen_format;
    Uint8
      screen_bpp;

    /* Video-mode surface we'll draw to: */
    screen_format = SDL_GetWindowPixelFormat (sdl_window);                      /* RGBA8888 */
    screen_bpp    = SDL_GetWindowSurface (sdl_window)->format->BitsPerPixel;    /* 32 */

    vmode_colors  = 256;  /* Video mode: indexed 8-bit (256) palette colors */  /* 256 */
    vmode_bpp     = (int)(log(vmode_colors) / log(2.0));                        /* 8 */

    vmode_data = (Uint8 *)calloc (screen_width * screen_height, sizeof(Uint8));

    if ((vmode_surface=SDL_CreateRGBSurfaceWithFormatFrom (vmode_data, screen_width, screen_height,
                                                           screen_bpp, screen_width, SDL_PIXELFORMAT_INDEX8)) == NULL)
    {
      SDL_SetError ("Unable to create 'screen' surface: %s\n", SDL_GetError ());
      return (-5);
    }

    /* Construct a surface thatʼs in a format close to the texture: */
    { SDL_Surface *
        rgba_surface;
      Uint32
        #if SDL_BYTEORDER == SDL_BIG_ENDIAN
        redMask   = 0xFF000000,
        greenMask = 0x00FF0000,
        blueMask  = 0x0000FF00,
        alphaMask = 0x000000FF;
        #else
        redMask   = 0x000000FF,
        greenMask = 0x0000FF00,
        blueMask  = 0x00FF0000,
        alphaMask = 0xFF000000;
        #endif

      if ((rgba_surface=SDL_CreateRGBSurface (SDL_SWSURFACE, screen_width, screen_height, screen_bpp,
                                              redMask, greenMask, blueMask, alphaMask)) == NULL)
      {
        SDL_SetError ("Unable to create 'screen' surface: %s\n", SDL_GetError ());
        return (-6);
      }

      if ((screen_surface=SDL_ConvertSurfaceFormat (rgba_surface, screen_format, 0)) == NULL)
      {
        SDL_SetError ("Cannot convert surface to 'screen' format: %s\n", SDL_GetError ());
        return (-7);
      }

      SDL_FreeSurface (rgba_surface);
    }

    /* Create a texture using the windows' pixel format; the SDL_TEXTUREACCESS_TARGET access
       pattern is required for 8-bit to 24-bit conv. – see also: Texture Streaming vs. Target:
       https://discourse.libsdl.org/t/texture-streaming-vs-target/20376/9 */
    if ((screen_texture=SDL_CreateTexture (sdl_renderer, screen_format, SDL_TEXTUREACCESS_STREAMING /* SDL_TEXTUREACCESS_TARGET */,
                                                         screen_width, screen_height)) == NULL)
    {
      SDL_SetError ("Unable to create 'screen' texture: %s\n", SDL_GetError ());
      return (-8);
    }

    SDL_SetTextureBlendMode (screen_texture, SDL_BLENDMODE_NONE);
  }

  if (vmode_bpp <= 8)
  {
    /* Load alternate color palette? */
    if (alt_palette)
    {
      LoadFractalPalette ();

      #if SDL_VER > 1
      /* See SDL_SetPaletteColors() call below! */
      #else
      if (SDL_SetPalette (vmode_surface, SDL_LOGPAL | SDL_PHYSPAL, color_palette, 0, 256) != 1)
      {
        SDL_SetError ("Unable to set palette: %s\n", SDL_GetError ());
        return (-9);
      }
      #endif
    }
    /* Match SDL 1.2's default palette with SDL_CreateRGBSurface()?
       https://stackoverflow.com/questions/43771325/match-sdl-1-2s-default-palette-with-sdl-creatergbsurface#answer-43771326 */
    else
    { Uint8 red   [8] = { 0, 36, 73, 109, 146, 182, 219, 255 };
      Uint8 green [8] = { 0, 36, 73, 109, 146, 182, 219, 255 };
      Uint8 blue  [4] = { 0, 85, 170, 255 };
      size_t index = 0, r, g, b;

      for (r = 0; r < 8; ++r)
        for (g = 0; g < 8; ++g)
          for (b = 0; b < 4; ++b)
          {
            color_palette [index].r = red [r];
            color_palette [index].g = green [g];
            color_palette [index].b = blue [b];
            color_palette [index].a = 0xFF;
            ++index;
          }
    }

    if (SDL_SetPaletteColors (vmode_surface->format->palette, color_palette, 0, 256) < 0)
    {
      SDL_SetError ("Unable to set palette: %s\n", SDL_GetError ());
      return (-10);
    }
  }
  #endif

  /* SDL_TTF library available? */
  #if SDL_TTF_SUPPORTED
  if (TTF_Init () < 0)
  {
    SDL_SetError ("TrueType initialization: %s\n", TTF_GetError());
    return (-11);
  }

  if ((screen_font=TTF_OpenFont ("arial.ttf", 16))==NULL)
  {
    SDL_SetError ("Missing font: %s\n", TTF_GetError());
    return (-12);
  }
  #endif

  /* Store pointer to pixel bitmap into ‘pixel_data’ variable: */
  pixel_data = (Uint8 *)vmode_surface->pixels;
  return (0);
}


extern int GetResolutionX (void)
{
  return (screen_width>0? screen_width : WIDTH);
}


extern int GetResolutionY (void)
{
  return (screen_height>0? screen_height : HEIGHT);
}


#if defined(_DEBUG)
extern void DumpPalette (void)
{ SDL_Palette
   *palette;
  SDL_Color
    entry;
  int
    i;

  palette = vmode_surface->format->palette;

  for (i=0; i < palette->ncolors; i++)
  {
    entry = palette->colors [i];
    fprintf (stdout, "  {%3d, %3d, %3d}%c  // [%3d]\n",
      entry.r, entry.g, entry.b,
      ((i < palette->ncolors-1)? ',' : ' '), i);
  }
}
#endif


static int LoadFractalPalette (void)
{ int
    i, offset;

  offset = 0;
  for (i=1; i < 32; ++i)
  {                                                                     /*  [0]  [1]  [2]  ...  [15] */
    color_palette [i + offset].r = (Uint8)( 7 + 8*(32 - abs(i - 32)) ); /*    0   32   32  ...   240 */
    color_palette [i + offset].g = 0;                                   /*    0    0    0  ...     0 */
    color_palette [i + offset].b = (Uint8)( 7 + 8*abs(i-32) );          /*    0  240  224  ...    32 */
  }

  offset = 32;
  for (i=0; i < 32; ++i)
  {                                                                     /*  [0]  [1]  [2]  ...  [15] */
    color_palette [i + offset].r = 0;                                   /*    0    0    0  ...     0 */
    color_palette [i + offset].g = (Uint8)( 7 + 8*(32 - abs(i-32)) );   /*    0   32   32  ...   240 */
    color_palette [i + offset].b = 0;                                   /*    0    0    0  ...     0 */
  }

  offset = 64;
  for (i=0; i < 32; ++i)
  {                                                                     /*  [0]  [1]  [2]  ...  [15] */
    color_palette [i + offset].r = 0;                                   /*    0    0    0  ...     0 */
    color_palette [i + offset].g = 0;                                   /*    0    0    0  ...     0 */
    color_palette [i + offset].b = (Uint8)( 7 + 8*(32 - abs(i-32)) );   /*    0   32   32  ...   240 */
  }

  offset = 96;
  for (i=0; i < 32; ++i)
  {                                                                     /*  [0]  [1]  [2]  ...  [15] */
    color_palette [i + offset].r = (Uint8)( 7 + 8*(32 - abs(i-32)) );   /*    0   32   32  ...   240 */
    color_palette [i + offset].g = (Uint8)( 7 + 8*(32 - abs(i-32)) );   /*    0   32   32  ...   240 */
    color_palette [i + offset].b = 0;                                   /*    0    0    0  ...     0 */
  }

  offset = 128;
  for (i=0; i < 32; ++i)
  {                                                                     /*  [0]  [1]  [2]  ...  [15] */
    color_palette [i + offset].r = 0;                                   /*    0    0    0  ...     0 */
    color_palette [i + offset].g = (Uint8)( 7 + 8*(32 - abs(i-32)) );   /*    0   32   32  ...   240 */
    color_palette [i + offset].b = (Uint8)( 7 + 8*(32 - abs(i-32)) );   /*    0   32   32  ...   240 */
  }

  offset = 160;
  for (i=0; i < 32; ++i)
  {                                                                     /*  [0]  [1]  [2]  ...  [15] */
    color_palette [i + offset].r = (Uint8)( 7 + 8*(32 - abs(i-32)) );   /*    0   32   32  ...   240 */
    color_palette [i + offset].g = 0;                                   /*    0    0    0  ...     0 */
    color_palette [i + offset].b = (Uint8)( 7 + 8*(32 - abs(i-32)) );   /*    0   32   32  ...   240 */
  }

  offset = 192;
  for (i=0; i < 32; ++i)
  {                                                                     /*  [0]  [1]  [2]  ...  [15] */
    color_palette [i + offset].r = (Uint8)( 7 + 8*(32 - abs(i-32)) );   /*    0   32   32  ...   240 */
    color_palette [i + offset].g = (Uint8)( 7 + 8*(32 - abs(i-32)) );   /*    0   32   32  ...   240 */
    color_palette [i + offset].b = (Uint8)( 7 + 8*(32 - abs(i-32)) );   /*    0   32   32  ...   240 */
  }

  offset = 224;
  for (i=0; i < 32; ++i)
  {                                                                     /*  [0]  [1]  [2]  ...  [7]  [8]  [9]  ...  [15] */
    color_palette [i + offset].r = (Uint8)( 15 + 15*(16 - abs(i-16)) ); /*    0   32   32  ...  112  128  112  ...    32 */
    color_palette [i + offset].g = (Uint8)( 15 + 15*(16 - abs(i-16)) ); /*    0   32   32  ...  112  128  112  ...    32 */
    color_palette [i + offset].b = (Uint8)( 15 + 15*(16 - abs(i-16)) ); /*    0   32   32  ...  112  128  112  ...    32 */
  }

  return (0);
}


extern void BeginDraw (void)
{
  /* Surfaces may reside in video memory, rather than in the directly accessible
     ‘regular’ memory, or in both (such as a RAM cached version). It depends on
     the video driver and card, and therefore one should always check SDL_MUSTLOCK! */
  if (SDL_MUSTLOCK (vmode_surface))
    if (SDL_LockSurface (vmode_surface) < 0)
    {
      fprintf (stderr, "BeginDraw(): %s\n", SDL_GetError ());
      exit (1);
    }
}


extern void ClearScreenULS (void)
{
  #if SDL_VER > 1
  SDL_RenderClear (sdl_renderer);
  #else
  DrawFilledRectangleULS (0, 0, screen_width-1, screen_width-1, *(unsigned int *)&black);
  #endif
}


extern /*INLINE*/ void PutPixelLS (int x, int y, unsigned int color)
{
  pixel_data [y*screen_width + x] = (unsigned char)color;
}


extern void DrawLineLS (int x1, int y1, int x2, int y2, unsigned int color)
{
  #if RENDER_FLICKER_DISABLE_SDL2 == 1
  if (not fullscreen)
  {
    /* SDL2 – RenderDrawLine()? */
    if (screen_bpp > 8)  /* 16-, 24 or 32-bit color depth? */
    {
      if (SDL_SetRenderDrawColor (sdl_renderer, color.comp.rgbRed, color.comp.rgbGreen,
                                                color.comp.rgbBlue, color.comp.rgbAlpha /*SDL_ALPHA_OPAQUE*/) < 0)
        fprintf (stderr, "SDL_SetRenderDrawColor() failed: %s\n", SDL_GetError ());
    }
    else
    {
      if (SDL_SetRenderDrawColor (sdl_renderer, color_palette [color.value].r, color_palette [color.value].b,
                                                color_palette [color.value].g, color_palette [color.value].a /*SDL_ALPHA_OPAQUE*/) < 0)
        fprintf (stderr, "SDL_SetRenderDrawColor() failed: %s\n", SDL_GetError ());
    }

    if (SDL_RenderDrawLine (sdl_renderer, x1, y1, x2, y2) < 0)
      fprintf (stderr, "SDL_RenderDrawLine() failed: %s\n", SDL_GetError ());
  }
  else
  #endif
  {
    /* SDL1 – SDL_draw: Draw_Line()? */
    #if SDL_DRAW_SUPPORTED
    Draw_Line (vmode_surface, (Sint16)x1, (Sint16)y1, (Sint16)x2, (Sint16)y2, color);

    /* SW sol. – Bresenhamʼs line algorithm, http://fredericgoset.ovh/mathematiques/courbes/en/bresenham_line.html */
    #else
    int dx = x2 - x1;
    int dy = y2 - y1;
    int incX = SGN(dx);
    int incY = SGN(dy);
    dx = ABS(dx);
    dy = ABS(dy);

    if (dy == 0)
    { int x;

      /* Horizontal line: */
      for (x = x1; x != x2 + incX; x += incX)
        PutPixelLS (x, y1, color);
    }
    else if (dx == 0)
    { int y;

      /* Vertical line: */
      for (y = y1; y != y2 + incY; y += incY)
        PutPixelLS (x1, y, color);
    }
    else if (dx >= dy)
    {
      /* More horizontal than vertical: */
      int slope = 2 * dy;
      int error = -dx;
      int errorInc = -2 * dx;
      int y = y1;
      int x;

      for (x = x1; x != x2 + incX; x += incX)
      {
        PutPixelLS (x, y, color);
        error += slope;

        if (error >= 0)
        {
          y += incY;
          error += errorInc;
        }
      }
    }
    else
    {
      /* More vertical than horizontal: */
      int slope = 2 * dx;
      int error = -dy;
      int errorInc = -2 * dy;
      int x = x1;
      int y;

      for (y = y1; y != y2 + incY; y += incY)
      {
        PutPixelLS (x, y, color);
        error += slope;

        if (error >= 0)
        {
          x += incX;
          error += errorInc;
        }
      }
    }
    #endif
  }
}


extern void DrawFilledRectangleLS (int x1, int y1, int x2, int y2, unsigned int color)
{ int x, y;

  for (y=y1; y <= y2; y++)
  {
    for (x=x1; x <= x2; x++)
    {
      PutPixelLS (x, y, color);
    }
  }

  return;
}


extern void DrawFilledRectangleULS (int x1, int y1, int x2, int y2, unsigned int color)
{ SDL_Rect
    rect;

  rect.x = (Sint16)x1;
  rect.y = (Sint16)y1;
  rect.w = (Uint16)(abs (x2 - x1) + 1);
  rect.h = (Uint16)(abs (y2 - y1) + 1);

  SDL_FillRect (vmode_surface, &rect, color);
}


extern void DrawStringULS (int x, int y, char *str)
{
  #if SDL_TTF_SUPPORTED
  SDL_Surface
   *text = NULL;
  SDL_Rect
    dest;

  /* Render UTF-8 text at high quality to a new ARGB surface: */
  text = TTF_RenderUTF8_Shaded (screen_font, str, white, black);

  if (text==NULL)
  {
    fprintf (stderr, "TTF_Render(): %s\n", TTF_GetError ());
    exit (2);
  }

  dest.x = (Sint16)x;
  dest.y = (Sint16)y;
  dest.w = (Uint16)text->w;
  dest.h = (Uint16)text->h;

  /* Bit-blit text surface unto another (higher bit-depth) surface? */
  #if SDL_VER > 1 && !SDL_SW_RENDERER
  if (screen_bpp != 8)
  {
    /* C.f. SDL2_ttf – Rendering Fonts and Texts: https://www.freepascal-meets-sdl.net/chapter-7-texts-fonts-surface-conversion/ */
    SDL_Texture *texture = SDL_CreateTextureFromSurface (sdl_renderer, text);

    if (SDL_RenderCopy (sdl_renderer, texture, NULL, &dest) < 0)
    {
      fprintf (stderr, "TTF:RenderCopy(): %s\n", TTF_GetError ());
      exit (254);
    }

    /* Free texture and surface: */
    SDL_DestroyTexture (texture);
    SDL_FreeSurface (text);
  }
  else
  #endif
  {
    /* BitBlt to window surface: */
    if (SDL_BlitSurface (text, NULL, vmode_surface, &dest) < 0)
    {
      fprintf (stderr, "TTF:BlitSurface(): %s\n", TTF_GetError ());
      exit (254);
    }

    /* Free text surface: */
    SDL_FreeSurface (text);
  }
  #else
  x = 0; y = 0;
  fprintf (stdout, "%s\n", str);
  fflush (stdout);
  #endif
}


#if SDL_VER == 1
static SDL_Surface *SDL_SurfaceRect (SDL_Surface *image, SDL_Rect *imgrect)
{ SDL_Surface *
    part_surface;
  SDL_PixelFormat
   *pixel_format = NULL;
  SDL_Rect
    partrect;

  pixel_format = vmode_surface->format;

  part_surface = SDL_CreateRGBSurface (SDL_SRCALPHA | SDL_SRCCOLORKEY, /* | SDL_HWSURFACE */
    imgrect->w, imgrect->h,
    pixel_format->BitsPerPixel,
    pixel_format->Rmask,
    pixel_format->Gmask,
    pixel_format->Bmask,
    pixel_format->Amask);

  /* Copy image data: */
  partrect.x = 0;
  partrect.y = 0;
  partrect.w = imgrect->w;
  partrect.h = imgrect->h;

  if (SDL_BlitSurface (image, imgrect, part_surface, &partrect) < 0)
  {
    SDL_FreeSurface (part_surface);
    part_surface = NULL;
  }

  return (part_surface);
}
#endif


#ifdef _SDL_rotozoom_h  /* SDL_gfx included? */
extern void DrawSurfaceULS (SDL_Surface *image, SDL_Rect *imgrect, BOOL zoom_to_screen)
{
  SDL_Rect screen_rect = {0};
  double zoom = 1.0;

  if (zoom_to_screen)
  {
    screen_rect.w = imgrect? imgrect->w : (Uint16)image->w;
    screen_rect.h = imgrect? imgrect->h : (Uint16)image->h;

    zoom = screen_width / (double)screen_rect.w;

    { double
        factor = screen_height / (double)screen_rect.h;

      if (factor < zoom)
        zoom = factor;

      if (zoom != 1.0)
      {
        screen_rect.w = (Uint16)(screen_rect.w * factor);
        screen_rect.h = (Uint16)(screen_rect.h * factor);
      }
    }
  }

  if (zoom_to_screen)
  {
    #if SDL_VER > 1
    SDL_BlitScaled (image, imgrect, vmode_surface, NULL);
    #else
    /* With SDL 1.x we need to rely on SDL_gfx to do this: */
    SDL_Surface *
      surface_rect;
    SDL_Surface *
      zoomed_surface;

    BOOL copy_rect = imgrect != NULL && not (imgrect->x == 0 && imgrect->y == 0
                                          && imgrect->w == image->w
                                          && imgrect->h == image->h);

    surface_rect = copy_rect? image : SDL_SurfaceRect(image, imgrect);
    zoomed_surface = zoomSurface (surface_rect, zoom, zoom, SMOOTHING_OFF);

    SDL_BlitSurface (zoomed_surface, NULL, vmode_surface, NULL);

    SDL_FreeSurface (zoomed_surface);
    if (copy_rect)
      SDL_FreeSurface (surface_rect);
    #endif
  }
  else
    SDL_BlitSurface (image, imgrect, vmode_surface, &screen_rect);
}
#endif


extern void EndDraw (void)
{
  if (SDL_MUSTLOCK (vmode_surface))
    SDL_UnlockSurface (vmode_surface);
}

extern void FlipScreen (void)
{
  #if SDL_VER > 1
  char const *
    operation;

  /* Rendering 8-bit palettized surfaces in SDL 2.0 applications, Sander van der Burg:
     https://sandervanderburg.blogspot.com/2014/05/rendering-8-bit-palettized-surfaces-in.html */
  { void *
      pixels;
    int
      pitch;

    /* Blit 8-bit palette surface onto the window surface thatʼs closer to the textureʼs format: */
    operation = "SDL_BlitSurface(vmode_surface)";
    if (SDL_BlitSurface (vmode_surface, NULL, screen_surface, NULL) < 0)
      goto OnError;

    /* Modify the textureʼs pixels */
    operation = "SDL_ConvertPixels(screen_texture)";
    if (SDL_LockTexture (screen_texture, NULL, &pixels, &pitch) < 0)
      goto OnError;
    if (SDL_ConvertPixels (screen_surface->w, screen_surface->h, screen_surface->format->format,
                           screen_surface->pixels, screen_surface->pitch, SDL_GetWindowPixelFormat(sdl_window),
                           pixels, pitch) < 0)
      goto OnError;
    SDL_UnlockTexture (screen_texture);

    /* Make the modified texture visible by rendering it: */
    operation = "SDL_RenderCopy(screen_texture)";
    if (SDL_RenderCopy (sdl_renderer, screen_texture, NULL, NULL) < 0)
      goto OnError;
  }

  /* Show updated window (up until now everything was drawn behind the scenes): */
  SDL_RenderPresent (sdl_renderer);

  #if defined(_DEBUG) && defined(WRITE_RAW_SCREENSHOT)
  { FILE *screenshot_raw;
    screenshot_raw = fopen ("Screenshot.raw", "wb");
    fwrite (vmode_surface->pixels, 1, screen_width*screen_height, screenshot_raw);
    fclose (screenshot_raw);
  }
  #endif
  return;

OnError:;
  fprintf (stderr, "%s: %s\n", operation, SDL_GetError ());
  exit (3);
  #else
  /* Flip screen to current frame: */
  SDL_Flip (vmode_surface);
  pixel_data = (Uint8 *)vmode_surface->pixels;
  #endif
}


static void FreeSDL (void)
{
#if SDL_VER > 1
  SDL_FreeSurface (vmode_surface);
  SDL_FreeSurface (screen_surface);
/*free (screen_surface.pixels);*/  /* Unnecessary, SDL_FreeSurface() does this for us! */
  SDL_DestroyRenderer (sdl_renderer);
  SDL_DestroyWindow (sdl_window);
#endif
#if SDL_TTF_SUPPORTED
  TTF_CloseFont (screen_font);
  TTF_Quit ();
#endif
  SDL_Quit ();
}
