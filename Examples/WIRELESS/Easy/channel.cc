#ifndef __channel_c__
#define __channel_c__

#include "channel.h"

void ShadowingChannel::setup (
		Long nt,		// The number of transceivers
		double rd,		// Reference distance
		double lo,		// Loss at reference distance
		double ex,		// Loss exponent
		double si,		// Sigma
		double no,		// Background noise
		double (*stb) (double),	// SNR to BER converter
		double bu,		// Activity signal threshold
		double co,		// Cut-off signal threshold
		int bpb,		// Bits per byte
		int frm,		// Packet frame length
		Long mpr		// Minimum received preamble length
	) {
		RFChannel::setup (nt);
		BNoise = dBToLin (no);
		Beta = ex;
		RDist = rd;
		Sigma = si;
		LORD = dBToLin (lo);
		AThrs = dBToLin (bu);
		COSL = dBToLin (co);

		BitsPerByte = bpb;
		FrameLength = frm;
		MinPr = mpr;

		ber = stb;
}

double ShadowingChannel::RFC_att (double xp, double d,
					Transceiver *src, Transceiver *dst) {

	if (d < RDist)
		d = RDist;

	// Note: for illustration, this formula is not optimized:
	// Received power = xp * LORD * Gauss (0.0, Sigma) * (d/RDist) ^ -Beta
	return (xp * dBToLin (dRndGauss (0.0, Sigma))) /
		(LORD * pow (d / RDist, Beta));
}

Boolean ShadowingChannel::RFC_act (double sl, double rs) {

	return sl * rs >= AThrs;
}

TIME ShadowingChannel::RFC_xmt (RATE r, Long tl) {

	assert ((tl & 0x7) == 0, "RFC_xmt: packet length %d not divisible by 8",
		tl);

	return (TIME) r * (LONG) ((tl >> 3) * BitsPerByte + FrameLength);
}

double ShadowingChannel::RFC_cut (double xp, double rp) {

pow ((rp * xp) / (COSL * LORD), 1.0 / Beta) * RDist;

	return pow ((rp * xp) / (COSL * LORD), 1.0 / Beta) * RDist;
}

Boolean ShadowingChannel::RFC_bot (RATE r, double sl, double rs,
							const IHist *h) {

	return (h->bits (r) >= MinPr) && !error (r, sl, rs, h, -1, MinPr);
		return NO;
}

Boolean ShadowingChannel::RFC_eot (RATE r, double sl, double rs,
							const IHist *h) {

	return TheTransceiver->isFollowed (ThePacket);
}

Long ShadowingChannel::RFC_erb (RATE tr, double sl, double rs, double ir,
								Long nb) {
	return lRndBinomial (ber (sl/(ir + BNoise)), nb);
}

Long ShadowingChannel::RFC_erd (RATE tr, double sl, double rs, double ir,
								Long nb) {
	double er;

	er = dRndPoisson (1.0 / ber ((sl * rs) / (ir + BNoise)));
	return (er > (double) MAX_Long) ? MAX_Long : (Long) er;
}

#endif