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

/*! \class QTcpSocket

    \reentrant
    \brief The QTcpSocket class provides a TCP connection.

    \if defined(commercial)
    It is part of the <a href="commercialeditions.html">Qt Enterprise Edition</a>.
\endif

    \ingroup io
    \module network

    It is a convenience class for QAbstractSocket.

    \sa QTcpServer, QUdpSocket
*/
#include "qdns.h"
#include "qlist.h"
#include "qsignal.h"
#include "qabstractsocket_p.h"
#include "qtcpsocket.h"
#include "qhostaddress.h"

//#define QTCPSOCKET_DEBUG

#define d d_func()
#define q q_func()

class QTcpSocketPrivate : public QAbstractSocketPrivate
{
    Q_DECLARE_PUBLIC(QTcpSocket)

public:
    QTcpSocketPrivate();
    virtual ~QTcpSocketPrivate();

    void close();
};

/*!
    Creates a QTcpSocket object in \c Qt::Unconnected state.

    The \a parent argument is passed on to the QObject constructor.
*/
QTcpSocket::QTcpSocket(QObject *parent) : QAbstractSocket(Qt::TcpSocket,
                                                          *new QTcpSocketPrivate, parent)
{
#if defined(QTCPSOCKET_DEBUG)
    qDebug("QTcpSocket::QTcpSocket()");
#endif
    setFlags(Sequential | Async);
    setState(Qt::UnconnectedState);
    d->isBuffered = true;
}

/*!
    Destroys the socket. Closes the connection if necessary.

    \sa close()
*/

QTcpSocket::~QTcpSocket()
{
#if defined(QTCPSOCKET_DEBUG)
    qDebug("QTcpSocket::~QTcpSocket()");
#endif
}

QTcpSocketPrivate::QTcpSocketPrivate()
{
    port = 0;
}

QTcpSocketPrivate::~QTcpSocketPrivate()
{
}
