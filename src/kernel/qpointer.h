/****************************************************************************
**
** Definition of QPointer class.
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

#ifndef QPOINTER_H
#define QPOINTER_H

#ifndef QT_H
#include "qobject.h"
#endif
template <class T>
class QPointer
{
    QObject *o;
public:
    inline QPointer() : o(0) {}
    inline QPointer(T *p) : o(p)
	{ QMetaObject::addGuard(&o); }
    inline QPointer(const QPointer<T> &p) : o(p.o)
	{ QMetaObject::addGuard(&o); }
    inline ~QPointer()
	{ QMetaObject::removeGuard(&o); }
    inline QPointer<T> &operator=(const QPointer<T> &p)
	{ QMetaObject::changeGuard(&o, p.o); return *this; }
    inline QPointer<T> &operator=(T* p)
	{ QMetaObject::changeGuard(&o, p); return *this; }

    inline bool operator==( const QPointer<T> &p ) const
	{ return o == p.o; }
    inline bool operator==(const T *p ) const
	{ return o == static_cast<const QObject*>(p); }
    inline bool operator!= (const QPointer<T> &p) const
	{ return o != p.o; }
    inline bool operator!= (const T *p) const
	{ return o != static_cast<const QObject*>(p); }

    inline bool isNull() const
	{ return !o; }

    inline T* operator->() const
	{ return static_cast<T*>(const_cast<QObject*>(o)); }
    inline T& operator*() const
	{ return *static_cast<T*>(const_cast<QObject*>(o)); }
    inline operator T*() const
	{ return static_cast<T*>(const_cast<QObject*>(o)); }
};


#endif // QPOINTER_H
