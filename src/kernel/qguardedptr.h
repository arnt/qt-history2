/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qguardedptr.h#5 $
**
** Definition of QGuardedPtr class
**
** Created : 990929
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.  This file is part of the kernel
** module and therefore may only be used if the kernel module is specified
** as Licensed on the Licensee's License Certificate.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QGUARDEDPTR_H
#define QGUARDEDPTR_H

#ifndef QT_H
#include "qobject.h"
#endif // QT_H

class Q_EXPORT QGuardedPtrPrivate : public  QObject, public QShared
{
    Q_OBJECT
public:
    QGuardedPtrPrivate( QObject*);
    ~QGuardedPtrPrivate();

    QObject* object() const;
private slots:
    void objectDestroyed();
private:
    QObject* obj;
};


template <class T> class Q_EXPORT QGuardedPtr
{
public:
    QGuardedPtr()
    {
	priv = new QGuardedPtrPrivate( 0 );
    }
    QGuardedPtr( T* o)
    {
#if defined(Q_TEMPLATE_NEEDS_EXPLICIT_CONVERSION)
	priv = new QGuardedPtrPrivate( (QObject*)o );
#else
	priv = new QGuardedPtrPrivate( o );
#endif
    }
    QGuardedPtr(const QGuardedPtr<T> &p)
    {
	priv = p.priv;
	ref();
    }
    ~QGuardedPtr()
    {
	deref();
    }

    QGuardedPtr<T> &operator=(const QGuardedPtr<T> &p)
    {
	if ( priv != p.priv ) {
	    deref();
	    priv = p.priv;
	    ref();
	}
	return *this;
    }

    QGuardedPtr<T> &operator=(T* o)
    {
	deref();
#if defined(Q_TEMPLATE_NEEDS_EXPLICIT_CONVERSION)
	priv = new QGuardedPtrPrivate( (QObject*)o );
#else
	priv = new QGuardedPtrPrivate( o );
#endif
	return *this;
    }

    bool operator==( const QGuardedPtr<T> &p ) const
    {
	return priv->object() == p.priv->object();
    }

    bool operator!= ( const QGuardedPtr<T>& p ) const
    {
	return !( *this == p );
    }

    bool isNull() const
    {
	return !priv->object();
    }

    T* operator->() const
    {
	return (T*) priv->object();
    }

    T& operator*() const
    {
	return *( (T*)priv->object() );
    }

    operator T*() const
    {
	return (T*) priv->object();
    }


private:
    void ref()
    {
	priv->ref();
    }
    void deref()
    {
	if ( priv->deref() )
	    delete priv;
    }
    QGuardedPtrPrivate* priv;
};




inline QObject* QGuardedPtrPrivate::object() const
{
    return obj;
}


#endif
