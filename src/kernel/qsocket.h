/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qsocket.h#4 $
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


class QSocket : public QObject, public QIODevice
{
    Q_OBJECT
public:
    QSocket();
    QSocket( int socket );
   ~QSocket();

    enum State { Idle, HostLookup, Connecting, Connection };
    State	 state() const;

    enum Mode { Binary, Ascii };
    Mode	 mode() const;
    void	 setMode( Mode );

    void	 connectToHost( const QString &host, int port );
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
    int		 readBlock( char *data, uint maxlen );
    int		 writeBlock( const char *data, uint len );

signals:
    void	 hostFound();
    void	 connected();
    void	 closed();
    void	 readyRead();
    void	 readyWrite();

protected slots:
    virtual void sn_read();
    virtual void sn_write();

protected:
    QSocketDevice *socketDevice();

private:
    QSocketPrivate *d;

    bool	 skipReadBuf( int nbytes, char * );
    bool	 skipWriteBuf( int nbytes );

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QSocket( const QSocket & );
    QSocket &operator=( const QSocket & );
#endif
};


#endif // QSOCKET_H
