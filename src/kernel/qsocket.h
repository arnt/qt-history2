/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qsocket.h#10 $
**
** Definition of QSocket class
**
** Created : 990221
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
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
    QSocket( QObject *parent=0, const char *name=0 );
    QSocket( int socket, QObject *parent=0, const char *name=0 );
   ~QSocket();

    enum State { Idle, HostLookup, Connecting, Connection, Closing };
    State	 state() const;

    enum Mode { Binary, Ascii };
    Mode	 mode() const;
    void	 setMode( Mode );

    bool	 connectToHost( const QString &host, int port );
    QString	 host() const;
    int		 port() const;

    // Implementation of QIODevice abstract virtual functions
    bool	 open( int mode );
    void	 close();
    void	 flush();
    uint	 size() const;
    int		 at() const;
    bool	 at( int );
    bool	 atEnd() const;    

    int		 bytesAvailable() const;
    int		 bytesToWrite() const;

    int		 readBlock( char *data, uint maxlen );
    int		 writeBlock( const char *data, uint len );

    int		 getch();
    int		 putch( int );
    int		 ungetch(int);

    bool	 canReadLine() const;
    QString	 readLine();

signals:
    void	 hostFound();
    void	 connected();
    void	 closed();
    void	 delayedCloseFinished();
    void	 readyRead();
    void	 bytesWritten( int nbytes );
    void	 error();

protected slots:
    virtual void sn_read();
    virtual void sn_write();

protected:
    QSocketDevice *socketDevice();
    void	  timerEvent( QTimerEvent * );

private:
    QSocketPrivate *d;

    bool	 skipReadBuf( int nbytes, char * );
    bool	 skipWriteBuf( int nbytes );
    bool	 scanNewline( QByteArray * = 0 );

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QSocket( const QSocket & );
    QSocket &operator=( const QSocket & );
#endif
};


#endif // QSOCKET_H
