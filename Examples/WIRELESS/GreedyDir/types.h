#ifndef __types_h__
#define	__types_h__

#define	DIRECTIONAL_DATA	1
#define	DIRECTIONAL_RTS		1
#define	DIRECTIONAL_CTS		1
#define	DIRECTIONAL_ACK		1
#define	DIRECTIONAL_RECEPTION	1

// Tracing options: switch them on (set to 1) to see various traces in the
// output file.
#define	TRACE_PACKETS		0
#define	TRACE_ROUTES		0
#define	TRACE_ANGLES		0
#define	TRACE_XEVENTS		0

#define	DEBUGGING		0	// This one is for the RF module

#include "angles.h"
#include "rfmod_dcf.h"

// HELLO packet types. Note that in SMURPH negative types are used for special
// packets, i.e., ones that are not generated by the traffic generator, but
// internally by the protocol. There are two packet types: HELLO and DATA. The
// type of DATA packets is zero, but more types (and a more elaborated traffic
// generator) can be immediatelly accommodated, as we never assume that type 0
// is all there is.
#define	PKT_TYPE_HELLO		(-1)

// Note: the RF module uses an internal notion of packet type, which is
// different, e.g., from its point of view both HELLO and DATA are "data"
// packets, and RTS, CTS, ACK are special packets tagged with the appropriate
// "internal" type attribute.

// HELLO packet: need the address of original sender, but no recipient address.
// On top of that, we have 8 bytes for the two coordinates + 2 bytes for a
// serial number + 2 bytes for TTL. Note that we use PKT_LENGTH_RTS as a
// reference (i.e., minimum packet length).
#define	PKT_LENGTH_HELLO	(PKT_LENGTH_RTS + (2 + 2 + 4 + 4) * 8)

// A DATA packet has two more addresses (original sender + target recipient) +
// TTL + some serial number (or whatever). Lots of framing, isn't it?
#define	PKT_LENGTH_DATA		(PKT_LENGTH_RTS + (6 + 6 + 2 + 2) * 8)

#define	PKT_TTL_DATA		15	// Initial TTL settings for data packets
#define	PKT_TTL_HELLO		10	// ... and for HELLO

// Number of re-forwarding attempts if the packet is dropped by the MAC layer
#define	MAX_ROUTE_TRIES		3

// A small distance used as EPSILON
#define	TINY_DISTANCE		0.01	// This is 1 cm

packet WDPacket : DCFPacket {

	// This is the common component of all our packet types. Note that
	// all packets have some standard attributes (which are not mentioned
	// here). For example Sender is the transport sender of the packet.

	Long	SN;		// Serial number for eliminating duplicates
	unsigned short TTL;	// Time to live
	unsigned short Retries;	// Number of re-routing attempts at last node
};

packet HelloPacket : WDPacket {

	// These packets are sent periodically by nodes to announce their
	// coordinates. They essentially flood the network. The standard
	// Sender attribute determines the originator.

	double	X, Y;

	void setup ();
};

packet DataPacket : WDPacket {

	// Data packet: nothing special here. The standard Receiver attribute
	// determines the final destination.

	void setup (Message*);

};

station Node;

class MY_RFModule : public RFModule {

// This class completes the specification of RFModule, which is an abstract
// library class. Some virtual methods that can be declared here provide
// hooks for various occasions, which, in particular, may be useful for
// implementing directional transmission/reception. I am listing (defining)
// them all for illustration; only those that are nonempty or different from
// the default actions need be defined.

	// Antenna setting: we assume that the setting is always described
	// by an absolute (and exact) angle, while, possibly, only a limited
	// number of discrete settings may be available.
	// This will give us most flexibility as the gain assessment can be
	// made independent of the actual policy for selecting the discrete
	// settings.
	// The angle is always nonnegative, measured from East == 0,
	// conterclockwise upto 2 PI; thus, North is PI/2, West is PI and
	// South is 3PI/2.
	// A negative value for an antenna angle means "omni".

	double	R_Ant,		// Receiver antenna setting
		X_Ant;		// Xmitter antenna setting

	Node	*MyNode;

	public:

	// These methods set the antennas based on the node (Id)
	// towards which they should be pointing. The methods do not rely
	// on absolute knowledge, but look up the node in the routing
	// database. This way, you need not use the exact coordinates,
	// but whatever coordinate info you decide to store in the routing
	// tables.
	void setXAnt (Long);
	void setRAnt (Long);

	// These ones retrieve antenna settings, e.g., to be resumed later
	// (at a lower cost, without having to go thorugh angle recalculations)
	inline double getXAnt () { return X_Ant; };
	inline double getRAnt () { return R_Ant; };

	// These ones re-set the antennas from saved values
	inline void setXAnt (double v) { X_Ant = v; };
	inline void setRAnt (double v) { R_Ant = v; };

	// An a couple more (see channel.c for the actual code)
	void setXAntOmni ();
	void setRAntOmni ();

	// --- rcv group ---

	void UPPER_rcv_data (DCFPacket*, double);
		// This method MUST be defined; it is called whenever a packet
		// has been received by the MAC layer; the second argument is
		// the RSSI. This is the only method that has no default
		// action.

	void UPPER_rcv_rts (Long snd, double rssi) {
		// Called immediately after RTS reception, with the first
		// argument identifying the sender; the second argument is
		// the RSSI at which the RTS has been received
#if TRACE_XEVENTS
		trace ("RCV RTS: %1d <- %1d", TheStation->getId (), snd);
#endif
	};

	void UPPER_rcv_cts (Long snd, double rssi) {
		// Called after (a solicited) CTS reception
#if TRACE_XEVENTS
		trace ("RCV CTS: %1d <- %1d", TheStation->getId (), snd);
#endif
	};

	void UPPER_rcv_ack (Long snd, double rssi) {
		// Called after a solicited ACK reception
#if TRACE_XEVENTS
		trace ("RCV ACK: %1d <- %1d", TheStation->getId (), snd);
#endif
#if DIRECTIONAL_RECEPTION
		// ACK is the last message of an exchange, so when we are done,
		// we reset the RCV antenna to OMNI
		setRAntOmni ();
#endif
	};

	// --- snd group ---

	void UPPER_snd_data (DCFPacket *p) {
		// Called when the RF module is about to start transmitting
		// a data packet
#if TRACE_XEVENTS
		trace ("SND DATA: %1d -> %1d", TheStation->getId (), p->DCFP_R);
#endif
#if DIRECTIONAL_DATA
		if (flagSet (p->DCFP_Flags, DCFP_FLAG_BCST))
			// Broadcast data packets are sent with OMNI XMT antenna
			// setting
			setXAntOmni ();
		else
			// Set the antenna towards the recipient
			setXAnt (p->DCFP_R);
#else
		// This is when DATA is sent omnidirectionally
		setXAntOmni ();
#endif
	};

	void UPPER_snd_rts (Long rcp) {
		// Called when the RF module is about to start transmitting
		// an RTS packet; the argument identifies the recipient
#if TRACE_XEVENTS
		trace ("SND RTS: %1d -> %1d", TheStation->getId (), rcp);
#endif
#if DIRECTIONAL_RTS
		setXAnt (rcp);
#else
		setXAntOmni ();
#endif
	};

	void UPPER_snd_cts (Long rcp) {
		// Called when the RF module is about to start transmitting
		// a CTS packet; the argument identifies the recipient
#if TRACE_XEVENTS
		trace ("SND CTS: %1d -> %1d", TheStation->getId (), rcp);
#endif
#if DIRECTIONAL_CTS
		setXAnt (rcp);
#else
		setXAntOmni ();
#endif
	};

	void UPPER_snd_ack (Long rcp) {
		// Called when the RF module is about to start transmitting
		// an ACK packet; the argument identifies the recipient
#if TRACE_XEVENTS
		trace ("SND ACK: %1d -> %1d", TheStation->getId (), rcp);
#endif
#if DIRECTIONAL_ACK
		setXAnt (rcp);
#else
		setXAntOmni ();
#endif
	};

	// --- snt group ---

	void UPPER_snt_data (DCFPacket *p) {
		// Called immediately after the RF module stops sending a data
		// packet; the packet is pointed to by the argument

#if TRACE_XEVENTS
		trace ("SNT DATA: %1d -> %1d", TheStation->getId (), p->DCFP_S);
#endif

#if DIRECTIONAL_RECEPTION
		// This can be conditional on short data packets as otherwise
		// the RAnt will have been set by snt_rts. If the packet is
		// broadcast, it was sent with OMNI and no ACK is expected, so
		// we need not worry about receiving any feedback. Otherwise,
		// we would like to point the RCV antenna for the ACK. But that
		// has been done already after RTS, unless the DATA packet
		// was below the RTS threshold (and there was no RTS).
		if (!flagSet (p->DCFP_Flags, DCFP_FLAG_BCST) && p->TLength <=
		    DCF_RTS_ths)
			setRAnt (p->DCFP_R);
#endif
	};

	void UPPER_snt_rts (Long rcp) {
		// Called immediately after the RF module stops sending an RTS
		// packet to the node identified by the argument
#if TRACE_XEVENTS
		trace ("SNT RTS: %1d -> %1d", TheStation->getId (), rcp);
#endif
#if DIRECTIONAL_RECEPTION
		// Having sent the RTS, point the RCV antenna towards the 
		// recipient for the CTS
		setRAnt (rcp);
#endif
	};

	void UPPER_snt_cts (Long rcp) {
		// Called immediately after the RF module stops sending a CTS
		// packet to the node identified by the argument
#if TRACE_XEVENTS
		trace ("SNT CTS: %1d -> %1d", TheStation->getId (), rcp);
#endif
#if DIRECTIONAL_RECEPTION
		// Having sent the CTS, point the antenna towards the recipient	
		// for the DATA packet
		setRAnt (rcp);
#endif
	};

	void UPPER_snt_ack (Long rcp) {
		// Called immediately after the RF module stops sending an ACK
		// packet to the node identified by the argument
#if TRACE_XEVENTS
		trace ("SNT ACK: %1d -> %1d", TheStation->getId (), rcp);
#endif
		// No directional settings after ACK - nothing more is
		// expected
	};

	// --- suc / fai

	// See node.cc for the code of these two methods; they have no
	// direction-related responsibilities

	void UPPER_suc_data (DCFPacket*, int, int);
		// Called whenever a data packet is successfully transmitted;
		// for a broadcast data packet, this means just transmitted
		// (the method is called along with UPPER_snt_data; for a
		// non-broadcast data packet, the method is called when the ACK
		// is receieved; the last two arguments give the values of the
		// short and long retransmission counters, respectively; they
		// are always both zeros for a broadcast packet.
		// The default method deletes the packet; note that it also
		// must be explicitly deleted (at some point) by any local
		// variant

	void UPPER_fai_data (DCFPacket*, int, int);
		// Called when the transmission of a data packet fails, i.e.,
		// one of the retransmission counters runs out; never called
		// for a broadcast packet; the last two arguments as for 
		// UPPER_suc_data; the failure occurs when the expected ACK
		// packet times out after the last allowed retransmission;
		// The default method deletes the packet; note that it also
		// must be explicitly deleted (at some point) by any local
		// variant

	// --- the col group

	void UPPER_col_data () {
		// Called when the data packet fails to arrive (timeout) after
		// CTS
#if TRACE_XEVENTS
		trace ("COL DATA: %1d", TheStation->getId ());
#endif
#if DIRECTIONAL_RECEPTION
		// DATA was expected, but didn't make it; should reset the
		// RCV antenna to OMNI
		setRAntOmni ();
#endif
	};

	void UPPER_col_cts (int sh, int ln) {
		// Called when CTS fails to arrive after RTS
#if TRACE_XEVENTS
		trace ("COL CTS: %1d", TheStation->getId ());
#endif
#if DIRECTIONAL_RECEPTION
		// CTS was expected, but didn't make it: reset the RCV antenna
		setRAntOmni ();
#endif
	};

	void UPPER_col_ack (int sh, int ln) {
		// Called when ACK fails to arrive after DATA
#if TRACE_XEVENTS
		trace ("COL ACK: %1d", TheStation->getId ());
#endif
#if DIRECTIONAL_RECEPTION
		// ACK was expected but didn't make it: reset the RCV antenna
		// as after receiving the ACK
		setRAntOmni ();
#endif
	};

	// --- end of redefineable methods ----

	MY_RFModule (Transceiver *t, Long pqs) : RFModule (t, pqs) {
		MyNode = (Node*) TheStation;
	};

};

struct RTE_s {

	// Routing table entry: describes the location of one prospective
	// destination

	struct RTE_s *prev, *next;	// Needed by the pooling operations

	Long A;				// Node address
	double X, Y;			// Coordinates
};

typedef	struct RTE_s RTE_i;

class RTable {

	// The routing table, i.e., the list of destinations we know about

	RTE_i *Head;

	public:

	// Look up a node and get its coordinates
	Boolean getCoords (Long, double&, double&);

	// Add/update destination info
	void update (Long, double, double);

	RTable () {
		// Starts as empty
		Head = NULL;
	};
};

struct NTE_s {

	// Neighbor table entry: describes the parameters of one neighbor

	struct NTE_s *prev, *next;	// Needed by the pooling operations

	TIME TStamp;			// Time stamp (when last set or updated)
	Long A;				// Node address
	double X, Y;			// Coordinates
	double RSSI;			// Signal strength of last HELLO
};

typedef	struct NTE_s NTE_i;

class NTable {

	// The neighbor table - describing the population of currently known
	// neighbors

	friend class Node;

	NTE_i *Head;
	Long NN;

	double RelRSSI,			// Reliable RSSI threshold
	       AvgRSSI,			// Average RSSI
	       MinRSSI,			// Minimum RSSI
	       MaxRSSI;			// Minimum RSSI

	// We implement a somewhat exotic scheme for re-routing (i.e., trying
	// different neighbors) after a forwarding failure, for which we need
	// the above statistics. This is explained in node.cc.

	public:

	// Add/update neighbor info
	void update (Long, double, double, double rssi);

	// Look up a given neighbor and get its coordinates
	Boolean getCoords (Long, double&, double&);

	// Called when routing through a neighbor fails; the argument is the
	// neighbor Id; its role is to update RelRSSI, i.e., the reliable RSSI
	// threshold
	void unreliable (Long);

	// Delete obsolete entries
	void deleteOld (TIME);

	NTable () {
		// Starts as empty
		Head = NULL;
		NN = 0;
		AvgRSSI = 0.0;
		MinRSSI = HUGE;
		RelRSSI = MaxRSSI = -HUGE;
	};
};

inline double ema (double ol, double ne, double alpha) {
// Exponential moving average
	return (ne * alpha + ol * (1.0 - alpha));
};

struct PCache_s {

	// A packet cache item. This is what we store (for a limited time) for
	// every received (and possibly forwarded) packet: Sender (the
	// originator) address, and the serial number.

	struct PCache_s *prev, *next;

	Long Sender,			// The sender
	     SN;			// Serial number
};

typedef struct PCache_s PCache_i;

class PCache {

	// Here is the actual cache

	PCache_i *Head;
	PCache_i *Tail;			// Needed to implement FIFO policy

	Long MaxSize;

	public:

	Boolean add (WDPacket*);

	PCache (Long m) {
		Head = Tail = NULL;
		MaxSize = m;
	};
};

station Node {

	DataPacket Buffer;	// For outgoing session packets
	Transceiver *RFI;	// The RF interface

	MY_RFModule *RFM;

	NTable	*Neighbors;	// The neighbor pool

	RTable 	*NetMap;	// Node coordinate database

	PCache 	*PC;		// Packet cache for duplicate rejection

	Long	Serial;		// Serial number for outgoing packets

	Long route (Long);
	void receive (WDPacket*, double);
	void sent (WDPacket*, int, int);
	void dropped (WDPacket*);
	void dispatch ();

	void setup (RATE, Long);
};

#define	TheNode	((Node*)TheStation)

process HelloSender (Node) {

	// See node.cc for the code method

	double MinInt, MaxInt;

	states { Delay, SendIt };

	TIME interval () {

		return etuToItu (dRndUniform (MinInt, MaxInt));
	};

	void setup (double mi, double ma) {
		MinInt = mi;
		MaxInt = ma;
	};

	perform;
};

process NeighborCleaner (Node) {

	// See node.cc for the code method

	TIME Interval, ExpirationTime;

	states { Delay, CleanIt };

	void setup (double hi, double ex) {

		Interval = etuToItu (hi);
		ExpirationTime = etuToItu (ex);
	};

	perform;
};

process DataSender (Node) {

	// See node.cc for the code method

	states { WMess } ;

	perform;
};

process PositionReporter {

	// See node.cc for the code method

	states { Report } ;

	perform;
};

void initNodes (Long, Long);
void initMobility ();
void initChannel (Long&, Long&);

#endif
