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

#include "qvfbsocket.h"

#include <fcntl.h>
#include <netdb.h>
#include <unistd.h>
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
 * QVFbSocket
 *
 **********************************************************************/
QVFbSocket::QVFbSocket(QObject *parent)
    : QTcpSocket(parent)
{
    QObject::connect( this, SIGNAL(stateChanged(SocketState)),
            this, SLOT(forwardStateChange(SocketState)));
}

QVFbSocket::~QVFbSocket()
{
}

bool QVFbSocket::connectToLocalFile(const QString &file)
{
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
        perror("QVFbSocket::connectToLocalFile could not connect:");
        ::close(s);
        emit error(ConnectionRefusedError);
        return false;
    }

    return true;
}


/***********************************************************************
 *
 * QVFbServerSocket
 *
 **********************************************************************/
QVFbServerSocket::QVFbServerSocket(const QString& file, QObject *parent)
    : QTcpServer(parent)
{
    init(file);
}

void QVFbServerSocket::init(const QString &file)
{
    int backlog = 16; //#####

// create socket
    int s = ::socket(PF_LOCAL, SOCK_STREAM, 0);
    if (s == -1) {
        perror("QVFbServerSocket::init");
        qWarning("QVFbServerSocket: unable to create socket.");
        return;
    }

    QByteArray fn = file.toLocal8Bit();
    unlink(fn.constData()); // doesn't have to succeed

    // bind socket
    struct sockaddr_un a;
    memset(&a, 0, sizeof(a));
    a.sun_family = PF_LOCAL;
    strncpy(a.sun_path, fn.constData(), sizeof(a.sun_path) - 1);
    int r = ::bind(s, (struct sockaddr*)&a, SUN_LEN(&a));
    if (r < 0) {
        perror("QVFbServerSocket::init");
        qWarning("QVFbServerSocket: could not bind to file %s", fn.constData());
        ::close(s);
        return;
    }

    if (chmod(fn.constData(), 0600) < 0) {
        perror("QVFbServerSocket::init");
        qWarning("Could not set permissions of %s", fn.constData());
        ::close(s);
        return;
    }

    // listen
    if (::listen(s, backlog) == 0) {
        if (!setSocketDescriptor(s))
            qWarning( "QVFbServerSocket could not set descriptor %d : %s", s, errorString().toLatin1().constData());
    } else {
        perror("QVFbServerSocket::init");
        qWarning("QVFbServerSocket: could not listen to file %s", fn.constData());
        ::close(s);
    }
}

QVFbServerSocket::~QVFbServerSocket()
{
}


void QVFbServerSocket::incomingConnection(int socketDescriptor)
{
    inboundConnections.append( socketDescriptor );
    emit newConnection();
}


QVFbSocket *QVFbServerSocket::nextPendingConnection()
{
    QMutexLocker locker( &ssmx );
    if ( inboundConnections.count() == 0 )
        return 0;
    QVFbSocket *s = new QVFbSocket();
    s->setSocketDescriptor( inboundConnections.takeFirst() );
    return s;
}

