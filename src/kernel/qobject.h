/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qobject.h#42 $
**
** Definition of QObject class
**
** Created : 930418
**
** Copyright (C) 1993-1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QOBJECT_H
#define QOBJECT_H

#include "qobjdefs.h"
#include "qstring.h"
#include "qevent.h"


class QObject					// base class for Q objects
{
public:
    QObject( QObject *parent=0, const char *name=0 );
    virtual ~QObject();

    virtual bool event( QEvent * );
    virtual bool eventFilter( QObject *, QEvent * );

    virtual QMetaObject *metaObject() const { return metaObj; }
    virtual const char	*className()  const;

    bool	isA( const char * )	 const;
    bool	inherits( const char * ) const;

    const char *name()		  const { return objname; }
    void	setName( const char *name );
    bool	isWidgetType()	  const { return isWidget; }
    bool	highPriority()	  const { return hiPriority; }

    bool	signalsBlocked()  const { return blockSig; }
    void	blockSignals( bool b );

    int		startTimer( int interval );
    void	killTimer( int id );
    void	killTimers();

    const QObjectList *children() const { return childObjects; }
    QObjectList	      *queryList( const char *inheritsClass = 0,
				  const char *objName = 0,
				  bool regexpMatch = TRUE,
				  bool recursiveSearch = TRUE );

    void	insertChild( QObject * );
    void	removeChild( QObject * );

    void	installEventFilter( const QObject * );
    void	removeEventFilter( const QObject * );

    static bool connect( const QObject *sender, const char *signal,
			 const QObject *receiver, const char *member );
    bool	connect( const QObject *sender, const char *signal,
			 const char *member ) const;
    static bool disconnect( const QObject *sender, const char *signal,
			    const QObject *receiver, const char *member );
    bool	disconnect( const char *signal=0,
			    const QObject *receiver=0, const char *member=0 );
    bool	disconnect( const QObject *receiver, const char *member=0 );

    void	dumpObjectTree();
    void	dumpObjectInfo();

signals:
    void	destroyed();

public:
    QObject	*parent() const { return parentObj; }

protected:
    bool	activate_filters( QEvent * );
    QConnectionList *receivers( const char *signal ) const;
    void	activate_signal( const char *signal );
    void	activate_signal( const char *signal, short );
    void	activate_signal( const char *signal, int );
    void	activate_signal( const char *signal, long );
    void	activate_signal( const char *signal, const char * );
    const QObject *sender();

    virtual void initMetaObject();

    virtual void timerEvent( QTimerEvent * );

    virtual void connectNotify( const char *signal );
    virtual void disconnectNotify( const char *signal );
    virtual bool checkConnectArgs( const char *signal, const QObject *receiver,
				   const char *member );

    static  void badSuperclassWarning( const char *className, 
				       const char *superclassName );

    uint	isSignal   : 1;
    uint	isWidget   : 1;
    uint	hiPriority : 1;
    uint	pendTimer  : 1;
    uint	pendEvent  : 1;
    uint	blockSig   : 1;

private slots:
    void	cleanupEventFilter();

private:
    QMetaObject *queryMetaObject() const;
    static QMetaObject *metaObj;
    char	*objname;
    QObject	*parentObj;
    QObjectList *childObjects;
    QSignalDict *connections;
    QObjectList *senderObjects;
    QObjectList *eventFilters;
    QObject	*sigSender;

    friend class QApplication;
    friend class QWidget;
    friend class QSignal;
    friend class QSenderObject;

private:	// Disabled copy constructor and operator=
    QObject( const QObject & ) {}
    QObject &operator=( const QObject & ) { return *this; }
};


inline bool QObject::connect( const QObject *sender, const char *signal,
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

inline const QObject *QObject::sender()
{
    return sigSender;
}


class QSenderObject : public QObject		// object for sending signals
{
public:
    void setSender( QObject *s ) { sigSender=s; }
};


#endif // QOBJECT_H
