/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qobject.h#6 $
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
public:
    QObject( QObject *parent=0, const char *name=0 );
    virtual ~QObject();

    virtual bool event( QEvent * );		// handle event

    virtual QMetaObject *metaObject() const { return metaObj; }
    virtual const char *className() const;	// get name of class

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

    void    	insertChild( QObject * );	// add child object
    void	removeChild( QObject * );	// remove child object

    bool    	connect( QObject *, const char *, const char * );
    bool    	connect( QObject *, const char *, const QObject*, const char*);
    bool    	disconnect( QObject *, const char * );
    bool	bind( const char *, const QObject *, const char * );
    bool	unbind( const char * );

    virtual void initMetaObject();		// initialize meta object

    uint	isWidget   : 1;			// is widget
    uint	hasTimer   : 1;			// receives timer events
    uint	blockSig   : 1;			// blocking signals
    QObject    *parentObj;			// parent object
    QObject    *sender;				// sender of last signal

private:
    static QMetaObject *metaObj;		// meta object for class
    QString	  objname;			// object name
    QConnections *connections;			// signal connections
    QObjectList  *childObjects;			// list of children
};


inline bool QObject::connect( QObject *sigobj, const char *signal,
			      const char *member )
{
    return connect( sigobj, signal, this, member );
}


class QSenderObject : public QObject		// object for sending signals
{
public:
    void setSender( QObject *s ) { sender=s; }
};


#endif // QOBJECT_H
