/****************************************************************************
** $Id$
**
** Implementation of Network Extension Library
**
** Created : 970521
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
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


class QSocket : public QObject, public QIODevice
{
    Q_OBJECT
public:
    enum Error {
	ErrConnectionRefused,
	ErrHostNotFound,
	ErrSocketRead
    };

    QSocket( QObject *parent=0, const char *name=0 );
   ~QSocket();

    enum State { Idle, HostLookup, Connecting, Listening, Connection, Closing };
    State	 state() const;

    enum Mode { Binary, Ascii };
    Mode	 mode() const;
    void	 setMode( Mode );

    int		socket() const;
    virtual void setSocket( int, bool inet=TRUE );

#ifndef QT_NO_DNS
    virtual void connectToHost( const QString &host, int port );
#endif
    virtual void connectToLocalFile( const QString &file );
    QString	 peerName() const;

    virtual bool listen( const QHostAddress &, int port = 0 );

    // Implementation of QIODevice abstract virtual functions
    bool	 open( int mode );
    void	 close();
    void	 flush();
    uint	 size() const;
    int		 at() const;
    bool	 at( int );
    bool	 atEnd() const;

    int		 bytesAvailable() const;
    int		 waitForMore( int msecs );
    int		 bytesToWrite() const;

    int		 readBlock( char *data, uint maxlen );
    int		 writeBlock( const char *data, uint len );

    int		 getch();
    int		 putch( int );
    int		 ungetch(int);

    bool	 canReadLine() const;
    QString	 readLine();

    uint	 port() const;
    uint	 peerPort() const;
    QHostAddress address() const;
    QHostAddress peerAddress() const;

signals:
    void	 hostFound();
    void	 connected();
    void	 closed();
    void	 delayedCloseFinished();
    void	 readyRead();
    void	 bytesWritten( int nbytes );
    void	 error( int );

protected slots:
    virtual void sn_read();
    virtual void sn_write();

protected:
    QSocketDevice *socketDevice();
    void	timerEvent( QTimerEvent * );

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
