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

protected:
    virtual bool processNextEvent( int eventType, bool canWait );

private:
    QMotifEventLoopPrivate *d;

    friend void qmotif_timeout_handler( XtPointer, XtIntervalId * );
};

#endif // QMOTIFEVENTLOOP_H
