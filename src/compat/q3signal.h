/****************************************************************************
**
** Definition of Q3Signal class.
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

#ifndef Q3SIGNAL_H
#define Q3SIGNAL_H

#ifndef QT_H
#include "qkernelvariant.h"
#include "qobject.h"
#endif // QT_H


class Q_COMPAT_EXPORT Q3Signal : public QObject
{
    Q_OBJECT

public:
    Q3Signal( QObject *parent=0, const char *name=0 );
    ~Q3Signal();

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
    void	setValue( const QKernelVariant &value );
    QKernelVariant	value() const;
#endif
signals:
#ifndef QT_NO_VARIANT
    void signal( const QKernelVariant& );
#endif
    void intSignal( int );

private:
#ifndef QT_NO_VARIANT
    QKernelVariant val;
#endif
private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    Q3Signal( const Q3Signal & );
    Q3Signal &operator=( const Q3Signal & );
#endif
};


#endif // Q3SIGNAL_H
