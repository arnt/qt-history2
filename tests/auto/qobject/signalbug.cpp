
#include "signalbug.h"

#include <qcoreapplication.h>
#include <qstring.h>


static int Step = 0;
Sender RandomSender (0, 0);


void TRACE (int step, const char *name)
{
	for (int t = 0; t < step - 1; t++)
		fprintf (stderr, "\t");
	fprintf (stderr, "Step %d: %s\n", step, name);
	return;
}


Receiver::Receiver ()
	: QObject ()
{
}

void Receiver::received ()
{
	::Step++;
	const int stepCopy = ::Step;
	TRACE (stepCopy, "Receiver::received()");
	Q_ASSERT (::Step == 2 || ::Step == 4);

	if (::Step == 2)
		s->fire ();

	fprintf (stderr, "Receiver<%s>::received() sender=%s\n",
		(const char *) objectName ().toAscii (), sender ()->metaObject()->className());

	TRACE (stepCopy, "ends Receiver::received()");
}


Disconnector::Disconnector ()
	: QObject ()
{
}

void Disconnector::received ()
{
	::Step++;
	const int stepCopy = ::Step;
	TRACE (stepCopy, "Disconnector::received()");
	Q_ASSERT (::Step == 5 || ::Step == 6);

	fprintf (stderr, "Disconnector<%s>::received() sender=%s\n",
		(const char *) objectName ().toAscii (), sender ()->metaObject()->className());
	if (sender () == 0)
		fprintf (stderr, "WE SHOULD NOT BE RECEIVING THIS SIGNAL\n");

	if (::Step == 5)
	{
		disconnect (s, SIGNAL (fired ()), s->d, SLOT (received ()));

		connect (&RandomSender, SIGNAL (fired ()), s->d, SLOT (received ()));
	}

	TRACE (stepCopy, "ends Disconnector::received()");
}


Sender::Sender (Receiver *r, Disconnector *d)
	: QObject ()
{
	this->r = r; this->d = d;
	if (r)
		connect (this, SIGNAL (fired ()), r, SLOT (received ()));
	if (d)
		connect (this, SIGNAL (fired ()), d, SLOT (received ()));
};

void Sender::fire ()
{
	::Step++;
	const int stepCopy = ::Step;
	TRACE (stepCopy, "Sender::fire()");
	Q_ASSERT (::Step == 1 || ::Step == 3);

	emit fired ();
	TRACE (stepCopy, "ends Sender::fire()");
}


int main (int argc, char *argv [])
{
	QCoreApplication app (argc, argv);

	Receiver r;
	Disconnector d;
	Sender s (&r, &d);

	r.s = &s;
	d.s = &s;


	::Step = 0;
	s.fire ();
	return 0;
}


