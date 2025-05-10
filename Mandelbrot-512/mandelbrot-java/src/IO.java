//************************************************************
//************************************************************
//  IO-Redirection Module
//  Copyright (c) 2010 by Michael Buchberger
//************************************************************
//************************************************************
// Date: 2010-09-05               Coded by: Michael Buchberger
//                                Module name: default
//                                Source file: IO.java
//
// Program description: Redirects Input and/or Output
//************************************************************
// Development environment: JCreator Pro 4.5
//************************************************************
// Programmer comments:
// 2010-09-05 Initial Version
//************************************************************

import java.util.Locale;
import java.io.*;
import java.util.Map;


class Output {
  protected PrintStreamLogger out;
  protected BufferedReader keyboard;
  static protected InputStream consoleIn = System.in;
  static protected PrintStream consoleOut = System.out;
  static protected FileOutputStream fileOutput;
  static {
    try {
          fileOutput = new FileOutputStream("..\\output.txt");
    } catch (FileNotFoundException e) {
      e.printStackTrace();
    }
  }

  public Output() {
    try {
      out = new PrintStreamLogger(consoleOut,new BufferedOutputStream (fileOutput));
      keyboard = new BufferedReader(new InputStreamReader(consoleIn));
      System.setIn(consoleIn);
      System.setOut(out);
      System.setErr(out);
    } catch (Exception e) {
      e.printStackTrace();
    }
  }

  protected String getMainClassName()
  {
    Map<Thread,StackTraceElement[]> stackTraceMap = Thread.getAllStackTraces();
    for (Thread t : stackTraceMap.keySet()) {
      if ("main".equals(t.getName())) {
        StackTraceElement[] mainStackTrace = stackTraceMap.get(t);
        for (StackTraceElement element : mainStackTrace) {
          String cn = element.toString();
          if (cn.contains(".main"))
            return (cn.substring(0,cn.indexOf(".")));
        }
      }
    }
    return("unknown");
  }


  protected String getNextLine () throws IOException, NoInputException {
    String line = keyboard.readLine();
    out.logText(line);
    return (line);
  }

  private void println (String text) {
    out.println (text);
  }

  private void print (String text) {
    out.print (text);
  }

  public int inInt(String text){
    boolean correct;
    int value=0;
    String inputLine="";
    do {
      correct=true;
      print(text);
      try {
        inputLine = getNextLine();
      }
      catch(IOException e){
        println ("Eingabe nicht korrekt verlaufen!");
        correct = false;
      }
      catch (NoInputException e2) {
        println ("Keine Eingabedaten mehr vorhanden!");
        return (value);
      }
      try {
        value = Integer.parseInt(inputLine);
      }
      catch(NumberFormatException e1){
        println("Keine Zahl eingegeben!");
        correct = false;
      }
    } while (!correct);

    return value;
  }

  public double inDouble(String text){
    boolean correct=true;
    String inputLine="";
    double value=0;
    do {
      correct=true;
      print(text);
      try {
        inputLine = getNextLine();
      }
      catch(IOException e){
        println ("Eingabe nicht korrekt verlaufen!");
        correct = false;
      }
      catch (NoInputException e2) {
        println ("Keine Eingabedaten mehr vorhanden!");
        return (value);
      }
      try {
        value = Double.parseDouble(inputLine);
      }
      catch(Exception e1){
        println("Keine Kommazahl eingegeben!");
        correct = false;
      }
    } while (!correct);

    return value;
  }


  public String inString(String text) {
    boolean correct=true;
    String inputLine="";
    do {
      correct=true;
      print(text);
      try {
        inputLine = getNextLine();
      }
      catch(IOException e){
        println ("Eingabe nicht korrekt verlaufen!");
        correct = false;
      }
      catch (NoInputException e1) {
        println ("Keine Eingabedaten mehr vorhanden!");
        return (inputLine);
      }
    } while (!correct);

    return inputLine;
  }

  public  char inChar(String text){

    boolean correct = false;
    String inputLine="";
    char value=' ';

    while (correct == false){

      correct = true;
      print(text);
      try{
        inputLine = getNextLine();

      }
      catch(IOException e){
        println ("Eingabe nicht korrekt verlaufen!");
        correct = false;

      }
      catch (NoInputException e1) {
        println ("Keine Eingabedaten mehr vorhanden!");
        return (value);
      }
      try{
        value = inputLine.charAt(0);
      }
      catch(IndexOutOfBoundsException e1){
        println("Kein Zeichen an der ersten Stelle!");
        correct = false;
      }
    }

    return value;
  }
}


class InputOutput extends Output {
  private BufferedInputStream in;

  public InputOutput() {
    try {
      in = new BufferedInputStream(new FileInputStream("..\\input.txt"));
      System.setIn(in);
      keyboard = new BufferedReader(new InputStreamReader(System.in));
    } catch (Exception e) {
      e.printStackTrace();
    }
  }

  protected String getNextLine () throws IOException, NoInputException {
    String line="";
    boolean isComment;
    do{
        if (!keyboard.ready()) {
          throw new NoInputException("Keine Inputdaten mehr vorhanden!");
        }
        line = keyboard.readLine();
        isComment = (line != null) && (line.trim().startsWith("//"));
    } while (isComment);
    out.println(line);
    return (line);
  }
}

class NoInputException extends Exception {
  public NoInputException() {
  }
  public NoInputException(String e) {
    super(e);
  }
}

class PrintStreamLogger extends PrintStream {
  PrintStream printStream1, printStream2;
  public PrintStreamLogger (OutputStream outputStream1, OutputStream outputStream2) {
    super(outputStream1);
    printStream1 = new PrintStream (outputStream1,true);
    printStream2 = new PrintStream (outputStream2,true);
  }

  public void logText (String text) {
    printStream2.println(text);
  }

  public void print(boolean b) {
    printStream1.print(b);
    printStream2.print(b);
  }

  public void print(char c) {
    printStream1.print(c);
    printStream2.print(c);
  }

  public void print(int i) {
    printStream1.print(i);
    printStream2.print(i);
  }

  public void print(long l) {
    printStream1.print(l);
    printStream2.print(l);
  }

  public void print(float f) {
    printStream1.print(f);
    printStream2.print(f);
  }

  public void print(double d) {
    printStream1.print(d);
    printStream2.print(d);
  }

  public void print(char s[]) {
    printStream1.print(s);
    printStream2.print(s);
  }

  public void print(String s) {
    printStream1.print(s);
    printStream2.print(s);
  }

  public void print(Object obj) {
    printStream1.print(obj);
    printStream2.print(obj);
  }

  public void println() {
    printStream1.println();
    printStream2.println();
  }

  public void println(boolean x) {
    printStream1.println(x);
    printStream2.println(x);
  }

  public void println(char x) {
    printStream1.println(x);
    printStream2.println(x);
  }

  public void println(int x) {
    printStream1.println(x);
    printStream2.println(x);
  }

  public void println(long x) {
    printStream1.println(x);
    printStream2.println(x);
  }

  public void println(float x) {
    printStream1.println(x);
    printStream2.println(x);
  }

  public void println(double x) {
    printStream1.println(x);
    printStream2.println(x);
  }

  public void println(char x[]) {
    printStream1.println(x);
    printStream2.println(x);
  }

  public void println(String x) {
    printStream1.println(x);
    printStream2.println(x);
  }

  public void println(Object x) {
    printStream1.println(x);
    printStream2.println(x);
  }

  public PrintStream printf(String format, Object ... args) {
    printStream1.printf (format,args);
    printStream2.printf (format,args);
    return (this);
  }

  public PrintStream printf(Locale l, String format, Object ... args) {
    printStream1.printf (l,format,args);
    printStream2.printf (l,format,args);
    return (this);
  }

  public PrintStream format(String format, Object ... args) {
    printStream1.format (format,args);
    printStream2.format (format,args);
    return (this);
  }

  public PrintStream format(Locale l, String format, Object ... args) {
    printStream1.format (l,format,args);
    printStream2.format (l,format,args);
    return (this);
  }

  public PrintStream append(CharSequence csq) {
    printStream1.append (csq);
    printStream2.append (csq);
    return (this);
  }

  public PrintStream append(CharSequence csq, int start, int end) {
    printStream1.append (csq,start,end);
    printStream2.append (csq,start,end);
    return (this);
  }

  public PrintStream append(char c) {
    printStream1.append (c);
    printStream2.append (c);
    return (this);
  }

}

