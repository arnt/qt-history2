/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qaccel.h#27 $
**
** Definition of QAccel class
**
** Created : 950419
**
** Copyright (C) 1995-1997 by Troll Tech AS.  All rights reserved.
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

    void	repairEventFilter();

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

private:	// Disabled copy constructor and operator=
    QAccel( const QAccel & );
    QAccel &operator=( const QAccel & );
};


inline bool QAccel::isEnabled() const
{
    return enabled;
}


#endif // QACCEL_H
