/****************************************************************************
** $Id$
**
** Definition of QGuardedPtr class
**
** Created : 990929
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef QGUARDEDPTR_H
#define QGUARDEDPTR_H

#ifndef QT_H
#include "qobject.h"
#endif // QT_H

class QGuardedPtrData
{
public:
    static void add(QObject **ptr);
    static void remove(QObject **ptr);
    static void replace(QObject **ptr, QObject *o);
};

template <class T>
class QGuardedPtr
{
    QObject *o;
public:
    inline QGuardedPtr() : o(0) {}
    inline QGuardedPtr(T *obj) : o(obj)
	{ QGuardedPtrData::add(&o); }
    inline QGuardedPtr(const QGuardedPtr<T> &p) : o(p.o)
	{ QGuardedPtrData::add(&o); }
    inline ~QGuardedPtr() { QGuardedPtrData::remove(&o); }
    inline QGuardedPtr<T> &operator=(const QGuardedPtr<T> &p)
	{ QGuardedPtrData::replace(&o, p.o); return *this; }
    inline QGuardedPtr<T> &operator=(T* obj)
	{ QGuardedPtrData::replace(&o, obj); return *this; }

    inline bool operator==( const QGuardedPtr<T> &p ) const
	{ return (o == p.o); }
    inline bool operator!= ( const QGuardedPtr<T>& p ) const
	{ return (o != p.o); }

    inline bool isNull() const { return !o; }

    inline T* operator->() const { return static_cast<T*>(const_cast<QObject*>(o)); }
    inline T& operator*() const { return *static_cast<T*>(const_cast<QObject*>(o)); }
    inline operator T*() const { return static_cast<T*>(const_cast<QObject*>(o)); }
};

#define Q_DEFINED_QGUARDEDPTR
#include "qwinexport.h"
#endif
