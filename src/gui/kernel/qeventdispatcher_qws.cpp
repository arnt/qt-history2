#include "qeventdispatcher_qws_p.h"

#include "qplatformdefs.h"
#include "qapplication.h"
#include "qwscommand_qws.h"
#include "qwsdisplay_qws.h"
#include "qwsevent_qws.h"
#include "qwindowsystem_qws.h"
#include <private/qeventdispatcher_unix_p.h>


#if defined(QT_THREAD_SUPPORT)
#  include "qmutex.h"
#endif // QT_THREAD_SUPPORT

#include <errno.h>

//#define d d_func()
//#define q q_func()

class QEventDispatcherQWSPrivate : public QEventDispatcherUNIXPrivate
{
    Q_DECLARE_PUBLIC(QEventDispatcherQWS)
public:
    inline QEventDispatcherQWSPrivate()
    { }
};


QEventDispatcherQWS::QEventDispatcherQWS(QObject *parent)
    : QEventDispatcherUNIX(*new QEventDispatcherQWSPrivate, parent)
{ }

QEventDispatcherQWS::~QEventDispatcherQWS()
{ }



// from qapplication_qws.cpp
extern QWSDisplay* qt_fbdpy; // QWS `display'

//#define ZERO_FOR_THE_MOMENT

bool QEventDispatcherQWS::processEvents(QEventLoop::ProcessEventsFlags flags)
{
    // process events from the QWS server
    int           nevents = 0;

    // handle gui and posted events
    QApplication::sendPostedEvents();

    while (qt_fbdpy->eventPending()) {        // also flushes output buffer
#ifdef ZERO_FOR_THE_MOMENT
        if (d->shortcut) {
            return false;
        }
#endif
        QWSEvent *event = qt_fbdpy->getEvent();        // get next event
        if (filterEvent(event))
            continue;
        nevents++;

        bool ret = qApp->qwsProcessEvent(event) == 1;
        delete event;
        if (ret) {
            return true;
        }
    }

#ifdef ZERO_FOR_THE_MOMENT
    if (d->shortcut) {
        return false;
    }
#endif
    extern QList<QWSCommand*> *qt_get_server_queue();
    if (!qt_get_server_queue()->isEmpty()) {
        QWSServer::processEventQueue();
    }

    if (QEventDispatcherUNIX::processEvents(flags))
        return true;
    return (nevents > 0);
}

bool QEventDispatcherQWS::hasPendingEvents()
{
    extern uint qGlobalPostedEventsCount(); // from qapplication.cpp
    return qGlobalPostedEventsCount() || qt_fbdpy->eventPending();
}

void QEventDispatcherQWS::startingUp()
{

}

void QEventDispatcherQWS::closingDown()
{

}

void QEventDispatcherQWS::flush()
{
    if(qApp)
        qApp->sendPostedEvents();
    (void)qt_fbdpy->eventPending(); // flush
}


int QEventDispatcherQWS::select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
                                timeval *timeout)
{
    return QEventDispatcherUNIX::select(nfds, readfds, writefds, exceptfds, timeout);
}

