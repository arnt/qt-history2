/****************************************************************************
**
** Definition of QDeepCopy class.
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

#ifndef QDEEPCOPY_H
#define QDEEPCOPY_H

#ifndef QT_H
#  include "qglobal.h"
#endif // QT_H

#ifdef QT_COMPAT
template <class T>
class QT_COMPAT QDeepCopy
{
public:
    inline QDeepCopy()
    {
    }

    inline QDeepCopy( const T &t )
	: deepcopy( t )
    {
	deepcopy.detach();
    }

    inline QDeepCopy<T> &operator=( const T &t )
    {
	deepcopy = t;
	deepcopy.detach();
	return *this;
    }

    inline operator T ()
    {
	T tmp = deepcopy;
	tmp.detach();
	return tmp;
    }

private:
    T deepcopy;
};
#endif

#endif // QDEEPCOPY_H
