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

//#define QTCPSOCKET_DEBUG

/*! \class QTcpSocket

    \brief The QTcpSocket class provides a TCP socket.

    \reentrant
    \ingroup io
    \module network

    TCP (Transmission Control Protocol) is a reliable,
    stream-oriented, connection-oriented transport protocol. It is
    especially well suited for continuous transmission of data.

    QTcpSocket is a convenience subclass of QAbstractSocket that
    allows you to establish a TCP connection and transfer streams of
    data. See the QAbstractSocket documentation for details.

    The \l{network/fortuneserver}{Fortune Server},
    \l{network/fortuneclient}{Fortune Client},
    \l{network/threadedfortuneserver}{Threaded Fortune Server}, and
    \l{network/blockingfortuneclient}{Blocking Fortune Client}
    examples illustrate how to use QTcpSocket in applications.

    \sa QTcpServer, QUdpSocket, QFtp, QHttp
*/
#include "qlist.h"
#include "qsignal.h"
#include "qabstractsocket_p.h"
#include "qtcpsocket.h"
#include "qhostaddress.h"

#define d d_func()
#define q q_func()

class QTcpSocketPrivate : public QAbstractSocketPrivate
{
    Q_DECLARE_PUBLIC(QTcpSocket)
};

/*!
    Creates a QTcpSocket object in state \c UnconnectedState.

    \a parent is passed on to the QObject constructor.

    \sa socketType()
*/
QTcpSocket::QTcpSocket(QObject *parent)
    : QAbstractSocket(TcpSocket, *new QTcpSocketPrivate, parent)
{
#if defined(QTCPSOCKET_DEBUG)
    qDebug("QTcpSocket::QTcpSocket()");
#endif
    d->isBuffered = true;
}

/*!
    Destroys the socket, closing the connection if necessary.

    \sa close()
*/

QTcpSocket::~QTcpSocket()
{
#if defined(QTCPSOCKET_DEBUG)
    qDebug("QTcpSocket::~QTcpSocket()");
#endif
}
