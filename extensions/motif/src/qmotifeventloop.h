#ifndef QMOTIFEVENTLOOP_H
#define QMOTIFEVENTLOOP_H

#include <qeventloop.h>

#include <X11/Intrinsic.h>


class QMotif;
class QMotifEventLoopPrivate;

class QMotifEventLoop : public QEventLoop
{
    Q_OBJECT

public:
    QMotifEventLoop( QMotif *motif, QObject *parent, const char *name = 0 );
    virtual ~QMotifEventLoop();

    void registerSocketNotifier( QSocketNotifier * );
    void unregisterSocketNotifier( QSocketNotifier * );

protected:
    virtual bool processNextEvent( ProcessEventsFlags flags, bool canWait );

private:
    QMotifEventLoopPrivate *d;

    friend void qmotif_socknot_handler( XtPointer, int *, XtInputId * );
    friend void qmotif_timeout_handler( XtPointer, XtIntervalId * );
};

#endif // QMOTIFEVENTLOOP_H
