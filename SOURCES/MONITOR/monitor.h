/* ooooooooooooooooooooooooooooooooooooo */
/* Copyright (C) 1991-03   P. Gburzynski */
/* ooooooooooooooooooooooooooooooooooooo */

/* --- */

/* ---------------------------------------------------------- */
/* The  constant below declares the i/o timeout for receiving */
/* a complete request from the  other  party  (or  sending  a */
/* complete  response to the other party). If this timeout is */
/* exceeded, The monitor ignores the  operation  and  assumes */
/* that  the  other  party  is dead. Feel free to change, but */
/* don't make it bigger than CTIMEOUT.                        */
/* ---------------------------------------------------------- */
#define         GTIMEOUT        30      // seconds

// -----------------------------------------------------------------------
// This is a shorter timeout for short things that should be there already
// -----------------------------------------------------------------------
#ifdef		STIMEOUT
#undef		STIMEOUT
#endif
#define		STIMEOUT        10     // seconds

/* ------------------------------------- */
/* Default size of polling socket buffer */
/* ------------------------------------- */
#define		LSKBUFSIZE      1024

/* ------------------------------------------------------- */
/* Short buffer size -- for sending simple status requests */
/* ------------------------------------------------------- */
#define         SHTBUFSIZE      256

/* ------------------------------------------ */
/* Large buffer size -- Smurph to DSD channel */
/* ------------------------------------------ */
#define		LRGBUFSIZE      8192

// These two are for the polling version only

#define         POLLDELAY_DRM   1000000   // Dormant poll delay (usec)
#define         POLLDELAY_RUN      5000   // Running poll delay

/* --------------------------------------------- */
/* The field length of program arguments for DSD */
/* --------------------------------------------- */
#define		SML_PARG	20

// -----------------------------------
// Some constants borrowed from Smurph
// -----------------------------------
#define YES     1
#define NO      0
#define NONE    (-1)
#define OK      0
#define ERROR   1

// -------------------------------------
// Values returned by readSignatureBlock
// -------------------------------------
#define SMURPH_SIGNATURE   2
#define DSD_SIGNATURE      3

class   SmDesc  {
/* ------------------------------------------ */
/* Description of a single simulator instance */
/* ------------------------------------------ */
  public:
  SmDesc  *next, *prev;           // List pointers
  char    HName [128];            // Full host name
  char    PArgs [128];            // Program call line (abbreviation)
  char    DTime [32];             // startup date/time
  Long    PId,                    // SMURPH process ID
          RPId,                   // Relay process ID
  Handle;                         // Internal handle for identification
  int     socket;
  SmDesc  ();
  ~SmDesc ();
};

class Template;

class TmpList {
/* ------------------------------------------ */
/* Template list entry for outgoing templates */
/* ------------------------------------------ */
  public:
  TmpList *next;
  Template *tmplt;
  TmpList (Template *t) { tmplt = t; next = NULL; };
};

#include	"../LIB/lib.h"
#include	"../LIB/syshdr.h"

