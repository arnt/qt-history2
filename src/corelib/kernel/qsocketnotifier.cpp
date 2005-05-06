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

#include "qplatformdefs.h"

#include "qabstracteventdispatcher.h"
#include "qcoreapplication.h"


/*!
    \class QSocketNotifier
    \brief The QSocketNotifier class provides support for monitoring
    activity on a file descriptor.

    \ingroup io

    The QSocketNotifier makes it possible to integrate Qt's event
    loop with other event loops based on file descriptors. For
    example, the QtCorba Solution uses it to process CORBA events.
    File descriptor action is detected in Qt's main event loop
    (QCoreApplication::exec()).

    Once you have opened a device using a low-level (usually
    platform-specific) API, you can create a socket notifier to
    monitor the file descriptor. You can then connect the activated()
    signal to the slot you want to be called whenever an event
    occurs.

    Although the class is called QSocketNotifier, it is normally used
    for other types of devices than sockets. QTcpSocket and
    QUdpSocket provide notification through signals, so there is
    normally no need to use a QSocketNotifier on them.

    \target write notifiers

    There are three types of socket notifiers: read, write, and
    exception. You must specify one of these in the constructor.

    The type specifies when the activated() signal is to be emitted:
    \list 1
    \o QSocketNotifier::Read - There is data to be read.
    \o QSocketNotifier::Write - Data can be written.
    \o QSocketNofifier::Exception - An exception has occurred.
       We recommend against using this.
    \endlist

    If you need to monitor both reads and writes for the same file
    descriptor, you must create two socket notifiers.

    \sa QFile, QProcess, QTcpSocket, QUdpSocket
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

    QAbstractEventDispatcher *eventDispatcher = QAbstractEventDispatcher::instance(thread());
    if (!eventDispatcher) {
        qWarning("QSocketNotifier can only be used with threads started with QThread");
    } else {
        eventDispatcher->registerSocketNotifier(this);
    }
}

#ifdef QT3_SUPPORT
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

    QAbstractEventDispatcher *eventDispatcher = QAbstractEventDispatcher::instance(thread());
    if (!eventDispatcher) {
        qWarning("QSocketNotifier can only be used with threads started with QThread");
    } else {
        eventDispatcher->registerSocketNotifier(this);
    }
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
    notifiers in the \l{write notifiers}{class description} above.

    \sa isEnabled(), activated()
*/

void QSocketNotifier::setEnabled(bool enable)
{
    if (sockfd < 0)
        return;
    if (snenabled == enable)                        // no change
        return;
    snenabled = enable;

    QAbstractEventDispatcher *eventDispatcher = QAbstractEventDispatcher::instance(thread());
    if (!eventDispatcher) // perhaps application/thread is shutting down
        return;
    if (snenabled)
        eventDispatcher->registerSocketNotifier(this);
    else
        eventDispatcher->unregisterSocketNotifier(this);
}


/*!\reimp
*/
bool QSocketNotifier::event(QEvent *e)
{
    // Emits the activated() signal when a \c QEvent::SockAct is
    // received.
    if (e->type() == QEvent::ThreadChange) {
        if (snenabled) {
            QMetaObject::invokeMethod(this, "setEnabled", Qt::QueuedConnection,
                                      Q_ARG(bool, snenabled));
            setEnabled(false);
        }
    }
    QObject::event(e);                        // will activate filters
    if (e->type() == QEvent::SockAct) {
        emit activated(sockfd);
        return true;
    }
    return false;
}
