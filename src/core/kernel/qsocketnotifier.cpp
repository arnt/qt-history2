/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qsocketnotifier.h"
#include "qcoreapplication.h"
#include "qeventloop.h"

#if defined(Q_OS_UNIX)
#include <sys/types.h>
#endif


/*!
    \class QSocketNotifier qsocketnotifier.h
    \brief The QSocketNotifier class provides support for socket callbacks.

    \ingroup io

    The QSocketNotifier class makes it possible to write asynchronous
    socket-based code in Qt. Synchronous socket operations block
    execution of the program, which is clearly not acceptable for an
    event-driven GUI application.

    Once you have opened a non-blocking socket (whether for TCP, UDP,
    a UNIX-domain socket, or any other protocol family your operating
    system supports), you can create a socket notifier to monitor the
    socket. You can then connect the activated() signal to the slot you
    want to be called whenever a socket event occurs.

    Note for Windows users: the socket passed to QSocketNotifier will
    become non-blocking, even if it was created as a blocking socket.

    There are three types of socket notifiers (read, write and
    exception); you must specify one of these in the constructor.

    The type specifies when the activated() signal is to be emitted:
    \list 1
    \i QSocketNotifier::Read - There is data to be read (socket read event).
    \i QSocketNotifier::Write - Data can be written (socket write event).
    \i QSocketNofifier::Exception - An exception has occurred (socket
    exception event). We recommend against using this.
    \endlist

    For example, if you need to monitor both reads and writes for the
    same socket you must create two socket notifiers.

    Example:
    \code
    int sockfd;                                 // socket identifier
    struct sockaddr_in sa;                      // should contain host address
    sockfd = socket(AF_INET, SOCK_STREAM, 0);   // create TCP socket
    // make the socket non-blocking here, usually using fcntl(O_NONBLOCK)
    ::connect(sockfd, (struct sockaddr*)&sa, sizeof(sa)); // connect to host
                                                          // NOT QObject::connect()!
    QSocketNotifier *sn;
    sn = new QSocketNotifier(sockfd, QSocketNotifier::Read, parent);
    QObject::connect(sn, SIGNAL(activated(int)),
                     myObject, SLOT(dataReceived()));
    \endcode

    The optional \e parent argument can be set to make the socket
    notifier a child of any QObject; e.g. a widget. This will ensure
    that it is automatically destroyed when the widget is destroyed.

    For read notifiers it makes little sense to connect the
    activated() signal to more than one slot because the data can be
    read from the socket only once.

    Also observe that if you do not read all the available data when
    the read notifier fires, it fires again and again.

    If you disable the read notifier your program may deadlock. (The
    same applies to exception notifiers if you must use them, for
    instance if you \e must use TCP urgent data.)

    For write notifiers, immediately disable the notifier after the
    activated() signal has been received, and when you have sent the
    data to be written on the socket. When you have more data to be
    written, enable it again to get a new activated() signal. The
    exception is if the socket data writing operation (send() or
    equivalent) fails with a "would block" error, which means that
    some buffer is full and you must wait before sending more data.
    In that case you do not need to disable and re-enable the write
    notifier; it will fire again as soon as the system allows more
    data to be sent.

    The behavior of a write notifier that is left in enabled state
    after having emitting the first activated() signal (and no "would
    block" error has occurred) is undefined. Depending on the
    operating system, it may fire on every pass of the event loop, or
    not at all.

    If you need a timeout for your sockets you can use either \link
    QObject::startTimer() timer events\endlink or the QTimer class.

    Socket action is detected in the \link QApplication::exec() main
    event loop\endlink of Qt. The X11 version of Qt has a single UNIX
    select() call that incorporates all socket notifiers and the X
    socket.

    \sa QSocket, QServerSocket, QSocketDevice
*/

/*!
    \enum QSocketNotifier::Type

    The socket notifier can be used to inform the application of the
    following types of event:

    \value Read       There is incoming data.
    \value Write      Data can be written.
    \value Exception  An exception has occurred.
*/


/*!
    Constructs a socket notifier with the given \a parent. It enables
    the \a socket, and watches for events of the given \a type.

    It is generally advisable to explicitly enable or disable the
    socket notifier, especially for write notifiers.

    \sa setEnabled(), isEnabled()
*/

QSocketNotifier::QSocketNotifier(int socket, Type type, QObject *parent)
    : QObject(parent)
{
    if (socket < 0)
        qWarning("QSocketNotifier: Invalid socket specified");
#if defined(Q_OS_UNIX)
    if (socket >= FD_SETSIZE)
        qWarning("QSocketNotifier: Socket descriptor too large for select()");
#endif
    sockfd = socket;
    sntype = type;
    snenabled = true;
    QEventLoop::instance(thread())->registerSocketNotifier(this);
}

#ifdef QT_COMPAT
/*!
  \obsolete

    Constructs a socket notifier called \a name, with the given
    \a parent. It enables the \a socket, and watches for events of the
    given \a type.

    It is generally advisable to explicitly enable or disable the
    socket notifier, especially for write notifiers.

    \sa setEnabled(), isEnabled()
*/

QSocketNotifier::QSocketNotifier(int socket, Type type, QObject *parent,
                                  const char *name)
    : QObject(parent)
{
    setObjectName(QString::fromAscii(name));
    if (socket < 0)
        qWarning("QSocketNotifier: Invalid socket specified");
#if defined(Q_OS_UNIX)
    if (socket >= FD_SETSIZE)
        qWarning("QSocketNotifier: Socket descriptor too large for select()");
#endif
    sockfd = socket;
    sntype = type;
    snenabled = true;
    QEventLoop::instance(thread())->registerSocketNotifier(this);
}
#endif
/*!
    Destroys the socket notifier.
*/

QSocketNotifier::~QSocketNotifier()
{
    setEnabled(false);
}


/*!
    \fn void QSocketNotifier::activated(int socket)

    This signal is emitted under certain conditions specified by the
    notifier type():
    \list 1
    \i QSocketNotifier::Read - There is data to be read (socket read event).
    \i QSocketNotifier::Write - Data can be written (socket write event).
    \i QSocketNofifier::Exception - An exception has occurred (socket
    exception event).
    \endlist

    The \a socket is the \link socket() socket\endlink identifier.

    \sa type(), socket()
*/


/*!
    \fn int QSocketNotifier::socket() const

    Returns the socket identifier specified to the constructor.

    \sa type()
*/

/*!
    \fn Type QSocketNotifier::type() const

    Returns the socket event type specified to the constructor: \c
    QSocketNotifier::Read, \c QSocketNotifier::Write, or \c
    QSocketNotifier::Exception.

    \sa socket()
*/


/*!
    \fn bool QSocketNotifier::isEnabled() const

    Returns true if the notifier is enabled; otherwise returns false.

    \sa setEnabled()
*/

/*!
    If \a enable is true, the notifier is enabled; otherwise the notifier
    is disabled.

    The notifier is enabled by default.

    If the notifier is enabled, it emits the activated() signal
    whenever a socket event corresponding to its \link type()
    type\endlink occurs. If it is disabled, it ignores socket events
    (the same effect as not creating the socket notifier).

    Write notifiers should normally be disabled immediately after the
    activated() signal has been emitted; see discussion of write
    notifiers in the \link #details class description\endlink above.

    \sa isEnabled(), activated()
*/

void QSocketNotifier::setEnabled(bool enable)
{
    if (sockfd < 0)
        return;
    if (snenabled == enable)                        // no change
        return;
    snenabled = enable;

    QEventLoop *eventloop = QEventLoop::instance(thread());
    if (! eventloop) // perhaps application is shutting down
        return;

    if (snenabled)
        eventloop->registerSocketNotifier(this);
    else
        eventloop->unregisterSocketNotifier(this);
}


/*!\reimp
*/
bool QSocketNotifier::event(QEvent *e)
{
    // Emits the activated() signal when a \c QEvent::SockAct is
    // received.
    QObject::event(e);                        // will activate filters
    if (e->type() == QEvent::SockAct) {
        emit activated(sockfd);
        return true;
    }
    return false;
}
