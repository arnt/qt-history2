/****************************************************************************
** $Id$
**
** Definition of QSocket class.
**
** Created : 970521
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the network module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.  This file is part of the network
** module and therefore may only be used if the network module is specified
** as Licensed on the Licensee's License Certificate.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QSOCKET_H
#define QSOCKET_H

#ifndef QT_H
#include "qsocketdevice.h"
#include "qsocketnotifier.h"
#endif // QT_H


class QSocketPrivate;


class Q_EXPORT QSocket : public QObject, public QIODevice
{
    Q_OBJECT
public:
    enum Error {
	ErrConnectionRefused,
	ErrHostNotFound,
	ErrSocketRead
    };

    QSocket( QObject *parent=0, const char *name=0 );
    virtual ~QSocket();

    enum State { Idle, HostLookup, Connecting,
		 Listening, Connection, Closing };
    State	 state() const;

    int		socket() const;
    virtual void setSocket( int );

#ifndef QT_NO_DNS
    virtual void connectToHost( const QString &host, Q_UINT16 port );
#endif
    QString	 peerName() const;

    // Implementation of QIODevice abstract virtual functions
    bool	 open( int mode );
    void	 close();
    void	 flush();
    uint	 size() const;
    int		 at() const;
    bool	 at( int );
    bool	 atEnd() const;

    int		 bytesAvailable() const;
    int		 waitForMore( int msecs ) const;
    int		 bytesToWrite() const;

    int		 readBlock( char *data, uint maxlen );
    int		 writeBlock( const char *data, uint len );
    int		 readLine( char *data, uint maxlen );

    int		 getch();
    int		 putch( int );
    int		 ungetch(int);

    bool	 canReadLine() const;
    virtual QString	 readLine();

    Q_UINT16	 port() const;
    Q_UINT16	 peerPort() const;
    QHostAddress address() const;
    QHostAddress peerAddress() const;

signals:
    void	 hostFound();
    void	 connected();
    void	 connectionClosed();
    void	 delayedCloseFinished();
    void	 readyRead();
    void	 bytesWritten( int nbytes );
    void	 error( int );

protected slots:
    virtual void sn_read();
    virtual void sn_write();

protected:
    QSocketDevice *socketDevice();

private slots:
    void	tryConnecting();

private:
    QSocketPrivate *d;

    bool	 consumeReadBuf( int nbytes, char * );
    bool	 consumeWriteBuf( int nbytes );
    bool	 scanNewline( QByteArray * = 0 );
    void	 tryConnection();

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QSocket( const QSocket & );
    QSocket &operator=( const QSocket & );
#endif
};


#endif // QSOCKET_H
