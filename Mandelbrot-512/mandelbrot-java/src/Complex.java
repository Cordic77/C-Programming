public class Complex
{
  public float real;        // Realteil
  public float imag;        // Imaginaerteil

  public Complex ()
  {
  }

  public Complex (float real, float imag)
  {
    this.real = real;
    this.imag = imag;
  }

  /*
  public double real ()
  {
    return (this.real);
  }

  public double imag ()
  {
    return (this.imag);
  }
  */

  public Complex AddComplex (Complex c2)
  {
    this.real += c2.real;
    this.imag += c2.imag;

    return (this);
  }

  public Complex MulComplex (Complex c2)
  { Complex
      result = new Complex ();

    this.real = this.real * c2.real - this.imag * c2.imag;
    this.imag = this.imag * c2.real + this.real * c2.imag;

    return (this);
  }

  public float AbsComplex ()
  {
    return ((float)Math.sqrt (this.real * this.real + this.imag * this.imag));
  }
};
