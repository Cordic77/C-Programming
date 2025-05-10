#ifndef _GRAPHICS_H_
#define _GRAPHICS_H_

#if defined(_WINDOWS)
  #define snprintf _snprintf
#endif

#define ABS(x) ((x)>=0? (x) : -(x))
#define SGN(x) ((x)<0? -1 : ((x)>0? 1 : 0))

/* Exports: */
extern int
  screen_width,
  screen_height,
  vmode_bpp,
  vmode_colors;
extern BOOL
  alt_palette,
  fullscreen;


/* Function declarations: */
extern int
  InitSDL (void);
extern int
  GetResolutionX (void),
  GetResolutionY (void);
extern void
  DumpPalette (void);
extern void
  BeginDraw (void);

/* Locked surface: */
extern void
  PutPixelLS (int x, int y, unsigned int color);
extern void
  DrawLineLS (int x1, int y1, int x2, int y2, unsigned int color);
extern void
  DrawFilledRectangleLS (int x1, int y1, int x2, int y2, unsigned int color);
/* Unlocked surface: */
extern void
  DrawFilledRectangleULS (int x1, int y1, int x2, int y2, unsigned int color);
extern void
  DrawStringULS (int x, int y, char *str);

extern void
  EndDraw (void);
extern void
  FlipScreen (void);

#endif
