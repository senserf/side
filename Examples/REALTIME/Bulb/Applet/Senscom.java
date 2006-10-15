import java.applet.*;
import java.net.*;
import java.awt.*;
import java.util.*;
import java.io.*;
import java.lang.*;
public class Senscom extends Applet implements Runnable {
  final static int NEYES = 2; // How many eyes we have
  final static int EMASK = 3; // Mask for the number of frames
  final static int TWIDTH = 227; // Picture width
  final static int THEIGHT = 111; // and height
  final static int EWIDTH = 66; // Eye width
  final static int EHEIGHT = 63; // Eye height
  final static int VINSET = 40; // Vertical inset for the eyes
  final static int HINSET = 20; // Horizontal inset
  final static String TFILE = "onoff.gif";
  final static String EFILES [] = {"eye1.gif", "eye2.gif",
                                     "eye3.gif", "eye4.gif",
                                     "eyeC.gif"};
  final static int MINDELAY = 300;
  final static int DELAYMASK = 1023;
  String hoststr, portstr; // parameters giving host & port
  Panel controls;
  TextField hostfield;
  TextField portfield;
  Button sensoron, sensoroff;
  Socket s;
  InputStream in;
  OutputStream out;
  int MouseEye;
  MediaTracker MTracker;
  Image Title, Eyes [], CurrEye [];
  int NFrames, XCor [], YCor [];
  long CTime, UpdateTime [];
  Image OSI;
  Graphics OSG;
  Thread Animator = null;
  Random RNG;
  Rectangle OnRect, OffRect;
 public void init() {
    URL url;
    int i;
    MouseEye = -1;
    Eyes = new Image [(NFrames = EFILES.length - 1) + 1];
    CurrEye = new Image [NEYES];
    UpdateTime = new long [NEYES];
    for (i = 0; i < NEYES; i++) UpdateTime [i] = 0;
    XCor = new int [NEYES];
    YCor = new int [NEYES];
    CTime = 0;
    RNG = new Random ();
    try {
      url = new URL (getDocumentBase (), TFILE);
    } catch (MalformedURLException e) {
      url = null;
    }
    Title = getImage (url);
    MTracker = new MediaTracker (this);
    MTracker.addImage (Title, 0); // Preload everything before displaying
    for (i = 0; i <= NFrames; i++) {
      try {
        url = new URL (getDocumentBase (), EFILES [i]);
      } catch (MalformedURLException e) {
        url = null;
      }
      Eyes [i] = getImage (url);
      MTracker.addImage (Eyes [i], 0);
    }
    for (i = 0; i < NEYES; i++) CurrEye [i] = Eyes [0];
    // Calculate positions for the eyes
    XCor [0] = HINSET; YCor [0] = VINSET;
    XCor [1] = TWIDTH - HINSET - EWIDTH; YCor [1] = VINSET;
    OSI = createImage (TWIDTH, THEIGHT);
    OSG = OSI.getGraphics ();
    OnRect = new Rectangle (XCor [0], YCor [0], EWIDTH, EHEIGHT);
    OffRect = new Rectangle (XCor [1], YCor [1], EWIDTH, EHEIGHT);
   // Set up interface
   hoststr = getParameter("host");
   portstr = getParameter("port");
   if (hoststr == null) hoststr = "legal.cs.ualberta.ca";
   if (portstr == null) portstr = "2286";
 }
  public synchronized void update (Graphics g) {
    int i;
    OSG.drawImage (Title, 0, 0, TWIDTH, THEIGHT, null);
    for (i = 0; i < NEYES; i++)
      OSG.drawImage (CurrEye [i], XCor [i], YCor [i], EWIDTH, EHEIGHT, null);
    g.drawImage (OSI, 0, 0, null);
  }
 void transmitch(char c) {
   try {
     out.write((int)c);
   }
   catch(IOException e) {};
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
 void sensormsg (int command) {
   int port;
   byte msg[] = new byte [7];
   int c;
// Start "status" communication code
   Applet status = null;
   status = getAppletContext().getApplet("status");
   if (status != null) ((Status)status).clearTime ();
// End "status" communication code
   port = Integer.parseInt(portstr);
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
     System.exit(5);
   }
   // display("Connected to "+hoststr+"\n");
   msg [0] = 8; // Write output
   msg [1] = 0; // Len 1
   msg [2] = 4; // Len 2
   msg [3] = 1; // Bus
   msg [4] = 2; // Addr 
   msg [5] = 0; // Data1
   msg [6] = (byte) command;
   transmit (msg, 7);
   try {
     c = in. read (); // Receive the status info
     c = in. read ();
     c = in. read ();
     c = in. read ();
   }
   catch(IOException e) {};
   s = null; in = null; out = null;
 }
  public synchronized void start () {
    if (Animator == null) {
      Animator = new Thread (this);
      Animator.start ();
    }
  }
  public synchronized void stop () {
    if (Animator != null && Animator.isAlive ())
      Animator.stop ();
    Animator = null;
  }
  public void run () {
    int i, rand, ne, nd;
    Thread ct = Thread.currentThread ();
    ct.setPriority (Thread.MIN_PRIORITY);
    try {
      // Wait for the images to be downloaded
      MTracker.waitForAll ();
    } catch (InterruptedException e) {
    }
    while (Animator == ct) {
      // Check update times
      for (i = 0; i < NEYES; i++) {
        if (i != MouseEye && UpdateTime [i] <= CTime) {
          rand = RNG.nextInt ();
          ne = (int)((rand >> 6) & EMASK);
          nd = (int)((rand >> 10) & DELAYMASK);
          CurrEye [i] = Eyes [ne];
          UpdateTime [i] = CTime + nd;
        }
      }
      repaint ();
      try {
        Thread.sleep (100);
        CTime += 100;
      } catch (InterruptedException e) {
        CTime += 100;
      }
    }
  }
 public boolean mouseDown (Event e, int x, int y) {
   if (OnRect.inside (x, y)) {
     CurrEye [MouseEye = 0] = Eyes [NFrames];
     repaint ();
     return true;
   } else if (OffRect.inside (x, y)) {
     CurrEye [MouseEye = 1] = Eyes [NFrames];
     repaint ();
     return true;
   }
   return false;
 }
 public boolean mouseUp (Event e, int x, int y) {
   boolean found = false;
   if (OnRect.inside (x, y)) {
     if (MouseEye == 0) {
       sensormsg (1);
       found = true;
     }
   } else if (OffRect.inside (x, y)) {
     if (MouseEye == 1) {
       sensormsg (0);
       found = true;
     }
   }
   if (MouseEye != -1) {
     CurrEye [MouseEye] = Eyes [0];
     MouseEye = -1;
   }
   return found;
 }
}
