#ifndef QEVENTDISPATCHER_QWS_H
#define QEVENTDISPATCHER_QWS_H

#include "qeventdispatcher_unix.h"

class QEventDispatcherQWSPrivate;

class QEventDispatcherQWS : public QEventDispatcherUNIX
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QEventDispatcherQWS)

public:
    QEventDispatcherQWS(QObject *parent = 0);
    ~QEventDispatcherQWS();

    bool processEvents(QEventLoop::ProcessEventsFlags flags);
    bool hasPendingEvents();

    void flush();

    void startingUp();
    void closingDown();

protected:
    int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
               timeval *timeout);
};

#endif // QEVENTDISPATCHER_QWS_H
