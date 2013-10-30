#ifndef	__wchannt_c__
#define	__wchannt_c__

#include "wchannt.h"
#include "wchan.cc"

#undef trc
//#define	trc(a, ...)	trace (a, ## __VA_ARGS__)

#ifndef trc
#define	trc(a, ...)
#endif


void RFNeutrino::setup (

	Long nt,		// The number of transceivers
	double ran,		// Range
	int bpb,		// Bits per byte
	int frm,		// Packet frame (extra physical bits)
	IVMapper **ivcc,	// Value converters
	MXChannels *mxc
) {
	RadioChannel::setup (nt, -HUGE, NULL, 0, bpb, frm, ivcc, mxc);
	Range = ran;

	print (Range, 		"  Range:", 10, 26);
}

// ============================================================================

double RFNeutrino::RFC_att (const SLEntry *xp, double d, Transceiver *src) {

	if (tagToCh (xp->Tag) != tagToCh (TheTransceiver->getTag ()) ||
	    d >= Range)
		// Different channels or out of range
		return 0.0;

	return 1.0;
}

Boolean RFNeutrino::RFC_act (double sl, const SLEntry *sn) {

	return NO;
}

double RFNeutrino::RFC_cut (double xp, double rp) {

	return Range;
}

Long RFNeutrino::RFC_erb (RATE tr, const SLEntry *sl, const SLEntry *rs,
							double ir, Long nb) {
	return 0;
}

Long RFNeutrino::RFC_erd (RATE tr, const SLEntry *sl, const SLEntry *rs,
	double ir, Long nb) {

	return MAX_Long;
}

Boolean RFNeutrino::RFC_bot (RATE r, const SLEntry *sl, const SLEntry *sn,
	const IHist *h) {

	return YES;
}

Boolean RFNeutrino::RFC_eot (RATE r, const SLEntry *sl, const SLEntry *sn,
	const IHist *h) {

	return YES;
}

#endif
