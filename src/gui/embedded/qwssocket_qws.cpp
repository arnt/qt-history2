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

#include "qplatformdefs.h"
#include "qwssocket_qws.h"

#ifndef QT_NO_QWS_MULTIPROCESS

#include <fcntl.h>
#include <netdb.h>
#include <errno.h>
#include <stdio.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/un.h>

#ifdef __MIPSEL__
# ifndef SOCK_DGRAM
#  define SOCK_DGRAM 1
# endif
# ifndef SOCK_STREAM
#  define SOCK_STREAM 2
# endif
#endif

#if defined (Q_OS_SOLARIS)
// uff-da apparently Solaris doesn't have the SUN_LEN macro, here is
// an implementation of it...
#ifndef SUN_LEN
#define SUN_LEN(su) \
        sizeof(*(su)) - sizeof((su)->sun_path) + strlen((su)->sun_path)
#endif

// nor the POSIX names of UNIX domain sockets *sigh*
#ifndef AF_LOCAL
#define AF_LOCAL        AF_UNIX
#endif
#ifndef PF_LOCAL
#define PF_LOCAL        PF_UNIX
#endif

#endif

/***********************************************************************
 *
 * QWSSocket
 *
 **********************************************************************/
QWSSocket::QWSSocket(QObject *parent)
#ifndef QT_NO_SXV
    : QUnixSocket(parent)
#else
    : QTcpSocket(parent)
#endif
{
#ifndef QT_NO_SXV
    QObject::connect( this, SIGNAL(stateChanged(SocketState)),
            this, SLOT(forwardStateChange(SocketState)));
#endif
}

QWSSocket::~QWSSocket()
{
}

#ifndef QT_NO_SXV
QString QWSSocket::errorString()
{
    switch ( QUnixSocket::error() )
    {
        case NoError:
            return QString();
        case InvalidPath:
        case NonexistentPath:
            return QString( "Bad path" ); // NO_TR
        default:
            return QString( "Bad socket" ); // NO TR
    }
}

void QWSSocket::forwardStateChange(QUnixSocket::SocketState st  )
{
    switch ( st )
    {
        case ConnectedState:
            emit connected();
            break;
        case ClosingState:
            break;
        case UnconnectedState:
            emit disconnected();
            break;
        default:
            // nothing
            break;
    }
    if ( QUnixSocket::error() != NoError )
        emit error((QAbstractSocket::SocketError)0);
}
#endif

bool QWSSocket::connectToLocalFile(const QString &file)
{
#ifndef QT_NO_SXV
    bool result = QUnixSocket::connect( file.toLocal8Bit() );
    if ( !result )
    {
        perror( "QWSSocketAuth::connectToLocalFile could not connect:" );
        emit error(QAbstractSocket::ConnectionRefusedError);
        return false;
    }
    return true;
#else
    // create socket
    int s = ::socket(PF_LOCAL, SOCK_STREAM, 0);

    // connect to socket
    struct sockaddr_un a;
    memset(&a, 0, sizeof(a));
    a.sun_family = PF_LOCAL;
    strncpy(a.sun_path, file.toLocal8Bit().constData(), sizeof(a.sun_path) - 1);
    int r = ::connect(s, (struct sockaddr*)&a, SUN_LEN(&a));
    if (r == 0) {
        setSocketDescriptor(s);
    } else {
        perror("QWSSocket::connectToLocalFile could not connect:");
        ::close(s);
        emit error(ConnectionRefusedError);
        return false;
    }
#endif
    return true;
}


/***********************************************************************
 *
 * QWSServerSocket
 *
 **********************************************************************/
QWSServerSocket::QWSServerSocket(const QString& file, QObject *parent)
#ifndef QT_NO_SXV
    : QUnixSocketServer(parent)
#else
    : QTcpServer(parent)
#endif
{
    init(file);
}

void QWSServerSocket::init(const QString &file)
{
#ifndef QT_NO_SXV
    QByteArray fn = file.toLocal8Bit();
    bool result = QUnixSocketServer::listen( fn );
    if ( !result )
    {
        QUnixSocketServer::ServerError err = serverError();
        switch ( err )
        {
            case InvalidPath:
                qWarning("QWSServerSocket:: invalid path %s", qPrintable(file));
                break;
            case ResourceError:
            case BindError:
            case ListenError:
                qWarning("QWSServerSocket:: could not listen on path %s", qPrintable(file));
                break;
            default:
                break;
        }
    }
#else
    int backlog = 16; //#####

// create socket
    int s = ::socket(PF_LOCAL, SOCK_STREAM, 0);
    QByteArray fn = file.toLocal8Bit();
    unlink(fn.constData()); // doesn't have to succeed

    // bind socket
    struct sockaddr_un a;
    memset(&a, 0, sizeof(a));
    a.sun_family = PF_LOCAL;
    strncpy(a.sun_path, fn.constData(), sizeof(a.sun_path) - 1);
    int r = ::bind(s, (struct sockaddr*)&a, SUN_LEN(&a));
    if (r < 0) {
        qWarning("QWSServerSocket: could not bind to file %s", fn.constData());
        ::close(s);
        return;
    }

    if (chmod(fn.constData(), 0600) < 0) {
        qWarning("Could not set permissions of %s", fn.constData());
        ::close(s);
        return;
    }

    // listen
    if (::listen(s, backlog) == 0) {
        if (!setSocketDescriptor(s))
            qWarning( "QWSServerSocket could not set descriptor %d : %s", s, errorString().toLatin1().constData());
    } else {
        qWarning("QWSServerSocket: could not listen to file %s", fn.constData());
        ::close(s);
    }
#endif
}

QWSServerSocket::~QWSServerSocket()
{
}

#ifndef QT_NO_SXV

void QWSServerSocket::incomingConnection(int socketDescriptor)
{
    inboundConnections.append( socketDescriptor );
    emit newConnection();
}


QWSSocket *QWSServerSocket::nextPendingConnection()
{
    QMutexLocker locker( &ssmx );
    if ( inboundConnections.count() == 0 )
        return 0;
    QWSSocket *s = new QWSSocket();
    s->setSocketDescriptor( inboundConnections.takeFirst() );
    return s;
}

#endif // QT_NO_SXV

#endif  //QT_NO_QWS_MULTIPROCESS
