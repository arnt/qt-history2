/****************************************************************************
** $Id: //depot/qt/main/src/tools/qtcomm.h#1 $
**
** Definition of QTComm class
**
** Author  : Haavard Nord
** Created : 940913
**
** Copyright (C) 1994-1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QTCOMM_H
#define QTCOMM_H

#include "qiodev.h"
#include "qbuffer.h"


#define IO_Connected	IO_Open			// extended QTComm states
#define IO_Listen	(IO_Connected*2)


class QTComm : public QIODevice			// transport communication
{
public:
    QTComm();
   ~QTComm();

    char   *hostName()	  const { return host.data(); }
    char   *sap()	  const { return servap.data(); }
    void    setHost( const char *hostName, const char *serviceAccessPoint );

    bool    readComplete() const { return readcomp; }
    void    setReadComplete( bool rc ) { readcomp = rc; }

    bool    isConnected() const { return state() == IO_Connected; }
    bool    isListening() const { return state() == IO_Listen; }

    virtual bool listen() = 0;			// enable server
    virtual void hangup() = 0;			// hangup server

    virtual bool connect( int mode ) = 0;	// connect to server

    virtual bool disconnect() = 0;		// disconnect server/client
    virtual void abort() = 0;			// violent disconnect

    virtual bool transmit() = 0;		// xmit/flush buffered data
    void    flush()		{ transmit(); }

    long    size()	const	{ return buffer.size(); }
    int	    getch();				// not working for comm classes
    int	    putch( int );
    int	    ungetch( int );			// not working for comm classes

protected:
    QString host;
    QString servap;
    QBuffer buffer;
    bool    readcomp;

private:
    bool    open( int m )	{ return connect( m ); }
    void    close()		{ disconnect(); }
};


#endif // QTCOMM_H
