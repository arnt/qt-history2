/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qobject.h#20 $
**
** Definition of QObject class
**
** Author  : Haavard Nord
** Created : 930418
**
** Copyright (C) 1993-1995 by Troll Tech AS.  All rights reserved.
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
    virtual bool eventFilter( QObject *, QEvent * );

    virtual QMetaObject *metaObject() const { return metaObj; }
    virtual const char  *className()  const;	// get name of class

    bool	isA( const char * )	 const;
    bool	inherits( const char * ) const;

    const char *name()		  const { return (const char *)objname; }
    void	setName( const char *name );
    bool	isWidgetType()	  const { return isWidget; }
    bool	highPriority()	  const { return hiPriority; }

    bool	signalsBlocked()  const { return blockSig; }
    void	blockSignals( bool b );

    int		startTimer( long interval );	// start timer events
    void	killTimer( int id );		// kill timer event
    void	killTimers();			// kill all timers for object

    const QObjectList *children() const { return childObjects; }

    void	insertChild( QObject * );	// add child object
    void	removeChild( QObject * );	// remove child object

    void	installEventFilter( const QObject * );
    void	removeEventFilter( const QObject * );

    static bool	connect( QObject *sender, const char *signal,
			 const QObject *receiver, const char *member );
    bool	connect( QObject *sender, const char *signal,
			 const char *member ) const;
    static bool disconnect( QObject *sender, const char *signal,
			    const QObject *receiver, const char *member );
    bool	disconnect( const char *signal=0,
			    const QObject *receiver=0, const char *member=0 );
    bool	disconnect( const QObject *receiver, const char *member=0 );

    void	dumpObjectTree();		// NOTE!!! For debugging
    void	dumpObjectInfo();

protected:
    QObject	*parent() const { return parentObj; }
    bool	activate_filters( QEvent * );
    QConnectionList *receivers( const char *signal ) const;
    void	activate_signal( const char *signal );
    void	activate_signal( const char *signal, short );
    void	activate_signal( const char *signal, int );
    void	activate_signal( const char *signal, long );
    void	activate_signal( const char *signal, const char * );
    QObject     *sender();			// sender of last signal

    virtual void initMetaObject();		// initialize meta object

    uint	isSignal   : 1;			// is signal object
    uint	isWidget   : 1;			// is widget object
    uint	hiPriority : 1;			// high priority object
    uint	pendTimer  : 1;			// pending timer(s)
    uint	pendEvent  : 1;			// pending event(s)
    uint	blockSig   : 1;			// blocking signals

private:
    bool	 bind( const char *, const QObject *, const char * );
    QMetaObject *queryMetaObject() const;
    static QMetaObject *metaObj;		// meta object for class
    QString	 objname;			// object name
    QObject     *parentObj;			// parent object
    QObjectList *childObjects;			// list of children objects
    QSignalDict *connections;			// connections (signals out)
    QObjectList *senderObjects;			// list of sender objects
    QObjectList *eventFilters;			// list of event filters
    QObject	*sigSender;			// sender of last signal
};


inline bool QObject::connect( QObject *sender, const char *signal,
			      const char *member ) const
{
    return connect( sender, signal, this, member );
}

inline bool QObject::disconnect( const char *signal,
				 const QObject *receiver, const char *member )
{
    return disconnect( this, signal, receiver, member );
}

inline bool QObject::disconnect( const QObject *receiver, const char *member )
{
    return disconnect( this, 0, receiver, member );
}

inline QObject *QObject::sender()
{
    return sigSender;
}

class QSenderObject : public QObject		// object for sending signals
{
public:
    void setSender( QObject *s ) { sigSender=s; }
};


#endif // QOBJECT_H
