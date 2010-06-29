identify "Alternating Bit";

packet PacketType { int SequenceBit; };

packet AckType { int SequenceBit; };

station SenderType {

	PacketType PacketBuffer;
	Port *IncomingPort, *OutgoingPort;
	Mailbox *AlertMailbox;
	int LastSent;
	void setup ();
};

station RecipientType {

	AckType AckBuffer;
	Port *IncomingPort, *OutgoingPort;
	Mailbox *AlertMailbox;
	int Expected;
	void setup ();
};

traffic TrafficType (Message, PacketType) { };

process TransmitterType (SenderType) {

	Port *Channel;
	PacketType *Buffer;
	Mailbox *Alert;
	TIME Timeout;
	states { NextPacket, Retransmit, EndXmit, Acked };
	void setup (TIME);
	perform;
};

process AckReceiverType (SenderType) {

	Port *Channel;
	Mailbox *Alert;
	states { WaitAck, AckBegin, AckArrival, Collision };
	void setup ();
	perform;
};

process ReceiverType (RecipientType) {

	Port *Channel;
	Mailbox *Alert;
	TIME Timeout;
	states { WaitPacket, BeginPacket, PacketArrival, ReAck, Collision };
	void setup (TIME);
	perform;
};

process AcknowledgerType (RecipientType) {

	Port *Channel;
	AckType *Ack;
	Mailbox *Alert;
	states { WaitAlert, SendAck, ReAck, EndXmit };
	void setup ();
	perform;
};

process Root {

	void readData (), buildNetwork (), defineTraffic (), startProtocol ();
	void printResults ();
	states { Start, Stop };
	perform;
};

int AckLength;
SenderType *Sender;
RecipientType *Recipient;
TIME TransmissionRate, SenderTimeout, RecipientTimeout, Distance;
Link *SenderToRecipient, *RecipientToSender;
double MessageLength, MeanMessageInterarrivalTime, FaultRate;
long MessageNumberLimit;

void SenderType::setup () {

	IncomingPort = create Port;
	OutgoingPort = create Port (TransmissionRate);
	AlertMailbox = create Mailbox (1);
	LastSent = 0;
};

void RecipientType::setup () {

	IncomingPort = create Port;
	OutgoingPort = create Port (TransmissionRate);
	AlertMailbox = create Mailbox (1);
	AckBuffer.fill (this, Sender, AckLength);
	Expected = 0;
};

void TransmitterType::setup (TIME tmout) {

	Channel = S->OutgoingPort;
	Buffer = &(S->PacketBuffer);
	Alert = S->AlertMailbox;
	Timeout = tmout;
};

void AckReceiverType::setup () {

	Channel = S->IncomingPort;
	Alert = S->AlertMailbox;
};

void ReceiverType::setup (TIME tmout) {

	Channel = S->IncomingPort;
	Alert = S->AlertMailbox;
	Timeout = tmout;
};

void AcknowledgerType::setup () {

	Channel = S->OutgoingPort;
	Ack = &(S->AckBuffer);
	Alert = S->AlertMailbox;
};

TransmitterType::perform {

	state NextPacket:

		if (Client->getPacket (Buffer)) {
			Buffer->SequenceBit = S->LastSent;
			proceed Retransmit;
		} else
			Client->wait (ARRIVAL, NextPacket);

	state Retransmit:

		if (Channel->busy ()) {
			Channel->wait (SILENCE, Retransmit);
			sleep;
		}
		Channel->transmit (Buffer, EndXmit);

	state EndXmit:

		Channel->stop ();
		Alert->wait (RECEIVE, Acked);
		Timer->wait (Timeout, Retransmit);

	state Acked:

		Buffer->release ();
		S->LastSent = 1 - S->LastSent;
		proceed NextPacket;
};

AckReceiverType::perform {

	state WaitAck:

		Channel->wait (BMP, AckBegin);

	state AckBegin:

		Channel->wait (EMP, AckArrival);
		Channel->wait (COLLISION, Collision);

	state AckArrival:

		if (((AckType*)ThePacket)->SequenceBit == S->LastSent)
			Alert->put ();
		skipto WaitAck;

	state Collision:

		skipto WaitAck;

};

ReceiverType::perform {

	state WaitPacket:

		Channel->wait (BMP, BeginPacket);
		Timer->wait (Timeout, ReAck);

	state BeginPacket:

		Channel->wait (COLLISION, Collision);
		Channel->wait (EMP, PacketArrival);

	state PacketArrival:

		if (((PacketType*)ThePacket)->SequenceBit == S->Expected) {
			Client->receive (ThePacket, Channel);
			S->Expected = 1 - S->Expected;
		}
		Alert->put ();
		skipto WaitPacket;

	state ReAck:

		Alert->put ();
		skipto WaitPacket;

	state Collision:

		skipto WaitPacket;
};

AcknowledgerType::perform {

	state WaitAlert:

		Alert->wait (RECEIVE, SendAck);

	state SendAck:

		Ack->SequenceBit = 1 - S->Expected;
		proceed ReAck;

	state ReAck:

		if (Channel->busy ()) {
			Channel->wait (SILENCE, ReAck);
			sleep;
		}

		Channel->transmit (Ack, EndXmit);

	state EndXmit:

		Channel->stop ();
		proceed WaitAlert;

};

Root::perform {

	state Start:

		readData ();
		buildNetwork ();
		defineTraffic ();
		startProtocol ();
		Kernel->wait (DEATH, Stop);

	state Stop:

		printResults ();
};

void Root::readData () {

	readIn (AckLength);
	readIn (TransmissionRate);
	readIn (FaultRate);
	readIn (Distance);
	readIn (SenderTimeout);
	readIn (RecipientTimeout);
	readIn (MessageLength);
	readIn (MeanMessageInterarrivalTime);
	readIn (MessageNumberLimit);

};

void Root::buildNetwork () {

	Port *from, *to;
	
	Sender = create SenderType;
	Recipient = create RecipientType;
	SenderToRecipient = create Link (2);
	RecipientToSender = create Link (2);
	(from = Sender->OutgoingPort)->connect (SenderToRecipient);
	(to = Recipient->IncomingPort)->connect (SenderToRecipient);
	from->setDTo (to, Distance);
	RecipientToSender->setFaultRate (FaultRate);
	(from = Recipient->OutgoingPort)->connect (RecipientToSender);
	(to = Sender->IncomingPort)->connect (RecipientToSender);
	from->setDTo (to, Distance);
	SenderToRecipient->setFaultRate (FaultRate);
};

void Root::defineTraffic () {

	TrafficType *tp;

	tp = create TrafficType (MIT_exp + MLE_fix,
	MeanMessageInterarrivalTime, MessageLength);
	tp->addSender (Sender);
	tp->addReceiver (Recipient);
	setLimit (MessageNumberLimit);
};

void Root::startProtocol () {

	create (Sender) TransmitterType (SenderTimeout);
	create (Sender) AckReceiverType;
	create (Recipient) ReceiverType (RecipientTimeout);
	create (Recipient) AcknowledgerType;
};

void Root::printResults () {

	Client->printPfm ();
	SenderToRecipient->printPfm ();
	RecipientToSender->printPfm ();

};
