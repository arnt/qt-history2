/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qsignal.h#5 $
**
** Definition of QSignal class
**
** Author  : Haavard Nord
** Created : 941201
**
** Copyright (C) 1994-1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QSIGNAL_H
#define QSIGNAL_H

#include "qobject.h"


class QSignal : private QObject			// signal class
{
    Q_OBJECT
public:
    QSignal( QObject *parent=0, const char *name=0 );

    const char *name() const		{ return QObject::name(); }
    void    setName( const char *name ) { QObject::setName(name); }

    bool    connect( const QObject *receiver, const char *member );
    bool    disconnect( const QObject *receiver, const char *member=0 );

    bool    isBlocked()	 const		{ return QObject::signalsBlocked(); }
    void    block( bool b )		{ QObject::blockSignals( b ); }

    void    activate()			{ activate_signal("x()"); }

private:
    void    dummy();
};


#endif // QSIGNAL_H
