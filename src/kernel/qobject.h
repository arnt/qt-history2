/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qobject.h#2 $
**
** Definition of QObject class
**
** Author  : Haavard Nord
** Created : 930418
**
** Copyright (C) 1993,1994 by Troll Tech as.  All rights reserved.
**
** --------------------------------------------------------------------------
** The QObject class is the base object of all objects that receive events or
** emit signals.
*****************************************************************************/

#ifndef QOBJECT_H
#define QOBJECT_H

#include "qobjdefs.h"


class QObject					// base class for Q objects
{
friend class QPart;
friend class QView;
public:
    QObject( QObject *parent=0 );		// create object with parent
    virtual ~QObject();

    virtual bool  event( QEvent * );		// handle event

    virtual QMetaObject *metaObject() const { return metaObj; }
    virtual char *className() const;		// get name of class

    bool    isParentType() const { return isPType; }
    bool    isWidgetType() const { return isWidget; }

    int	    startTimer( long interval );	// start timer events
    void    killTimer( int id );		// kill timer event
    void    killTimers();			// kill all timers for object

protected:
    QObject	*parent() const { return parentObj; }
    QConnection *receiver( const char *signal) const;

    virtual void initMetaObject();		// initialize meta object

    bool    bind( const char *, const QObject *, const char * );
    bool    unbind( const char * );

    int	    isPType    : 1;			// is parent type
    int	    isWidget   : 1;			// is widget
    int	    hasTimer : 1;			// receives timer events
    QObject *parentObj;				// parent object
    QObject *sender;				// sender of last signal

private:
    static QMetaObject *metaObj;		// meta object for class
    QConnections *connections;			// signal connections
};


class QSenderObject : public QObject		// object for sending signals
{
public:
    void setSender( QObject *s ) { sender=s; }
};


#endif // QOBJECT_H
