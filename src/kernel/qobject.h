/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qobject.h#7 $
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
friend class QWidget;
friend class QSignal;
friend class QSenderObject;
public:
    QObject( QObject *parent=0, const char *name=0 );
    virtual ~QObject();

    virtual bool event( QEvent * );		// handle event

    virtual QMetaObject *metaObject() const { return metaObj; }
    virtual const char  *className() const;	// get name of class

    const char *name()		  const { return (const char *)objname; }
    void	setName( const char *name );
    bool	isWidgetType()	  const { return isWidget; }

    bool	signalsBlocked()  const { return blockSig; }
    void	blockSignals( bool b );

    int		startTimer( long interval );	// start timer events
    void	killTimer( int id );		// kill timer event
    void	killTimers();			// kill all timers for object

    QObjectList *children() const { return childObjects; }

    void	dumpObjectTree();		// NOTE!!! For debugging

protected:
    QObject	*parent() const { return parentObj; }
    QConnection *receiver( const char *signal) const;
    QObject     *sender();			// sender of last signal

    void	insertChild( QObject * );	// add child object
    void	removeChild( QObject * );	// remove child object

    static bool	connect( QObject *sender, const char *signal,
			 const QObject *receiver, const char *member );
    bool	connect( QObject *sender, const char *signal,
			 const char *member );
    static bool disconnect( QObject *sender, const char *signal,
			    const QObject *receiver, const char *member );
    bool	disconnect( QObject *sender, const char *signal=0,
			    const char *member=0 );

    virtual void initMetaObject();		// initialize meta object

    uint	isSignal   : 1;			// is signal object
    uint	isWidget   : 1;			// is widget object
    uint	hasTimer   : 1;			// receives timer events
    uint	blockSig   : 1;			// blocking signals

private:
    bool	bind( const char *, const QObject *, const char * );
    static QMetaObject *metaObj;		// meta object for class
    QString	  objname;			// object name
    QObject      *parentObj;			// parent object
    QObjectList  *childObjects;			// list of children objects
    QConnections *connections;			// connections (signals out)
    QObjectList  *senderObjects;		// list of sender objects
    QObject	 *sigSender;			// sender of last signal
};


inline bool QObject::connect( QObject *sender, const char *signal,
			      const char *member )
{
    return connect( sender, signal, this, member );
}

inline bool QObject::disconnect( QObject *sender, const char *signal,
				 const char *member )
{
    return disconnect( sender, signal, this, member );
}


class QSenderObject : public QObject		// object for sending signals
{
public:
    void setSender( QObject *s ) { sigSender=s; }
};


#endif // QOBJECT_H
