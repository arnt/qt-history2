#include "qeventdispatcher_x11.h"

#include "qapplication.h"
#include "qx11info_x11.h"

#include "qt_x11_p.h"
#include <private/qeventdispatcher_unix_p.h>

class QEventDispatcherX11Private : public QEventDispatcherUNIXPrivate
{
    Q_DECLARE_PUBLIC(QEventDispatcherX11)
public:
    inline QEventDispatcherX11Private()
        : xfd(-1)
    { }
    int xfd;
};

QEventDispatcherX11::QEventDispatcherX11(QObject *parent)
    : QEventDispatcherUNIX(*new QEventDispatcherX11Private, parent)
{ }

QEventDispatcherX11::~QEventDispatcherX11()
{ }

bool QEventDispatcherX11::processEvents(QEventLoop::ProcessEventsFlags flags)
{
    // Q_D(QEventDispatcherX11);

    int nevents = 0;

    QApplication::sendPostedEvents();

    // Two loops so that posted events accumulate
    do {
        while (
#if 0
               !d->shortcut &&
#endif
	       XEventsQueued(QX11Info::display(), QueuedAlready)) {
            // process events from the X server
            XEvent event;
            XNextEvent(QX11Info::display(), &event);

            if (flags & QEventLoop::ExcludeUserInputEvents) {
                switch (event.type) {
                case ButtonPress:
                case ButtonRelease:
                case MotionNotify:
                case XKeyPress:
                case XKeyRelease:
                case EnterNotify:
                case LeaveNotify:
                    continue;

                case ClientMessage: {
                    // only keep the wm_take_focus and
                    // qt_qt_scrolldone protocols, discard all
                    // other client messages
                    if (event.xclient.format != 32)
                        continue;

                    if (event.xclient.message_type == ATOM(WM_PROTOCOLS) ||
                        (Atom) event.xclient.data.l[0] == ATOM(WM_TAKE_FOCUS))
                        break;
                    if (event.xclient.message_type == ATOM(_QT_SCROLL_DONE))
                        break;
                }

                default: break;
                }
            }

            nevents++;
            if (qApp->x11ProcessEvent(&event) == 1)
                return true;
        }
    } while (
#if 0
             !d->shortcut &&
#endif
             XEventsQueued(QX11Info::display(), QueuedAfterFlush));

#if 0
    if (d->shortcut)
	return false;
#endif

    // 0x08 == ExcludeTimers for X11 only
    const uint exclude_all =
        QEventLoop::ExcludeSocketNotifiers | 0x08 | QEventLoop::WaitForMoreEvents;
    if (nevents > 0 && (flags & exclude_all) == exclude_all) {
        QApplication::sendPostedEvents();
        return true;
    }

    // return true if we handled events, false otherwise
    return QEventDispatcherUNIX::processEvents(flags) ||  (nevents > 0);
}

bool QEventDispatcherX11::hasPendingEvents()
{
    extern uint qGlobalPostedEventsCount(); // from qapplication.cpp
    return (qGlobalPostedEventsCount() || XPending(QX11Info::display()));
}

void QEventDispatcherX11::flush()
{
    XFlush(QX11Info::display());
}

void QEventDispatcherX11::startingUp()
{
    Q_D(QEventDispatcherX11);
    d->xfd = XConnectionNumber(QX11Info::display());
}

void QEventDispatcherX11::closingDown()
{
    Q_D(QEventDispatcherX11);
    d->xfd = -1;
}

int QEventDispatcherX11::select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
                                timeval *timeout)
{
    Q_D(QEventDispatcherX11);
    if (d->xfd > 0) {
        nfds = qMax(nfds, d->xfd);
        FD_SET(d->xfd, readfds);
    }
    return QEventDispatcherUNIX::select(nfds, readfds, writefds, exceptfds, timeout);
}
