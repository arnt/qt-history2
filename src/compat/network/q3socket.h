/****************************************************************************
** $Id$
**
** Definition of Q3Socket class.
**
** Created : 970521
**
** Copyright (C) 1992-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the network module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition licenses may use this
** file in accordance with the Qt Commercial License Agreement provided
** with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef Q3SOCKET_H
#define Q3SOCKET_H

#ifndef QT_H
#include "qiodevice.h"
#include "qhostaddress.h" // int->QHostAddress conversion
#endif // QT_H

#ifndef QT_NO_NETWORK
class Q3SocketPrivate;
class Q3SocketDevice;


class Q_CORE_EXPORT Q3Socket : public QIODevice
{
    Q_OBJECT
public:
    enum Error {
	ErrConnectionRefused,
	ErrHostNotFound,
	ErrSocketRead
    };

    Q3Socket( QObject *parent=0, const char *name=0 );
    virtual ~Q3Socket();

    enum State { Idle, HostLookup, Connecting,
		 Connected, Closing,
		 Connection=Connected };
    State	 state() const;

    int		 socket() const;
    virtual void setSocket( int );

    Q3SocketDevice *socketDevice();
    virtual void setSocketDevice( Q3SocketDevice * );

#ifndef QT_NO_DNS
    virtual void connectToHost( const QString &host, Q_UINT16 port );
#endif
    QString	 peerName() const;

    // Implementation of QIODevice abstract virtual functions
    bool	 open( int mode );
    void	 close();
    bool	 flush();
    Offset	 size() const;
    Offset	 at() const;
    bool	 at( Offset );
    bool	 atEnd() const;

    Q_LONGLONG	 bytesAvailable() const; // ### QIODevice::Offset instead?
    Q_ULONG	 waitForMore( int msecs, bool *timeout  ) const;
    Q_ULONG	 waitForMore( int msecs ) const; // ### Qt 4.0: merge the two overloads
    Q_LONGLONG	 bytesToWrite() const;
    void	 clearPendingData();

    int		 getch();
    int		 putch( int );
    int		 ungetch(int);

    bool	 canReadLine() const;

    Q_UINT16	 port() const;
    Q_UINT16	 peerPort() const;
    QHostAddress address() const;
    QHostAddress peerAddress() const;

    void	 setReadBufferSize( Q_ULONG );
    Q_ULONG	 readBufferSize() const;

    inline bool  isSequential() const { return !isOpen(); }

signals:
    void	 hostFound();
    void	 connected();
    void	 connectionClosed();
    void	 delayedCloseFinished();
    void	 readyRead();
    void	 bytesWritten( int nbytes );
    void	 error( int );

protected slots:
    virtual void sn_read( bool force=FALSE );
    virtual void sn_write();

protected:
    Q_LONGLONG readData(char *data, Q_LONGLONG maxlen);
    Q_LONGLONG writeData(const char *data, Q_LONGLONG len);

private slots:
    void	tryConnecting();
    void	emitErrorConnectionRefused();

private:
    Q3SocketPrivate *d;

    bool	 consumeWriteBuf( Q_ULONG nbytes );
    void	 tryConnection();
    void         setSocketIntern( int socket );

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    Q3Socket( const Q3Socket & );
    Q3Socket &operator=( const Q3Socket & );
#endif
};

#endif //QT_NO_NETWORK
#endif // Q3SOCKET_H
