import java.applet.*;
import java.net.*;
import java.awt.*;
import java.util.*;
import java.io.*;
import java.lang.*;
public class Status extends Applet implements Runnable {
 String hoststr, portstr; // parameters giving host & port
 int current;
 TextField hostfield;
 TextField portfield;
 Socket s;
 InputStream in;
 OutputStream out;
 URL imageSource;
 Image images [];
 Thread engine = null;
 long TT = 0;
 Needle needle;
 boolean debug = false, cleared = false;
 public void clearTime () {
  TT = System.currentTimeMillis ();
  cleared = true;
  // display ("Time cleared\n");
 }
 void showTime () {
  if (cleared) {
    cleared = false;
    TT = System.currentTimeMillis () - TT;
    needle.setValue (TT);
  }
  // display ("Time " +TT+ "\n");
 }
 void transmitch(char c) {
   try {
     out.write((int)c);
   }
   catch(IOException e) {};
   if (debug) System.out.println("Sent char " + String.valueOf((int)c) +
     " = " + String.valueOf(c));
 }
 void transmit(byte str[], int len) {
   int i;
   for(i=0; i<len; i++) transmitch ((char) (str [i]));
   try {
     out.flush();
   }
   catch(IOException e) {};
 }
 void display (String s) {
   System.out.println (s);
 }
 void outImage (int c) {
 }
 public void paint(Graphics g) {
   Dimension d = size();
   /*display ("Drawing\n");*/
   //g.setColor(Color.blue);
   //g.drawRect(0, 0, d.width - 1, d.height - 1);
   g.drawImage (images [current], 0, 0, this);
   //g.drawString("Round-trip time: " +TT+ "ms", 50, 25);
 }
 public void init() {
   // Set up interface
   String param = getParameter("imagesource");
   try {
     imageSource = (param == null) ? getDocumentBase() :
                                       new URL(getDocumentBase(), param + "/");
   }
   catch (MalformedURLException e) {
     display ("Malformed URL\n");
     return;
   }
   setBackground (new Color (238,233,191));
   hoststr = getParameter("host");
   portstr = getParameter("port");
   if (hoststr == null) hoststr = "legal.cs.ualberta.ca";
   if (portstr == null) portstr = "2286";
   needle = new Needle (0.0, 5000.0, 1000.0, 0.0);
   needle.setZone("cyanzone",2000.0,3000.0,Color.cyan);
   needle.setZone("greenzone",3000.0,4000.0,Color.green);
   needle.setZone("redzone",4000.0,5000.0,Color.red);
   needle.setNumbersFont(new Font("Helvetica", Font.PLAIN, 7));
   needle.setCaption("Round-trip delay");
   //setLayout(new GridLayout(1, 2));
   add (needle);
 }
 public void run () {
   // Fetch images
   images = new Image [2];
   try {
     URL url = new URL (imageSource, "Off.gif");
//display("URL: "+url+"\n");
     images [1] = getImage (url);
     url = new URL (imageSource, "On.gif");
//display("URL: "+url+"\n");
     images [0] = getImage (url);
   }
   catch (MalformedURLException e) {
     display ("Malformed URL\n");
     return;
   }
   byte msg[] = new byte [5];
   int port = Integer.parseInt(portstr);
   try {
     try {
       try {
         if ((s = new Socket(hoststr,port)) == null) {
           display("Failed to connect to host "+hoststr+"\n");
           return;
         }
       }
       catch(UnknownHostException e) {
         display("Host " + hoststr + " not found\n");
         return;
       }
     }
     catch(IOException e) {
       display("IOException on connect to host "+hoststr+" " +port+ "\n");
       e.printStackTrace();
       return;
     }
   }
   catch (Exception e) {
     display("Security violation on socket for host "+hoststr+"\n");
     return;
   }
   try {
     in = s.getInputStream();
     out = s.getOutputStream();
   }
   catch(IOException e) {
     if (debug) System.out.println("Failed to get stream from socket");
     System.exit(5);
   }
//   display("Connected to "+hoststr+"\n");
   if (debug) System.out.println("Connected to host");
   msg [0] = 9; // CRT_AIS
   msg [1] = 0; // Len 1
   msg [2] = 2; // Len 2
   msg [3] = 1; // Bus
   msg [4] = 1; // Addr
   transmit (msg, 5);
   while (true) {
     try {
       int c = in. read ();
// display ("Read "+c+"\n");
       if (c == 0x6e) {
         c = in. read ();
// display ("Skipped "+c+"\n");
         c = in. read ();
// display ("Status "+c+"\n");
         current = c;
         if (current > 1 || current < 0) current = 1;
         showTime ();
         repaint ();
       }
     }
     catch(IOException e) {};
   }
 }
 public void start () {
   if (engine == null) {
     engine = new Thread(this);
     engine.start();
   }
 }
 public void stop() {
   if (engine != null && engine.isAlive()) {
     engine.stop();
   }
   engine = null;
 }
}
/**
  NAME:         Needle.java
                Copyright (c) 1996 Liam Relihan. All rights reserved.
  PURPOSE:      This class implements a simple swing needle widget.
                Inherits from Meter.class.
  AUTHOR:       Liam Relihan
                <relihanl@ul.ie>
  SCCS:         %M%        %R%.%L% %D%
  DATE:         15-May-1996
  PARAMETERS:   
  CALLED BY:
  MODIFIED:     
  BUGS:         
 */
class Needle extends Meter
{
   private double radiansPerDegree = Math.PI / 180;
   private final double MIN_NEEDLE_ANGLE=10.0;
   private final double MAX_NEEDLE_ANGLE=170.0;
   public Vector zones;
   protected Color needleColor=Color.black;
   /**  Constructor for Needle class.
    * @arg lo     lowest value the meter can indicate
    * @arg hi     highest value the meter can indicate
    * @arg grad   graduations in which scale is marked
    * @arg val    initial value
    */
   public Needle(double lo,double hi,double grad,double val){
     super(lo,hi,grad,val);
     zones=new Vector();
   }
   /** set a color zone, e.g. a red "Danger" zone 
    * @arg zoneName     the name of the zone, e.g. "danger zone"
    * @arg lowerBound   the lower bound of the zone
    * @arg upperBound   the upper bound of the zone
    * @arg color        the color of the zone
    */
   public void setZone(String zoneName,
                       double lowerBound,
                       double upperBound,
                       Color color){
     if (lowerBound<lowestValue) lowerBound=lowestValue;
     if (upperBound<lowestValue) upperBound=lowestValue;
     if (lowerBound>highestValue) lowerBound=highestValue;
     if (upperBound>highestValue) upperBound=highestValue;
     zones.addElement(new MeterZone(zoneName,lowerBound,upperBound,color));
     repaint();
   }
   /** set the needle color 
    * @arg theColor   the color of the needle
    */
   public void setNeedleColor(Color theColor){
     needleColor=theColor;
     repaint();
   }
   /** get the needle color */
   public Color getNeedleColor(){
     return(needleColor);
   }
   /* draws the gauge in the background. This allows the reduction of
    * flicker.
    */
   protected void drawMeter(Graphics g) {
     int width=1000;
     int height=1000;
     int needle_length=width/3;
     int mark_length=width/30;
     int width_of_scale=needle_length*2;
     int meter_border=20;
     int pivot_x=width/2;
     int pivot_y=(int)((double)height*0.7);
     int caption_y=(int)((double)height*0.85);
     int coil_width=width/30;
     int coil_height=height/30;
     ScalablePlane sp=new ScalablePlane(width,
                                        height,
                                        bounds().width,
                                        bounds().height);
     g.setColor(new Color (205, 92, 92));
     g.fill3DRect(sp.x(meter_border),
                  sp.y(meter_border),
                  sp.x(width-meter_border*2),
                  sp.y(height-meter_border*2),
                  true);
     // draw the colored zones
     for(int j=0;j<zones.size();j++){
       MeterZone cur_zone= (MeterZone)zones.elementAt(j);
       double start_angle=valueToAngle(cur_zone.getZoneLow());
       double end_angle=valueToAngle(cur_zone.getZoneHigh());
       g.setColor(cur_zone.getZoneColor());
       g.fillArc(sp.x(pivot_x-needle_length),sp.y(pivot_y-needle_length),
                 sp.x(needle_length*2),sp.y(needle_length*2),
                 (int)start_angle,
                 (int)(end_angle-start_angle));
     }
     g.setColor(Color.black);
     // draw graduations
     for(double i=lowestValue;i<=highestValue;i=i+scaleGrad) {
       double mark_angle=valueToAngle(i);
       mark_angle=360-mark_angle;
       mark_angle=mark_angle * radiansPerDegree;
       int mark_x1=sp.x(pivot_x)+
         (int)(Math.cos(mark_angle)*(double)sp.x(needle_length));
       int mark_y1=sp.y(pivot_y)+
         (int)(Math.sin(mark_angle)*(double)sp.y(needle_length));
       int mark_x2=sp.x(pivot_x)+
         (int)(Math.cos(mark_angle)*(double)sp.x(needle_length+mark_length));
       int mark_y2=sp.y(pivot_y)+
         (int)(Math.sin(mark_angle)*(double)sp.y(needle_length+mark_length));
       g.drawLine(mark_x1,mark_y1,mark_x2,mark_y2);
       int text_x=sp.x(pivot_x)+
         (int)(Math.cos(mark_angle)*
               (double)sp.x(needle_length+mark_length*2));
       int text_y=sp.y(pivot_y)+
         (int)(Math.sin(mark_angle)*
               (double)sp.y(needle_length+mark_length*2));
       g.setFont(numbersFont);
       int mid_string=(g.getFontMetrics().stringWidth(String.valueOf(i))/2);
       g.drawString(String.valueOf(i),text_x-mid_string,text_y);
     }
     // draw the needle
     double angle=valueToAngle(value);
     angle=360-angle;
     angle=angle * radiansPerDegree;
     g.setColor(needleColor);
     g.drawLine(sp.x(pivot_x),
                sp.y(pivot_y),
                sp.x(pivot_x)+
                (int)(Math.cos(angle)*(double)sp.x(needle_length)),
                sp.y(pivot_y)+
                (int)(Math.sin(angle)*(double)sp.y(needle_length)));
     // draw coil
     g.fillRect(sp.x(pivot_x-coil_width/2),
                sp.y(pivot_y-coil_height/2),
                sp.x(coil_width),
                sp.y(coil_height));
     // draw caption
     g.setFont(captionFont);
     int caption_length=g.getFontMetrics().stringWidth(caption);
     g.drawString(caption,sp.x(pivot_x)-(caption_length/2),sp.y(caption_y));
   }
   public Dimension minimumSize() { return new Dimension(250, 150); }
   public Dimension preferredSize() { return minimumSize(); }
   /** calculates the angle (in degrees) for a given value */
   private double valueToAngle(double value){
     double degrees_per_unit=(MAX_NEEDLE_ANGLE-MIN_NEEDLE_ANGLE)/
       (highestValue-lowestValue);
     return MAX_NEEDLE_ANGLE-((value-lowestValue)*degrees_per_unit);
   }
 }
class MeterZone extends Object implements Cloneable {
  private String zonename;
  private double lo;
  private double hi;
  private Color zonecolor;
  public MeterZone(String name_val,double lo_val, double hi_val, Color color){
    zonename=name_val;
    lo=lo_val;
    hi=hi_val;
    zonecolor=color;
  }
  public String getZoneName(){ return zonename;}
  public double getZoneLow(){ return lo;}
  public double getZoneHigh(){ return hi;}
  public Color getZoneColor(){ return zonecolor;}
}
/**
  NAME:         Meter.java
                Copyright (c) 1996 Liam Relihan. All rights reserved.
  PURPOSE:      

  AUTHOR:       Liam Relihan
                <relihanl@ul.ie>
  SCCS:         %M%        %R%.%L% %D%
  DATE:         15-May-1996
  PARAMETERS:   
  CALLED BY:
  MODIFIED:     10-June-96     LOR     Added control for Panel color
  BUGS:         
 */
abstract class Meter extends Panel
{
   protected double lowestValue=0.0;
   protected double highestValue=100.0;
   protected double scaleGrad=10.0;
   protected double value = 0.0;
   protected Font captionFont;
   protected Font numbersFont;
   protected String caption="Uncaptioned Meter";
   protected Image offScreenImage;
   protected Dimension offScreenSize;
   protected Graphics offScreenGraphics;
   protected Color panelColor=Color.lightGray;
   /**  Constructor for Meter class.
    * @arg lo     lowest value the meter can indicate
    * @arg hi     highest value the meter can indicate
    * @arg grad   graduations in which scale is marked
    * @arg val    initial value
    */
   public Meter(double lo,double hi,double grad,double val) {
     lowestValue=lo;
     highestValue=hi;
     scaleGrad=grad;
     value=val;
     // default fonts
     captionFont = new Font("Helvetica", Font.PLAIN, 15);
     numbersFont = new Font("Helvetica", Font.PLAIN, 10);
   }
   /**  set the valid range of the  meter
    * @arg theMinimum     lowest value the meter can indicate
    * @arg theMaximum     highest value the meter can indicate
    */
   public void setRange(int theMinimum, int theMaximum) {
      lowestValue = theMinimum;
      highestValue = theMaximum;
      repaint();
   }
   /**  set the value that meter should indicate
    * @arg theValue     the new value of the meter
    */
   public void setValue(double theValue)
   {
      value = theValue;
      if (value < lowestValue)
         value = lowestValue;
      if (value > highestValue)
         value = highestValue;
      repaint();
   }
   /** get the minimum value that the meter can indicate */
   public double getMinimum() { return lowestValue; }
   /** get the maximum value that the meter can indicate */
   public double getMaximum() { return highestValue; }
   /** get the value that the meter is indicating */
   public double getValue() { return value; }
   /** sets the panel color */
   public void setPanelColor(Color theColor)
   {
      panelColor = theColor;
      repaint();
   }
   /** gets the panel color */
   public Color getPanelColor()
   {
      return(panelColor);
   }
   /** sets the caption */
   public void setCaption(String theCaption)
   {
      caption = theCaption;
      repaint();
   }
   /** sets the font of the caption */
   public void setCaptionFont(Font theFont)
   {
      captionFont = theFont;
      repaint();
   }
   /** sets the font of the readings */
   public void setNumbersFont(Font theFont)
   {
      numbersFont = theFont;
      repaint();
   }
   /** paint the meter on the foreground*/
   public synchronized void paint(Graphics g) {
     Dimension d = this.size();
     if((offScreenImage == null) || (d.width != offScreenSize.width) ||
        (d.height != offScreenSize.height)) {
       offScreenImage = createImage(d.width, d.height);
       offScreenSize = d;
       offScreenGraphics = offScreenImage.getGraphics();
    }
     drawMeter(offScreenGraphics);
     g.drawImage(offScreenImage, 0, 0, null);
   }
   /** redefines inherited method to exclude screen clear */
   public void update(Graphics g) {
     paint(g);
   }
   /* draws the gauge in the background. This allows the reduction of
    * flicker.
    */
   protected abstract void drawMeter(Graphics g);
   public abstract Dimension minimumSize();
   public abstract Dimension preferredSize();
 }
class ScalablePlane extends Object implements Cloneable{
  public static double x_scale;
  public static double y_scale;
  public ScalablePlane(int sp_x, int sp_y, int actual_x, int actual_y) {
    x_scale=(double)actual_x/(double)sp_x;
    y_scale=(double)actual_y/(double)sp_y;
  }
  public int x(int sp_x){return (int)((double)sp_x*x_scale);}
  public int y(int sp_y){return (int)((double)sp_y*y_scale);}
}
