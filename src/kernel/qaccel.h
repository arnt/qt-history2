/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qaccel.h#48 $
**
** Definition of QAccel class
**
** Created : 950419
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QACCEL_H
#define QACCEL_H

#ifndef QT_H
#include "qobject.h"
#endif // QT_H

#ifndef QT_NO_ACCEL
class QAccelPrivate;				// internal class


class Q_EXPORT QAccel : public QObject			// accelerator class
{
    Q_OBJECT
public:
    QAccel( QWidget *parent, const char *name=0 );
    QAccel( QWidget* watch, QObject *parent, const char *name=0 );
    ~QAccel();

    bool	isEnabled() const;
    void	setEnabled( bool );

    uint	count() const;

    int		insertItem( int key, int id=-1);
    void	removeItem( int id );
    void	clear();

    int		key( int id );
    int		findKey( int key ) const;

    bool	isItemEnabled( int id ) const;
    void	setItemEnabled( int id, bool enable );

    bool	connectItem( int id,
			     const QObject *receiver, const char* member );
    bool	disconnectItem( int id,
				const QObject *receiver, const char* member );

    void	repairEventFilter();

    void setWhatsThis( int id, const QString& );
    QString whatsThis( int id ) const;
    void setIgnoreWhatsThis( bool );
    bool ignoreWhatsThis() const;

    static int shortcutKey( const QString & );
    static QString keyToString( int k );
    static int stringToKey( const QString & );

signals:
    void	activated( int id );

protected:
    bool	eventFilter( QObject *, QEvent * );

private:
    QAccelPrivate * d;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QAccel( const QAccel & );
    QAccel &operator=( const QAccel & );
#endif
};

#endif // QT_NO_ACCEL
#endif // QACCEL_H
