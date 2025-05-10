import java.awt.*;
import java.awt.image.*;
import java.awt.Graphics2D;
import java.awt.event.*;
import java.awt.geom.*;
import javax.swing.*;


/***********************************************************************
 * Die Klasse Pad stellt ein paar einfache Methoden zur
 * Unterst&uuml;tzung der graphischen Ausgabe zur Verf&uuml;gung.
 *  @author P. Pepper, Adaptiert Michael Buchberger
 *  @version 1.2
 ***********************************************************************/
class PadFrame extends JFrame {
  PadFrame (String title)
  {
    super(title);
    setResizable(false);
  }
}

public class Pad  {
  private static int delay = 100;   // in milliseconds
  private static int factor = 0;
  private static int point = 2;         // Punktgroesse

  private PadArea padArea;
  private PadFrame frame;

  /** Der fette Schrifttyp */
  public static final int BOLD   = Font.BOLD;
  /** Der kursive Schrifttyp */
  public static final int ITALIC = Font.ITALIC;
  /** Der normale Schrifttyp */
  public static final int PLAIN  = Font.PLAIN;

  /** Name des Schrifttyps ohne Serifen */
  public static final String SANSSERIF = "SansSerif";
  /** Name des Schrifttyps mit Serifen */
  public static final String SERIF     = "Serif";
  /** Name des Schrifttyps mit konstanter Zeichenbreite */
  public static final String FIXED     = "Monospaced";
  /** Name des DIALOG-Fonts */
  public static final String DIALOG    = "Dialog";

  /** Schwarze Farbe */
  static final public Color black   =  Color.black;
  /** Blaue Farbe */
  static final public Color blue    =  Color.blue;
  /** Gr&uuml;ne Farbe */
  static final public Color green   =  Color.green;
  /** Violette Farbe */
  static final public Color magenta =  Color.magenta;
  /** Rote Farbe */
  static final public Color red     =  Color.red;
  /** Weisse Farbe */
  static final public Color white   =  Color.white;
  /** Gelbe Farbe */
  static final public Color yellow  =  Color.yellow;
  /** Hellblaue Farbe */
  static final public Color lightBlue  =  new Color(0.0f,0.72f,0.9f);
  /** Mittelblaue Farbe */
  static final public Color mediumBlue =  new Color(0.0f,0.63f,0.9f);
  /** Hellgelbe Farbe */
  static final public Color lightYellow =  new Color(0.9f,0.9f,0.2f);

  /** Normaler Strich */
  static final public BasicStroke stroke =
    new BasicStroke(2.0f,BasicStroke.CAP_ROUND, BasicStroke.JOIN_ROUND );

  /** Dicker Strich */
  static final public BasicStroke wideStroke =
      new BasicStroke(8.0f, BasicStroke.CAP_ROUND,BasicStroke.JOIN_ROUND);

  /** Unterbrochener Strich */
  static final public BasicStroke dashed =
    new BasicStroke(1.0f, BasicStroke.CAP_BUTT, BasicStroke.JOIN_MITER,
        10.0f, new float[] {10.0f}, 0.0f);


/***********************************************************************
 * Konstruktor: Erzeugt ein Objekt der Klasse Pad.
 ***********************************************************************/
  public Pad (String title) {
    try { // Look-and-feel setzen
        UIManager.setLookAndFeel(UIManager.getCrossPlatformLookAndFeelClassName());
    } catch (Exception e) { }

    frame = new PadFrame(title);  // Hauptfenster
    padArea = new PadArea();     // Zeichenfeld innerhalb des Hauptfensters
    JButton button = new JButton("EXIT"); // Exit-Button
    button.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
          System.exit(0);
      }//actionPerformed
        });//addActionListener

    JPanel pane = new JPanel();
    pane.setLayout(new BorderLayout() );
    pane.add( BorderLayout.SOUTH, button);
    pane.add( BorderLayout.CENTER, padArea );
    frame.setContentPane( pane );

    frame.pack();       // Groessen der Teilfenster berechnen
    frame.setVisible( false );    // (noch) nicht am Bildschirm zeigen
    frame.addWindowListener(new WindowAdapter() {
      public void windowClosing(WindowEvent e) {
          System.exit(0) ;
      }
        });//addWindowListener
    frame.setMinimumSize(frame.getPreferredSize());
    frame.setMaximumSize(frame.getPreferredSize());
    setBackground(Pad.black);
    setVisible(true);
  }// Pad


  public Pad ()
  {
    this("");
  }

  public void switchBuffer()
  {
    padArea.switchBuffer();
  }

  public void redraw()
  {
    padArea.redraw();
  }

/***********************************************************************
 * Titelzeile des Fensters festlegen
 *  @param t Text der Titelzeile
 ***********************************************************************/
  public void setTitle( String t ) { frame.setTitle( t ); }

/***********************************************************************
 * Fenster auf dem Bildschirm positionieren
 *  @param x x-Koordinate der linken oberen Ecke (in Pixel)
 *  @param y y-Koordinate der linken oberen Ecke (in Pixel)
 ***********************************************************************/
  public void setLocation( int x, int y ) { frame.setLocation( x, y ); }

/***********************************************************************
 * Hintergrundfarbe des Fensters festlegen
 *  @param bg die Farbe
 ***********************************************************************/
  public void setBackground( Color bg ) { frame.setBackground( bg ); }

/***********************************************************************
 * L&auml;&szlig;t das Fenster auf dem Bildschirm erscheinen
 *  @param vis true=zeigen, false = verbergen
 ***********************************************************************/
  public void setVisible( boolean vis ) {
    frame.setVisible(vis) ;
    frame.repaint();
    padArea.initialize();
  }

/***********************************************************************
 * Setzt Breite und H&ouml;he des Fensters.
 *  @param w  gew&uuml;nschte Breite (in Pixel).
 *  @param h  gew&uuml;nschte H&ouml;he (in Pixel)
 ***********************************************************************/
  public void setPadSize( int w, int h ) {
    frame.setSize( w, h );
    padArea.setAreaSize( w, h );
    frame.pack();
  }
/***********************************************************************
 * Liefert die Breite der Zeichenfl&auml;che.
 * @return  Breite (in Pixel)
 ***********************************************************************/
  public int getAreaWidth() { return padArea.getSize().width; }

/***********************************************************************
 * Liefert die H&ouml;he der Zeichenfl&auml;che.
 * @return  H&ouml;he (in Pixel)
 ***********************************************************************/
  public int getAreaHeight() { return padArea.getSize().height; }

/***********************************************************************
 * Setzt Vordergrundfarbe des Fensters.
 *  @param col  gew&uuml;nschte Farbe.
 ***********************************************************************/
  public void setColor ( Color col ) { padArea.setColor(col); }

  public void setColor (int r, int g, int b) { setColor (new Color (r%256,g%256,b%256)); }

/***********************************************************************
 * Setzt den Font
 *  @param name  Name des Fonts.
 *  @param style Stil des Fonts.
 *  @param size  Gr&ouml;sse des Fonts.
 ***********************************************************************/
  public void setFont ( String name, int style, int size) {
    padArea.setFont( new Font(name,style,size) );
  }

/***********************************************************************
 * Strichst&auml;rke festlegen
 *  @param str die Strichst&auml;rke
 ***********************************************************************/
  public void setStroke( BasicStroke str ) { padArea.setStroke( str ); }

/***********************************************************************
 * Ermittelt die L&auml;nge einer Zeichenkette.
 *  @param text  Zeichenkette
 *  @return  L&auml;nge der Zeichenkette
 ***********************************************************************/
  public int stringWidth (String text) {
    return padArea.getStringWidth( text );
  }
/***********************************************************************
 * Ermittelt die Breite eines Zeichens.
 *  @param symbol  Zeichen.
 *  @return  Breite des Zeichens.
 ***********************************************************************/
  public int getCharWidth (char symbol) {
    return padArea.getCharWidth( symbol );
  }

/***********************************************************************
 * Ermittelt die H&ouml;he des Fensters.
 *  @return  H&ouml;he des Fensters.
 ***********************************************************************/
  public int getHeight () {
    return padArea.getLineHeight();
  }

/***********************************************************************
 * Ermittelt die H&ouml;he des typischen Gro&szlig;buchstaben.
 * @return  Ascent.
 ***********************************************************************/
  public int getAscent () {
    return padArea.getAscent();
  }

/***********************************************************************
 * Ermittelt die typische Unterl&auml;nge des Fonts unter die
 * Grundlinie.
 * @return  Descent.
 ***********************************************************************/
  public int getDescent () {
    return padArea.getDescent();
  }

/***********************************************************************
 * Ermittelt den Zeilenabstand unter Ber&uuml;cksichtigung des
 * Ascents und Descents.
 * @return  Spacing.
 ***********************************************************************/
  public int getLeading () {
    return padArea.getSpacing();
  }

/***********************************************************************
 * Zeichnet eine gerade Linie zwischen zwei Punkten
 * @param x1  x-Koordinate des ersten Punktes.
 * @param y1  y-Koordinate des ersten Punktes.
 * @param x2  x-Koordinate des zweiten Punktes.
 * @param y2  y-Koordinate des zweiten Punktes.
 ***********************************************************************/
  public void drawLine (int x1, int y1, int x2, int y2 ) {
    padArea.drawLine(x1,y1,x2,y2);
  }

/***********************************************************************
 * Zeichnet eine gerade Linie zwischen zwei Punkten
 * @param p1  erster Punkt.
 * @param p2  zweiter Punkt.
 ***********************************************************************/
  public void drawLine (Point p1, Point p2 ) {
    padArea.drawLine(p1, p2);
  }

/***********************************************************************
 * Zeichnet eine Kreislinie mit einem gegebenen Radius um
 *  einen gegebenen Mittelpunkt.
 *  @param x   x-Koordinate des Mittelpunkts.
 *  @param y   y-Koordinate des Mittelpunkts.
 *  @param radius  Gew&uuml;nschter Radius des Kreises.
 ***********************************************************************/
  public void drawCircle (int x, int y, int radius) {
    padArea.drawCircle(x,y,radius);
  }

/***********************************************************************
 * Zeichnet eine Kreislinie mit einem gegebenen Radius um
 *  einen gegebenen Mittelpunkt.
 *  @param p   Mittelpunkt.
 *  @param radius  Gew&uuml;nschter Radius des Kreises.
 ***********************************************************************/
  public void drawCircle (Point p, double radius) {
    padArea.drawCircle(p,radius);
  }

/***********************************************************************
 * Zeichnet eine farbige Kreisfl&auml;che mit einem gegebenen
 *  Radius um einen gegebenen Mittelpunkt.
 *  @param x  x-Koordinate des Mittelpunkts.
 *  @param y  y-Koordinate des Mittelpunkts.
 *  @param radius  Gew&uuml;nschter Radius des Kreises.
 *  @see #setColor(Color col)
 ***********************************************************************/
  public void fillCircle (int x, int y, int radius) {
    padArea.fillCircle(x,y,radius);
  }

/***********************************************************************
 * Zeichnet eine farbige Kreisfl&auml;che mit einem gegebenen
 *  Radius um einen gegebenen Mittelpunkt.
 *  @param p   Mittelpunkt.
 *  @param radius  Gew&uuml;nschter Radius des Kreises.
 *  @see #setColor(Color col)
 ***********************************************************************/
  public void fillCircle (Point p, int radius) {
    padArea.fillCircle(p,radius);
  }

/***********************************************************************
 * Zeichnet den Umri&szlig; einer Ellipse, die in das durch die
 *  Parameter gegebene Rechteck pa&szlig;t.
 *  @param x   x-Koordinate der linken oberen Ecke.
 *  @param y   y-Koordinate der linken oberen Ecke.
 *  @param w   Breite des Rechtecks.
 *  @param h   H&oumlhe des Rechtecks.
 ***********************************************************************/
  public void drawOval (int x, int y, int w, int h) {
    padArea.drawOval(x,y,w,h);
  }

/***********************************************************************
 * Zeichnet den Umri&szlig; einer Ellipse, die in das durch die
 *  Parameter gegebene Rechteck pa&szlig;t.
 *  @param p   linke obere Ecke.
 *  @param w   Breite des Rechtecks.
 *  @param h   H&oumlhe des Rechtecks.
 ***********************************************************************/
  public void drawOval (Point p, int w, int h) {
    padArea.drawOval(p,w,h);
  }

/***********************************************************************
 * Zeichnet eine Ellipsenfl&auml;che, die in das durch die
 *  Parameter gegebene Rechteck pa&szlig;t.
 *  @param x   x-Koordinate der linken oberen Ecke.
 *  @param y   y-Koordinate der linken oberen Ecke.
 *  @param w   Breite des Rechtecks.
 *  @param h   H&oumlhe des Rechtecks.
 ***********************************************************************/
  public void fillOval (int x, int y, int w, int h) {
    padArea.fillOval(x,y,w,h);
  }

/***********************************************************************
 * Zeichnet eine Ellipsenfl&auml;che, die in das durch die
 *  Parameter gegebene Rechteck pa&szlig;t.
 *  @param p   linke obere Ecke.
 *  @param w   Breite des Rechtecks.
 *  @param h   H&oumlhe des Rechtecks.
 ***********************************************************************/
  public void fillOval (Point p, int w, int h) {
    padArea.fillOval(p,w,h);
  }

/***********************************************************************
 *  Zeichnet den Umri&szlig; eines Rechtecks.
 *  @param x   x-Koordinate der linken oberen Ecke.
 *  @param y   y-Koordinate der linken oberen Ecke.
 *  @param w   Breite des Rechtecks.
 *  @param h   H&oumlhe des Rechtecks.
 ***********************************************************************/
  public void drawRect (int x, int y, int w, int h) {
    padArea.drawRect(x,y,w,h);
  }

/***********************************************************************
 *  Zeichnet den Umri&szlig; eines Rechtecks.
 *  @param p   linke obere Ecke.
 *  @param w   Breite des Rechtecks.
 *  @param h   H&oumlhe des Rechtecks.
 ***********************************************************************/
  public void drawRect (Point p, int w, int h) {
    padArea.drawRect(p,w,h);
  }

/***********************************************************************
 *  Zeichnet eine rechteckige Fl&auml;che.
 *  @param x   x-Koordinate der linken oberen Ecke.
 *  @param y   y-Koordinate der linken oberen Ecke.
 *  @param w   Breite des Rechtecks.
 *  @param h   H&oumlhe des Rechtecks.
 ***********************************************************************/
  public void fillRect (int x, int y, int w, int h) {
    padArea.fillRect(x,y,w,h);
  }

/***********************************************************************
 *  Zeichnet eine rechteckige Fl&auml;che.
 *  @param p   linke obere Ecke.
 *  @param w   Breite des Rechtecks.
 *  @param h   H&oumlhe des Rechtecks.
 ***********************************************************************/
  public void fillRect (Point p, int w, int h) {
    padArea.fillRect(p,w,h);
  }

/***********************************************************************
 *  Zeichnet den Umri&szlig; eines Rechtecks mit 3D-Effekt.
 *  @param x   x-Koordinate der linken oberen Ecke.
 *  @param y   y-Koordinate der linken oberen Ecke.
 *  @param w   Breite des Rechtecks.
 *  @param h   H&oumlhe des Rechtecks.
 *  @param raised  Mit 3D-Effekt?
 ***********************************************************************/
  public void draw3DRect (int x, int y, int w, int h, boolean raised) {
    padArea.draw3DRect(x,y,w,h,raised);
  }

/***********************************************************************
 *  Zeichnet den Umri&szlig; eines Rechtecks mit 3D-Effekt.
 *  @param p   linke obere Ecke.
 *  @param w   Breite des Rechtecks.
 *  @param h   H&oumlhe des Rechtecks.
 *  @param raised  Mit 3D-Effekt?
 ***********************************************************************/
  public void draw3DRect (Point p, int w, int h, boolean raised) {
    padArea.draw3DRect(p,w,h,raised);
  }

/***********************************************************************
 *  Zeichnet eine rechteckige Fl&auml;che mit 3D-Effekt.
 *  @param x   x-Koordinate der linken oberen Ecke.
 *  @param y   y-Koordinate der linken oberen Ecke.
 *  @param w   Breite des Rechtecks.
 *  @param h   H&oumlhe des Rechtecks.
 *  @param raised  Mit 3D-Effekt?
 ***********************************************************************/
  public void fill3DRect (int x, int y, int w, int h, boolean raised) {
    padArea.fill3DRect(x,y,w,h,raised);
  }

/***********************************************************************
 *  Zeichnet eine rechteckige Fl&auml;che mit 3D-Effekt.
 *  @param p   linke obere Ecke.
 *  @param w   Breite des Rechtecks.
 *  @param h   H&oumlhe des Rechtecks.
 *  @param raised  Mit 3D-Effekt?
 ***********************************************************************/
  public void fill3DRect (Point p, int w, int h, boolean raised) {
    padArea.fill3DRect(p,w,h,raised);
  }

/***********************************************************************
 *  Zeichnet den Umriss eines Polygons.
 *  @param x   Array mit den X-Koordinaten der Eckpunkte.
 *  @param y   Array mit den Y-Koordinaten der Eckpunkte.
 ***********************************************************************/
  public void drawPolygon( int x[], int y[] ){
    padArea.drawPolygon( x, y );
  }

/***********************************************************************
 *  Zeichnet den Umriss eines Polygons.
 *  @param p   Array mit den Koordinaten der Eckpunkte.
 ***********************************************************************/
  public void drawPolygon( Point p[] ){
    padArea.drawPolygon(p);
  }

/***********************************************************************
 * Gibt eine Zeichenkette aus.
 * @param text  Zeichenkette.
 * @param x  x-Koordinate.
 * @param y  y-Koordinate.
 ***********************************************************************/
  public void drawString ( String text, int x, int y ) {
    padArea.drawString( text, x, y );
  }

/***********************************************************************
 * Gibt eine Zeichenkette aus.
 * @param text  Zeichenkette.
 * @param p  Koordinaten.
 ***********************************************************************/
  public void drawString ( Point p, String text) {
    padArea.drawString( p, text );
  }

/***********************************************************************
 * Gibt eine ganze Zahl (long) als Zeichenkette aus.
 * Kleinere numerische Typen werden automatisch umgewandelt.
 * @param zahl 64-Bit Integer
 * @param x  x-Koordinate.
 * @param y  y-Koordinate.
 ***********************************************************************/
  public void drawString ( long zahl, int x, int y ) {
    padArea.drawString( String.valueOf(zahl), x, y );
  }

/***********************************************************************
 * Gibt eine ganze Zahl (long) als Zeichenkette aus.
 * Kleinere numerische Typen werden automatisch umgewandelt.
 * @param zahl 64-Bit Integer
 * @param p  Koordinaten.
 ***********************************************************************/
  public void drawString ( Point p, long zahl) {
    padArea.drawString( p, String.valueOf(zahl));
  }

/***********************************************************************
 * Gibt eine Gleitpunktzahl doppelter Genauigkeit als
 * Zeichenkette aus. Kleinere numerische Typen werden
 * automatisch umgewandelt.
 * @param zahl Gleitpunktzahl doppelter Genauigkeit
 * @param x  x-Koordinate.
 * @param y  y-Koordinate.
 ***********************************************************************/
  public void drawString ( double zahl, int x, int y ) {
    padArea.drawString( String.valueOf( zahl ), x, y );
  }

/***********************************************************************
 * Gibt eine Gleitpunktzahl doppelter Genauigkeit als
 * Zeichenkette aus. Kleinere numerische Typen werden
 * automatisch umgewandelt.
 * @param zahl Gleitpunktzahl doppelter Genauigkeit
 * @param p  Koordinaten.
 ***********************************************************************/
  public void drawString ( Point p, double zahl) {
    padArea.drawString( p, String.valueOf( zahl ) );
  }

/***********************************************************************
 * Zeichnet einen Punkt.
 * @param x  x-Koordinate.
 * @param y  y-Koordinate.
 ***********************************************************************/
  public void drawDot (int x, int y) {
    padArea.fillCircle(x,y,point);
  }

/***********************************************************************
 * Zeichnet einen Punkt.
 * @param p  Koordinaten.
 ***********************************************************************/
  public void drawDot (Point p) {
    padArea.fillCircle(p,point);
  }

/***********************************************************************
 * L&ouml;scht das Pad.
 ***********************************************************************/
  public void clear () {
    padArea.clear();
  }

  /* Verz&ouml;gert graphisch Operationen, so da&szlig; sie f&uuml;
 * den Betrachter sichtbar werden.
 ***********************************************************************/
  public void slow (int f) {
    padArea.slow(f);
  }


/***********************************************************************
 * Speichert den aktuellen Inhalt des Pads in einen Zwischenpuffer,
 * der mit restore() wiederhergestellt werden kann.
 * @see #restore()
 ***********************************************************************/
  public void save() {
    padArea.save() ;
  }

/***********************************************************************
 * Stellt den Inhalt des Pads, der mit der Methode save() in einen
 * Zwischenpuffer gesichert wurde, wieder her.
 * @see #save()
 ***********************************************************************/
  public void restore() {
    padArea.restore() ;
  }

} // end of class Pad



/************************************************************************/
class PadArea extends JLabel {

  private int delay = 1;    // in milliseconds
  private int factor = 1;

  private int width = 1024;
  private int height = 768;

  private BufferedImage ImageBuffer1;
  private BufferedImage ImageBuffer2;
  private Graphics2D gb;
  private boolean switchBuf=false;
  private FontMetrics metrics;

  public PadArea () {
    setBackground( Pad.lightBlue );
    setForeground( Pad.black );
  }

  public void initialize () {
    if ( ImageBuffer1 == null ) {
      ImageBuffer1 = (BufferedImage)createImage( getWidth(), getHeight() );
      gb = ( Graphics2D )ImageBuffer1.getGraphics();
//       gb.setRenderingHint( RenderingHints.KEY_ANTIALIASING,
//         RenderingHints.VALUE_ANTIALIAS_ON );
    }
    if (ImageBuffer2 == null)
      ImageBuffer2 = (BufferedImage)createImage( getWidth(), getHeight() );
    setStroke( Pad.stroke );
  }

  public void redraw()
  {
    repaint(0L);
  }

  public void switchBuffer()
  {
    switchBuf = !switchBuf;
    if (switchBuf)
      gb = ( Graphics2D )ImageBuffer1.getGraphics();
    else
      gb = ( Graphics2D )ImageBuffer2.getGraphics();
    clear();
    repaint(0L);
    pause();
  }

  void setAreaSize(int width, int height) {
    this.width = width ;
    this.height = height ;
  }

  public Dimension getPreferredSize(){
    return new Dimension( width, height );
  }

  public void paintComponent( Graphics g ) {
    Graphics2D g2 = ( Graphics2D ) g;
    super.paintComponent( g2 );

    if (switchBuf)
    {
      if ( ImageBuffer1 != null )
        g2.drawImage( ImageBuffer2, 0, 0, this );
    }
    else
    {
      if ( ImageBuffer1 != null )
        g2.drawImage( ImageBuffer1, 0, 0, this );
    }
  }

  public void setColor ( Color color ) {
    gb.setColor( color );
  }

  public void setFont ( Font fnt ) {
    super.setFont( fnt );
    if ( gb != null ) gb.setFont( fnt );
  }

  public int getStringWidth ( String text ) {
    metrics = gb.getFontMetrics();
    return metrics.stringWidth( text );
  }

  public int getCharWidth ( char symbol ) {
    metrics = gb.getFontMetrics();
    return metrics.charWidth( symbol );
  }

  public int getLineHeight () {
    metrics = gb.getFontMetrics();
    return metrics.getHeight();
  }

  public int getAscent () {
    metrics = gb.getFontMetrics();
    return metrics.getAscent();
  }

  public int getDescent () {
    metrics = gb.getFontMetrics();
    return metrics.getDescent();
  }

  public int getSpacing () {
    metrics = gb.getFontMetrics();
    return metrics.getLeading();
  }

  public void drawLine ( int x1, int y1, int x2, int y2 ) {
    gb.draw( new Line2D.Double( x1, y1, x2, y2 ) );
  }

  public void drawLine ( Point p1, Point p2 ) {
    gb.draw( new Line2D.Double( p1.getX(), p1.getY(), p2.getX(), p2.getY() ) );
  }

  public void drawCircle (int x, int y, int radius) {
    gb.draw( new Ellipse2D.Double( x-radius, y-radius,
           2*radius, 2*radius ) );
  }

  public void drawCircle (Point p, double radius) {
    gb.draw( new Ellipse2D.Double( p.getX()-radius, p.getY()-radius,
           2*radius, 2*radius ) );
  }

  public void fillCircle (int x, int y, int radius) {
    gb.fill( new Ellipse2D.Double( x-radius, y-radius, 2*radius, 2*radius ) );
  }

  public void fillCircle (Point p, int radius) {
    gb.fill( new Ellipse2D.Double( p.getX()-radius, p.getY()-radius,
           2*radius, 2*radius ) );
  }

  public void drawOval (int x, int y, int w, int h) {
    gb.draw( new Ellipse2D.Double( x, y, w, h ) );
  }

  public void drawOval (Point p, int w, int h) {
    gb.draw( new Ellipse2D.Double( p.getX(), p.getY(), w, h ) );
  }

  public void fillOval (int x, int y, int w, int h) {
    gb.fill( new Ellipse2D.Double( x, y, w, h ) );
  }

  public void fillOval (Point p, int w, int h) {
    gb.fill( new Ellipse2D.Double( p.getX(), p.getY(), w, h ) );
  }

  public void drawRect (int x, int y, int w, int h) {
    gb.draw( new Rectangle2D.Double( x, y, w, h ) );
  }

  public void drawRect (Point p, int w, int h) {
    gb.draw( new Rectangle2D.Double( p.getX(), p.getY(), w, h ) );
  }

  public void fillRect (int x, int y, int w, int h) {
    gb.fill( new Rectangle2D.Double( x, y, w, h ) );
  }

  public void fillRect (Point p, int w, int h) {
    gb.fill( new Rectangle2D.Double( p.getX(), p.getY(), w, h ) );
  }

  public void draw3DRect (int x, int y, int w, int h, boolean raised) {
    gb.draw3DRect(x,y,w,h,raised);
  }

  public void draw3DRect (Point p, int w, int h, boolean raised) {
    gb.draw3DRect(round(p.getX()), round(p.getY()),w,h,raised);
  }

  public void fill3DRect (int x, int y, int w, int h, boolean raised) {
    gb.fill3DRect(x,y,w,h,raised);
  }

  public void fill3DRect (Point p, int w, int h, boolean raised) {
    gb.fill3DRect(round(p.getX()),round(p.getY()),w,h,raised);
  }

  public void drawPolygon( int x[], int y[] ) {
    GeneralPath p = new GeneralPath( GeneralPath.WIND_EVEN_ODD,
             x.length );
    p.moveTo( x[0], y[0] );
    for ( int i = 1; i < x.length; i++ ) {
      p.lineTo( x[i], y[i] );
    }
    p.closePath();
    gb.draw( p );
  }

  public void drawPolygon( Point p[] ) {
    GeneralPath path = new GeneralPath( GeneralPath.WIND_EVEN_ODD,
             p.length );
    path.moveTo( round(p[0].getX()), round(p[0].getY()) );
    for ( int i = 1; i < p.length; i++ ) {
      path.lineTo( round(p[i].getX()), round(p[i].getY()) );
    }
    path.closePath();
    gb.draw( path );
  }

  public void drawString (String text, int x, int y) {
    gb.drawString(text,x,y);
  }

  public void drawString (Point p, String text) {
    gb.drawString(text,round(p.getX()),round(p.getY()));
  }

  public void clear () {
    gb.clearRect(0,0,getSize().width,getSize().height);
  }

  public void slow (int f) {
    factor = f;
  }

/***********************************************************************
 * undocumented features
***********************************************************************/
  public void setStroke( Stroke s ) {
    gb.setStroke( s );
  }

  public void draw( Shape s ) {
    gb.draw( s );
  }

  private void pause () {
    try { Thread.sleep( factor * delay );}
    catch ( InterruptedException e ) {};
  }


  private BufferedImage backup ;

  public void save() {
    if (backup == null)
      backup = (BufferedImage)createImage(width, height) ;
    repaint(0L);
    if (switchBuf)
      backup.setData(ImageBuffer1.getData());
    else
      backup.setData(ImageBuffer2.getData());
  }

  public void restore() {
    if (backup == null)
      backup = (BufferedImage)createImage(width, height);
    if (switchBuf)
      ImageBuffer1.setData(backup.getData());
    else
      ImageBuffer2.setData(backup.getData());
    repaint(0L) ;
  }

    private int round (double d) {
  return (int) d;
    }
} // end of class PadArea



