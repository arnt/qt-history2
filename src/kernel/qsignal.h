/****************************************************************************
**
** Definition of QSignal class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QSIGNAL_H
#define QSIGNAL_H

#ifndef QT_H
#include "qvariant.h"
#include "qobject.h"
#endif // QT_H


class Q_EXPORT QSignal : public QObject
{
    Q_OBJECT

public:
    QSignal( QObject *parent=0, const char *name=0 );
    ~QSignal();

    bool	connect( const QObject *receiver, const char *member );
    bool	disconnect( const QObject *receiver, const char *member=0 );

    void	activate();

#ifndef QT_NO_COMPAT
    bool	isBlocked()	 const		{ return QObject::signalsBlocked(); }
    void	block( bool b )		{ QObject::blockSignals( b ); }
#ifndef QT_NO_VARIANT
    void	setParameter( int value );
    int		parameter() const;
#endif
#endif

#ifndef QT_NO_VARIANT
    void	setValue( const QVariant &value );
    QVariant	value() const;
#endif
signals:
#ifndef QT_NO_VARIANT
    void signal( const QVariant& );
#endif
    void intSignal( int );

private:
#ifndef QT_NO_VARIANT
    QVariant val;
#endif
private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QSignal( const QSignal & );
    QSignal &operator=( const QSignal & );
#endif
};


#endif // QSIGNAL_H
