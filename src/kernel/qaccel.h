/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qaccel.h#12 $
**
** Definition of QAccel class
**
** Author  : Haavard Nord
** Created : 950419
**
** Copyright (C) 1995-1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QACCEL_H
#define QACCEL_H

#include "qobject.h"
#include "qkeycode.h"


class QAccelList;				// internal class


class QAccel : public QObject			// accelerator class
{
    Q_OBJECT
public:
    QAccel( QWidget *parent, const char *name=0 );
   ~QAccel();

    bool	isEnabled() const;
    void	setEnabled( bool );
#if defined(OBSOLETE)
    void	enable();			// enable accelerator
    void	disable();			// disable accelerator
    bool	isDisabled()	const;
#endif

    uint	count() const;

    int		insertItem( int key, int id=-1 );
    void	removeItem( int id );
    void	clear();

    int		key( int id );
    int		findKey( int key ) const;

    bool	isItemEnabled( int id )	 const;
    void	setItemEnabled( int id, bool enable );
#if defined(OBSOLETE)
    bool	isItemDisabled( int id ) const	{ return !isItemEnabled(id); }
    void	enableItem( int id )		{ setItemEnabled( id, TRUE ); }
    void	disableItem( int id )		{ setItemEnabled( id, FALSE );}
#endif

    bool	connectItem( int id,
			     const QObject *receiver, const char *member );
    bool	disconnectItem( int id,
				const QObject *receiver, const char *member );

signals:
    void	activated( int id );

protected:
    bool	eventFilter( QObject *, QEvent * );

private slots:
    void	tlwDestroyed();

private:
    QAccelList *aitems;
    bool	enabled;
    QWidget    *tlw;
};


bool QAccel::isEnabled() const
{
    return enabled;
}

#if defined(OBSOLETE)
void QAccel::enable()
{
    qObsolete( "QAccel", "enable", "setEnabled(TRUE)" );
    setEnabled( TRUE );
}

void QAccel::disable()
{
    qObsolete( "QAccel", "disable", "setEnabled(FALSE)" );
    setEnabled( FALSE );
}

bool QAccel::isDisabled() const
{
    qObsolete( "QAccel", "isDisabled", "!isEnabled()" );
    return !isEnabled();
}

bool QAccel::isItemDisabled( int id ) const
{
    qObsolete( "QAccel", "isItemDisabled", "!isItemEnabled(id)" );
    return !isItemEnabled(id);
}

void QAccel::enableItem( int id )
{
    qObsolete( "QAccel", "enableItem", "setItemEnabled(id,TRUE)" );
    setItemEnabled( id, TRUE );
}

void QAccel::disableItem( int id )
{
    qObsolete( "QAccel", "disableItem", "setItemEnabled(id,FALSE)" );
    setItemEnabled( id, FALSE );
}
#endif


#endif // QACCEL_H
