/****************************************************************************
**
** Definition of QObjectList.
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

#ifndef QOBJECTLIST_H
#define QOBJECTLIST_H

#ifndef QT_H
#include "qobject.h"
#include "qptrlist.h"
#endif // QT_H


#if defined(Q_TEMPLATEDLL)
//Q_TEMPLATE_EXTERN template class Q_EXPORT QPtrList<QObject>;
//Q_TEMPLATE_EXTERN template class Q_EXPORT QPtrListIterator<QObject>;
#endif


class Q_EXPORT QObjectList : public QPtrList<QObject>
{
public:
    QObjectList() : QPtrList<QObject>() {}
    QObjectList( const QObjectList &list ) : QPtrList<QObject>(list) {}
   ~QObjectList() { clear(); }
    QObjectList &operator=(const QObjectList &list)
	{ return (QObjectList&)QPtrList<QObject>::operator=(list); }
};

class Q_EXPORT QObjectListIterator : public QPtrListIterator<QObject>
{
public:
    QObjectListIterator( const QObjectList &l )
	: QPtrListIterator<QObject>( l ) { }
    QObjectListIterator &operator=( const QObjectListIterator &i )
	{ return (QObjectListIterator&)
		 QPtrListIterator<QObject>::operator=( i ); }
};

#endif // QOBJECTLIST_H
