import java.util.Locale;          // A Locale object represents a specific geographical, political, or cultural region
import java.util.Formatter;       // Interpreter for printf-style format strings

import java.awt.Color;            // Colors in the default sRGB color space


public class Mandelbrot
{ private Fractal
    fractal; //= null

  public static void main (String[] args)
  { Mandelbrot
      app = new Mandelbrot ();

    app.DrawMandelbrot ();
  }

  void DrawMandelbrot ()
  { // Graphics:
    Pad
      pad = new Pad ("Mandelbrot");
    int
      screen_width = pad.getAreaWidth (),
      screen_height = pad.getAreaHeight ();
    Fractal
      fractal;

    // Timing Code:
    int
      iter  = 0;                  // Anzahl durchlaufener Iterationen.
    long
//    ts_f  = 0,                  // Frequenz des HPET (High Performance Timer).
      ts_s  = 0,                  // Start des Benchmarks [nano].
      ts_c;                       // Aktuelle Zeitmarke [nano].
    long
      start = 0,                  // Start des Benchmarks [ms].
      last  = 0,                  // Letzte gemerkte Zeitmarke [ms].
      curr;                       // Aktuelle Zeitmarke [ms].

    // Initialer Fensterinhalt:
    fractal = new Fractal (pad);
    fractal.setView (1, true);
    fractal.fractalDraw (0, 0, screen_width-1, screen_height-1, true);
    pad.redraw ();

    // Benchmark:
    start = System.currentTimeMillis ();
    ts_s = System.nanoTime ();

    for (;;)
    {
      while (!fractal.fractalDraw (0, 0, screen_width-1, screen_height-1, false));
      iter++;

      curr = System.currentTimeMillis ();

      if ((curr-last) >= 1000)
      { double diff;

        ts_c = System.nanoTime ();
        diff = (ts_c - ts_s) / 1000000000.0;

        pad.setColor (Color.WHITE);
        pad.fillRect (0, 0, 100, 25);

        StringBuilder sb = new StringBuilder ();
        Formatter formatter = new Formatter (sb, Locale.GERMAN);
        formatter.format ("%1$2d:  %2$6.2f", iter, iter / diff);

        pad.setColor (Color.BLACK);
        pad.drawString (sb.toString(), 10, 20);
        pad.redraw ();

        last = curr;
      }
    }
  }

  public static void sleep (int ms)
  {
    try {
      Thread.sleep (ms);
    }
    catch (InterruptedException e) { }
  }
};
