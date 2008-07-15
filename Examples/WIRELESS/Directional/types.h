#ifndef __types_h__
#define	__types_h__

// HELLO packet types. Note that in SMURPH negative types are used for special
// packets, i.e., ones that are not generated by the traffic generator, but
// internally by the protocol. There are two packet types: HELLO and DATA. The
// type of DATA packets is zero, but more types (and a more elaborated traffic
// generator) can be immediatelly accommodated, as we never assume that type 0
// is all there is.
#define	PKT_TYPE_HELLO		(-1)

// This is the (logical) length of the common packet header for all (both)
// packet types (see WDPacket below). This may correspond to Source Address,
// Destination Address, MAC-level Sender (we need no explicit MAC-level
// receiver). This may be: MAC-sender address, transport sender address,
// TTL, and serial number.
#define	PKT_LENGTH_WD		(32+32+16+16)

// This is the extra length for a HELLO packets: two coordinates of the sending
// node.
#define	PKT_LENGTH_HELLO	(PKT_LENGTH_WD + 32 + 32)

// And here is the header length for a data packet. The new item represents the
// destination address.
#define	PKT_LENGTH_DATA		(PKT_LENGTH_WD + 32)

#define	PKT_TTL_DATA		18	// Initial TTL settings for data packets
#define	PKT_TTL_HELLO		10	// ... and for HELLO

// Tracing options: switch them on (set to 1) to see various traces in the
// output file.
#define	TRACE_HELLO		0
#define	TRACE_DATA		0
#define	TRACE_ROUTING		0
#define	TRACE_RTABLES		0
#define	TRACE_ANGLES		0

#if TRACE_HELLO
#define	trc_h(p,a)	do { \
			 if ((p)->TP == PKT_TYPE_HELLO) \
			  trace ("HE AT %1d, S %1d [%1d], TTL: %1d, SN %1d: " \
			   a, TheStation->getId (), (p)->Sender, (p)->SA, \
			    (p)->TTL, (p)->SN); \
			} while (0)
#else	
#define	trc_h(p,a)	do { } while (0)
#endif

#if TRACE_DATA
#define	trc_d(p,a)	do { \
			 if ((p)->TP != PKT_TYPE_HELLO) \
			  trace ("DA AT %1d, S %1d [%1d], R %1d, L %1d " \
			   "(%1d), TTL %1d, SN %1d: " a, getId (), \
			    (p)->Sender, (p)->SA, (p)->Receiver, (p)->ILength, \
			     (p)->TLength, (p)->TTL, (p)->SN); \
			} while (0)
#else	
#define	trc_d(p,a)	do { } while (0)
#endif

#if TRACE_RTABLES
#define	trc_t(rt,a)	do { (rt)->dump (a); } while (0)
#else
#define	trc_t(rt,a)	do { } while (0)
#endif

#if TRACE_ANGLES
#define trc_a(n,a)	trace ("ANGLE: %f [%f deg]: " a, n, ((n)/M_PI)*180.0)
#else
#define trc_a(n,a)	do { } while (0)
#endif

#if TRACE_ROUTING
#define	trc_r(n,a)	trace ("ROUTED TO %1d: ", a, n)
#else
#define	trc_r(n,a)	do { } while (0)
#endif

packet WDPacket {

	// This is the common component of all our packet types. Note that
	// all packets have some standard attributes (which are not mentioned
	// here). For example Sender is the transport sender of the packet.

	Long 	SA;		// MAC-level sender
	Long	SN;		// Serial number for duplicate detection
	int	TTL;		// Time to live

	// Antenna setting. Strictly speaking, this shouldn't belong to the
	// packet, but perhaps to some "packet buffer" encapsulating the packet
	// for transmission. What I am saying is that this is only needed
	// by the transmitter when the packet is about to be sent and the
	// antenna must be properly oriented. Thus we don't include this
	// item in the formal length of the header (PKT_LENGTH_WD).
	float	Direction;
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

mailbox	PQueue (Packet*) {

	// Queue of output packets to transmit. Its size is limited (see 
	// Node::setup in node.cc where the queue is created).

	Boolean queue (Packet *t) {
		if (free ()) {
			put (t);
#if TRACE_ROUTING
			trace ("QUEUED");
#endif
			return YES;
		}
		delete t;
#if TRACE_ROUTING
		trace ("QUEUE FULL!!");
#endif
		return NO;
	};
};

mailbox Barrier {

	// Mailbox for passing events among node processes. Used by the
	// xmitter and receiver processes (rfmodule.cc) to exchange signals.

	void setup () {
		// This is the way to say that mailbox acts as a barrier
		setLimit (-1);
	};
};

struct RTE_s {

	// Routing table entry: describes the wherebouts of a single node

	struct RTE_s *prev, *next;	// Needed by the pooling operations

	TIME TStamp;			// Time stamp (when last set or updated)
	Long A;				// Node address
	double X, Y;			// Coordinates
};

typedef	struct RTE_s RTE_i;

class RTable {

	// The oouting table. This may be too grandiose a word for the present
	// version, but I am sure you will turn this into something more
	// impresive.

	RTE_i *Head;

	public:

	// Look up a node and get its coordinates
	Boolean getCoords (Long, double&, double&);

	// Add/update node coordinates
	void update (Long, double, double);

	// Find the best neighbor towards which the packet should be sent
	Long route (double, double, double, double&);

	// Delete obsolte entries. Only used for the Neighbors pool.
	void deleteOld (TIME);

#if TRACE_RTABLES

	void dump (const char *a) {
		RTE_i *pe;
		trace ("RTable %s at %1d", a, TheStation->getId ());
		for_pool (pe, Head)
			trace ("  %1d %f %f [TS = %f]", pe->A, pe->X, pe->Y,
				ituToEtu (pe->TStamp));
	};
#endif

	RTable () {
		// Starts as empty
		Head = NULL;
	};
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

	Barrier *Event;		// For node events

	DataPacket Buffer;	// For outgoing session packets

	Transceiver *RFI;	// The RF interface

	PQueue *PQ;		// The outgoing packet queue

	RTable	*Neighbors,	// The neighbor pool
		*NetMap;	// Node coordinate database

	PCache 	*PC;		// Packet cache for duplicate rejection

	Long	Serial;		// Serial number for outgoing packets

	Boolean RFBusy;		// RFI busy flag

	void receive (WDPacket*);
	void dispatch ();

	void setup (RATE);
};

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

void initNodes (Long);
void initMobility ();

extern Long BitRate;

#endif
