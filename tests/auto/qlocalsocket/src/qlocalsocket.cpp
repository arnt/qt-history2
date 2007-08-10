/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qlocalsocket.h"
#include "qlocalsocket_p.h"

#include <qdatetime.h>

/*!
    Creates a new local socket. The parent argument is passed to QObject's constructor.
 */
QLocalSocket::QLocalSocket(QObject * parent) : QIODevice(*new QLocalSocketPrivate, parent)
{
    Q_D(QLocalSocket);
    d->init();
}

/*!
    Destroys the socket.
 */
QLocalSocket::~QLocalSocket()
{
    close();
}

/*!
    Returns the name of the peer as specified by connectToName(),
    or an empty QString if connectToName() has not been called.

    \sa connectToName()
 */
QString QLocalSocket::peerName() const
{
    Q_D(const QLocalSocket);
    return d->peerName;
}

/*!
    Returns the state of the socket.

    \sa error()
 */
QLocalSocket::LocalSocketState QLocalSocket::state() const
{
    Q_D(const QLocalSocket);
    return d->state;
}

#include "moc_qlocalsocket.cpp"
