#ifndef QGUIEVENTLOOP_H
#define QGUIEVENTLOOP_H

#include <qeventloop.h>

class QGuiEventLoop : public QEventLoop
{
    Q_OBJECT
public:
    QGuiEventLoop( QObject *parent = 0, const char *name = 0 );
    ~QGuiEventLoop();

#if defined(Q_WS_X11) || defined(Q_WS_QWS)
    virtual bool processEvents( ProcessEventsFlags flags );
    virtual bool hasPendingEvents() const;
#endif

protected:
#ifdef Q_WS_X11
    virtual void appStartingUp();
    virtual void appClosingDown();
#endif
};


#endif
