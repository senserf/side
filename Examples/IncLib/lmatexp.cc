#ifndef __lmatexp_c__
#define __lmatexp_c__

// This is the exposure code for local access delays shared by utraffil.cc and
// utraff2l.cc

UTraffic::exposure {
  int i, NS; LONG count;
  double X [MAXSTATIONS], Y [MAXSTATIONS], min, max, mom [2], Max;
  Boolean First;
  Traffic::expose;
  onpaper {
    exmode 4:
      if (Hdr == NULL) Hdr = "Local message access times";
      print (Hdr);
      print ("\n\n");
      for (i = 0, First = YES; i < NStations; i++)
        if (CInt [i] != NULL)
          if (First) {
            CInt [i] -> MAT -> printACnt ();
            First = NO;
          } else
            CInt [i] -> MAT -> printSCnt ();
      print ("\n");
  };
  onscreen {
    exmode 4:
      // Store the delays
      for (i = 0, NS = 0, Max = 0.0; i < NStations; i++) {
        X [i] = (double) i;
        if (CInt [i] != NULL) {
          NS = i;
          CInt [i] -> MAT -> calculate (min, max, mom, count);
          if ((Y [i] = mom [0]) > Max) Max = Y [i];
        } else
          Y [i] = 0.0;
      }
      startRegion (0.0, (double) NS, 0.0, Max * 1.05);
      displaySegment (022, NS+1, X, Y);
      endRegion ();
      for (i = 0; i <= NS; i++) {
        display (i);
        display (Y [i]);
      }
  }
};

void printLocalMeasures () { UTP->printOut (4); };

#endif
