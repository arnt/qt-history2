/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qobject.h#4 $
**
** Definition of QObject class
**
** Author  : Haavard Nord
** Created : 930418
**
** Copyright (C) 1993,1994 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QOBJECT_H
#define QOBJECT_H

#include "qobjdefs.h"
#include "qstring.h"


class QObject					// base class for Q objects
{
friend class QPart;
friend class QView;
public:
    QObject( QObject *parent=0, const char *name=0 );
    virtual ~QObject();

    virtual bool event( QEvent * );		// handle event

    virtual QMetaObject *metaObject() const { return metaObj; }
    virtual const char *className() const;	// get name of class

    const char *name()		  const { return (const char *)objname; }
    bool	isParentType()	  const { return isPType; }
    bool	isWidgetType()	  const { return isWidget; }

    bool	signalsBlocked()  const { return blockSig; }
    void	blockSignals( bool b );

    int		startTimer( long interval );	// start timer events
    void	killTimer( int id );		// kill timer event
    void	killTimers();			// kill all timers for object

protected:
    QObject	*parent() const { return parentObj; }
    QConnection *receiver( const char *signal) const;

    virtual void initMetaObject();		// initialize meta object

    bool	bind( const char *, const QObject *, const char * );
    bool	unbind( const char * );

    uint	isPType	   : 1;			// is parent type
    uint	isWidget   : 1;			// is widget
    uint	hasTimer   : 1;			// receives timer events
    uint	blockSig   : 1;			// blocking signals
    QObject    *parentObj;			// parent object
    QObject    *sender;				// sender of last signal

private:
    static QMetaObject *metaObj;		// meta object for class
    QString	objname;			// object name
    QConnections *connections;			// signal connections
};


class QSenderObject : public QObject		// object for sending signals
{
public:
    void setSender( QObject *s ) { sender=s; }
};


#endif // QOBJECT_H
