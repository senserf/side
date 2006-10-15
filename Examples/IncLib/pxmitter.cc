#ifndef __ptransmitter_c__
#define __ptransmitter_c__

// This is the code of the transmitter process shared by the two versions of
// Piggyback Ethernet.

#include "pxmitter.h"

int  MinIPL,      // Minimum inflated packet length (frame excluded)
     MinUPL,      // Minimum uninflated packet length
     MaxUPL,      // Maximum packet length (payload)
     PFrame;      // Frame length

TIME TPSpace,     // After a collision
     TJamL;

RATE TRate;

Long DelayQuantum;

DISTANCE L;       // Actual bus length

PTransmitter::perform {
  state NPacket:
    if (S->ready (MinUPL, MaxUPL, PFrame)) {
      CCounter = 0;
      proceed WaitBus;
    } else
      Client->wait (ARRIVAL, NPacket);
  state WaitBus:
    if (S->Blocked)
      Ready->wait (NEWITEM, Piggyback);
    else {
      inflate ();
      setPiggyDirection ();
      Bus->transmit (Buffer, XDone);
      Bus->wait (COLLISION, Abort);
    }
  state Piggyback:
    if (S->Blocked) {
      deflate ();
      Bus->wait (COLLISION, Error);
      // Note: in a real implementation this would be detected by the
      // transmitter rather than observer
    } else {
      inflate ();
      Bus->wait (COLLISION, Abort);
    }
    setPiggyDirection ();
    Bus->transmit (Buffer, XDone);
  state XDone:
    Bus->stop ();
    Buffer->release ();
    proceed NPacket;
  state Abort:
    Bus->abort ();
    Bus->sendJam (TJamL, EndJam);
  state EndJam:
    Bus->stop ();
    Timer->wait (backoff (), WaitBus);
  state Error:
    excptn ("Illegal collision for a piggybacked packet");
};

void PTransmitter::setup () {
  Bus = S->Bus;
  Buffer = &(S->Buffer);
  Ready = S->Ready;
};

TIME PTransmitter::backoff () {
  return ((TIME) (L+L)) * toss (1 << (CCounter > 10 ? 10 : CCounter));
};

void PTransmitter::inflate () {
  Buffer->TLength = Buffer->ILength + PFrame;
  if (Buffer->ILength < MinIPL) Buffer->TLength += (MinIPL - Buffer->ILength);
};

void PTransmitter::deflate () {
  Buffer->TLength = Buffer->ILength + PFrame;
};

#endif
