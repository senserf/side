#define	BUFFSIZE 128

mailbox Device (int);

Device	*Uart, *TTY;

process reader {

	char Buffer [BUFFSIZE];
	int nc;

	states { NewData, WriteOut };

	perform;

};

process writer {

	char Buffer [BUFFSIZE];
	int nc;

	void setup () { TTY->setSentinel ('\n'); };

	states { Start, NewData, WriteOut };

	perform;
};

writer::perform {

	char *ttyname;

	state Start:

		TTY->write ("Enter UART name:\n", 17);

		// Get from the user the COM port name
		if ((nc = TTY->readToSentinel (Buffer, BUFFSIZE)) == 0) {
			TTY->wait (SENTINEL, Start);
			sleep;
		}

		for (nc = 0; Buffer [nc] == ' '; nc++);
		ttyname = Buffer + nc;

		while (!isspace (Buffer [nc]))
			nc++;

		Buffer [nc] = '\0';

		// Try to open it

		if (Uart->connect (DEVICE+RAW+READ+WRITE, ttyname, 0, BUFFSIZE,
			9600, 0, 0) == ERROR) {

			TTY->write ("Cannot open '", 13);
			TTY->write (ttyname, strlen (ttyname));
			TTY->write ("'\n", 2);
			proceed Start;

		}

		TTY->write ("Succeeded\n", 10);

		create reader;

	transient NewData:

		if ((nc = TTY->readAvailable (Buffer, BUFFSIZE)) == 0) {
			TTY->wait (UPDATE, NewData);
			sleep;
		}

	transient WriteOut:

		if (Uart->write (Buffer, nc) != ACCEPTED) {
			Uart->wait (OUTPUT, WriteOut);
			sleep;
		}

		proceed NewData;
}

reader::perform {

	state NewData:

		if ((nc = Uart->readAvailable (Buffer, BUFFSIZE)) == 0) {
			Uart->wait (UPDATE, NewData);
			sleep;
		}

	transient WriteOut:

		if (TTY->write (Buffer, nc) != ACCEPTED) {
			TTY->wait (OUTPUT, WriteOut);
			sleep;
		}

		proceed NewData;
}

process Root {

	states { Start, Stop };

	perform {

		state Start:

			TTY = create Device;
			Uart = create Device;
			if (TTY->connect (DEVICE+READ+WRITE, "/dev/tty", 0,
				BUFFSIZE) == ERROR)
					excptn ("Cannot open tty");

			create writer;

			Kernel->wait (DEATH, Stop);

		state Stop:

			terminate;
	};
};