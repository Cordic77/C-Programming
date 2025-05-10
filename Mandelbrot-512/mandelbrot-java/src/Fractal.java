import java.awt.Color;            // Colors in the default sRGB color space


public class Fractal
{ private static Complex[]
    leftUpper = new Complex [] {  // Linke obere Ecke - Minimal: Mandelbrot
        new Complex (0f, 0f),
        new Complex (-2.008f, +1.400f)
      },
    rightLower = new Complex [] { // Rechte untere Ecke - Maximal: Mandelbrot
        new Complex (0f, 0f),
        new Complex (1.657f, -1.400f)
      };
  private final static int
    MAX_ITERATIONS = 255;         // VGA, 256 Farben
  private final static boolean
    MANDEL_CARDIOID_PERIOD2BULB = false;  // Früherkennung ob Punkt in cardioid/period-2 bulb liegt?

  private Pad
    pad;                          // Prof. Buchberger: Grafikfenster.
  private Color
    pal [];                       // Aktuelle Farbpalette.
  private int
    fractview;                    // Aktuelle Fraktalansicht.
  private int
    screen_width,                 // Breite des Zeichenfensters.
    screen_height;                // Höhe des Zeichenfensters.
  private float
    real_factor,
    imag_factor;                  // Umrechnung auf Bildschirmaufloesung.

  public Fractal (Pad pad)
  {
    this.pad = pad;
    this.screen_width = pad.getAreaWidth ();
    this.screen_height = pad.getAreaHeight ();
    this.pal = VGA.pal256;
  }

  public void setView (int view, boolean forceReinit)
  {
    fractview = view;

    // View initialisieren?
    forceReinit |= (leftUpper[fractview].real==0.0 && leftUpper[fractview].imag==0)
                && (rightLower[fractview].real==0.0 && rightLower[fractview].real==0);

    if (forceReinit)
    {
      leftUpper[0].real = leftUpper[fractview].real;
      leftUpper[0].imag = leftUpper[fractview].imag;
      rightLower[0].real = rightLower[fractview].real;
      rightLower[0].imag = rightLower[fractview].imag;
    }

    // real_factor, imag_factor anpassen:
    {
      float real_dist = Math.abs (rightLower[0].real - leftUpper[0].real);
      float imag_dist = Math.abs (rightLower[0].imag - leftUpper[0].imag);
      float ratio = real_dist / imag_dist;
      float corr_dist;

      // y1 = y2 + (x2 - x1) * h / w = -1,2 + (1,0 - -2,0) * 600 / 800 = +1,05i
      if (ratio < (float)screen_width / screen_height)
      {
        corr_dist = imag_dist * screen_width / screen_height;
        corr_dist = (real_dist - corr_dist) / 2;
        leftUpper[0].real = leftUpper[0].real + corr_dist;
        rightLower[0].real = rightLower[0].real - corr_dist;
      }
      else
      {
        corr_dist = real_dist * screen_height / screen_width;
        corr_dist = (imag_dist - corr_dist) / 2;
        leftUpper[0].imag = leftUpper[0].imag - corr_dist;
        rightLower[0].imag = rightLower[0].imag + corr_dist;
      }

      real_factor = (rightLower[0].real - leftUpper[0].real) / (screen_width - 1);
      imag_factor = -(leftUpper[0].imag - rightLower[0].imag) / (screen_height - 1);
    }
  }

  private boolean mandelbrotCardoidPeriod2Bulb (float x, float y)
  {
    // Cardioid:
    float x_  = (x - 0.25f);
    float y_2 = y*y;
    float q   = x_*x_ + y_2;

    if (4.0f * q*(q + x_) < y_2)
      return (true);

    // Period-2 bulb:
    x_ += 1.25f;

    if (16.0f * (x_*x_ + y_2) < 1.0f)
      return (true);

    return (false);
  }

  // Z_i^2 + c     =  (x_i + y_i*i)^2 + x_0 + y_0  <=>
  // Re(Z_i^2 + c) =  x_i^2 - y_i^2 + x_0
  // Im(Z_i^2 + c) =  2*x_i*y_i + y_0
  // Abs(Z_i+1)    =  x_i+1^2 + y_i+1^2
  private Complex c = new Complex();

  private int mandelbrotPixelColorINL (float x, float y)
  { float
      xi, yi;
    float
      xi2, yi2;
    float
      radius;
    int
      i;

    // Z_0 = x + y*i:
    xi = c.real = x;
    yi = c.imag = y;

    // |Z_i|^2 > 4.0 => Folge Z_i ist nicht beschraenkt:
    for (i=MAX_ITERATIONS; i > 0; i--)
    {
      xi2 = xi * xi;
      yi2 = yi * yi;

      radius = xi2 + yi2;

      if (radius > 4.0)
        break;

      // Nur ~40% aller Punkte terminieren nicht in der 3. Iteration:
      if (MANDEL_CARDIOID_PERIOD2BULB)
      {
        if (i==MAX_ITERATIONS-2 && mandelbrotCardoidPeriod2Bulb(c.real, c.imag))
        {
          i = 0;
          break;
        }
      }

      yi = 2*xi*yi + c.imag;
      xi = xi2 - yi2 + c.real;
    }

    return (i);
  }

  public boolean fractalDraw (int l, int u, int r, int b, boolean drawScreen)
  { int
      y = Integer.MAX_VALUE,
      x = 0,
      w;
    float
      c_real,
      c_curr,
      c_imag;
    int
      color;

    c_imag = leftUpper[0].imag + u * imag_factor;
    c_real = leftUpper[0].real + l * real_factor;
    w      = r-l+1;

    for (; u <= b; u++)
    {
      c_curr = c_real;

      while (l <= r)
      {
        color = mandelbrotPixelColorINL (c_curr, c_imag);
        c_curr += real_factor;
        if (drawScreen)
        {
          pad.setColor (this.pal [color]);
          pad.drawDot (l, u);
        }
        l++;
      }

      c_imag += imag_factor;
      l -= w;
    }

    return (true);
  }
};
