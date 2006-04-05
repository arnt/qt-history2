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

    The QSocketNotifier makes it possible to integrate Qt's event loop
    with other event loops based on file descriptors. For example, the
    \l
    {http://www.trolltech.com/products/solutions/catalog/4/Utilities/qtcorba/}
    {QtCorba Solution} uses it to process CORBA events.  File
    descriptor action is detected in Qt's main event loop
    (QCoreApplication::exec()).

    \target write notifiers

    Once you have opened a device using a low-level (usually
    platform-specific) API, you can create a socket notifier to
    monitor the file descriptor. The socket notifier is enabled by
    default, i.e. it emits the activated() signal whenever a socket
    event corresponding to its type occurs. Connect the activated()
    signal to the slot you want to be called when an event
    corresponding to your socket notifier's type occurs.

    There are three types of socket notifiers: read, write, and
    exception. The type is described by the \l Type enum, and must be
    specified when constructing the socket notifier. After
    construction it can be determined using the type() function. Note
    that if you need to monitor both reads and writes for the same
    file descriptor, you must create two socket notifiers. Note also
    that it is not possible to install two socket notifiers of the
    same type (\l Read, \l Write, \l Exception) on the same socket.

    The setEnabled() function allows you to disable as well as enable
    the socket notifier. It is generally advisable to explicitly
    enable or disable the socket notifier, especially for write
    notifiers. A disabled notifier ignores socket events (the same
    effect as not creating the socket notifier). Use the isEnabled()
    function to determine the notifier's current status.

    Finally, you can use the socket() function to retrieve the
    socket identifier.  Although the class is called QSocketNotifier,
    it is normally used for other types of devices than sockets.
    QTcpSocket and QUdpSocket provide notification through signals, so
    there is normally no need to use a QSocketNotifier on them.

    \bold{Note for Windows users:} The socket passed to QSocketNotifier
    will become non-blocking, even if it was created as a blocking socket.

    \sa QFile, QProcess, QTcpSocket, QUdpSocket
*/

/*!
    \enum QSocketNotifier::Type

    This enum describes the various types of events that a socket
    notifier can recognize. The type must be specified when
    constructing the socket notifier.

    Note that if you need to monitor both reads and writes for the
    same file descriptor, you must create two socket notifiers. Note
    also that it is not possible to install two socket notifiers of
    the same type (Read, Write, Exception) on the same socket.

    \value Read      There is data to be read.
    \value Write      Data can be written.
    \value Exception  An exception has occurred. We recommend against using this.

    \sa QSocketNotifier(), type()
*/

/*!
    Constructs a socket notifier with the given \a parent. It enables
    the \a socket, and watches for events of the given \a type.

    It is generally advisable to explicitly enable or disable the
    socket notifier, especially for write notifiers.

    \bold{Note for Windows users:} The socket passed to QSocketNotifier
    will become non-blocking, even if it was created as a blocking socket.

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
        qWarning("QSocketNotifier: Can only be used with threads started with QThread");
    } else {
        eventDispatcher->registerSocketNotifier(this);
    }
}

#ifdef QT3_SUPPORT
/*!
    \obsolete

    Use the QSocketNotifier() constructor combined with the
    QObject::setObjectName() function instead.

    \oldcode
        QSocketNotifier *notifier = new QSocketNotifier(socket, type, parent, name);
    \newcode
        QSocketNotifier *notifier = new QSocketNotifier(socket, type, parent);
        notifier->setObjectName(name);
    \endcode
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
        qWarning("QSocketNotifier: Can only be used with threads started with QThread");
    } else {
        eventDispatcher->registerSocketNotifier(this);
    }
}
#endif
/*!
    Destroys this socket notifier.
*/

QSocketNotifier::~QSocketNotifier()
{
    setEnabled(false);
}


/*!
    \fn void QSocketNotifier::activated(int socket)

    This signal is emitted whenever the socket notifier is enabled and
    a socket event corresponding to its \l {Type}{type} occurs.

    The socket identifier is passed in the \a socket parameter.

    \sa type(), socket()
*/


/*!
    \fn int QSocketNotifier::socket() const

    Returns the socket identifier specified to the constructor.

    \sa type()
*/

/*!
    \fn Type QSocketNotifier::type() const

    Returns the socket event type specified to the constructor.

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

    The notifier is enabled by default, i.e. it emits the activated()
    signal whenever a socket event corresponding to its
    \l{type()}{type} occurs. If it is disabled, it ignores socket
    events (the same effect as not creating the socket notifier).

    Write notifiers should normally be disabled immediately after the
    activated() signal has been emitted

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
    // Emits the activated() signal when a QEvent::SockAct is
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
