/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qaccel.h#18 $
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

    uint	count() const;

    int		insertItem( int key, int id=-1 );
    void	removeItem( int id );
    void	clear();

    int		key( int id );
    int		findKey( int key ) const;

    bool	isItemEnabled( int id )	 const;
    void	setItemEnabled( int id, bool enable );

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

#if defined(OBSOLETE)
public:
    void	enable();			// enable accelerator
    void	disable();			// disable accelerator
    bool	isDisabled()	const;
    bool	isItemDisabled( int id ) const;
    void	enableItem( int id );
    void	disableItem( int id );
#endif

private:	// Disabled copy constructor and operator=
    QAccel( const QAccel & ) {}
    QAccel &operator=( const QAccel & ) { return *this; }
};


inline bool QAccel::isEnabled() const
{
    return enabled;
}

#if defined(OBSOLETE)
inline void QAccel::enable()
{
    qObsolete( "QAccel", "enable", "setEnabled(TRUE)" );
    setEnabled( TRUE );
}

inline void QAccel::disable()
{
    qObsolete( "QAccel", "disable", "setEnabled(FALSE)" );
    setEnabled( FALSE );
}

inline bool QAccel::isDisabled() const
{
    qObsolete( "QAccel", "isDisabled", "!isEnabled()" );
    return !isEnabled();
}

inline bool QAccel::isItemDisabled( int id ) const
{
    qObsolete( "QAccel", "isItemDisabled", "!isItemEnabled(id)" );
    return !isItemEnabled(id);
}

inline void QAccel::enableItem( int id )
{
    qObsolete( "QAccel", "enableItem", "setItemEnabled(id,TRUE)" );
    setItemEnabled( id, TRUE );
}

inline void QAccel::disableItem( int id )
{
    qObsolete( "QAccel", "disableItem", "setItemEnabled(id,FALSE)" );
    setItemEnabled( id, FALSE );
}
#endif


#endif // QACCEL_H
