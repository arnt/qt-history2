/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qsignal.h#1 $
**
** Definition of QSignal class
**
** Author  : Haavard Nord
** Created : 941201
**
** Copyright (C) 1994 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QSIGNAL_H
#define QSIGNAL_H

#include "qobject.h"


class QSignal : private QObject			// signal class
{
public:
    QSignal( const char *name=0 );
   ~QSignal();

    const char *className() const;
    const char *name() const		{ return QObject::name(); }
    void    setName( const char *name ) { QObject::setName(name); }

    bool    connect( const QObject *receiver, const char *member );
    bool    disconnect( const QObject *receiver, const char *member=0 );

    void    activate();				// activate signal

    bool    blocked()  const	    	{ return QObject::signalsBlocked(); }
    void    block( bool b )		{ QObject::blockSignals( b ); }

private:
    QConnectionList *connlist;			// connection list
};


#endif // QSIGNAL_H
