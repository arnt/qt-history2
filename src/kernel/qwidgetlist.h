/****************************************************************************
**
** Definition of QWidgetList.
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

#ifndef QWIDGETLIST_H
#define QWIDGETLIST_H

#ifndef QT_H
#include "qwidget.h"
#include "qptrlist.h"
#endif // QT_H

class Q_EXPORT QWidgetList : public QPtrList<QWidget>
{
public:
    QWidgetList() : QPtrList<QWidget>() {}
    QWidgetList( const QWidgetList &list ) : QPtrList<QWidget>(list) {}
   ~QWidgetList() { clear(); }
    QWidgetList &operator=(const QWidgetList &list)
	{ return (QWidgetList&)QPtrList<QWidget>::operator=(list); }
};

class Q_EXPORT QWidgetListIt : public QPtrListIterator<QWidget>
{
public:
    QWidgetListIt( const QWidgetList &l ) : QPtrListIterator<QWidget>(l) {}
    QWidgetListIt &operator=(const QWidgetListIt &i)
	{ return (QWidgetListIt&)QPtrListIterator<QWidget>::operator=(i); }
};

#endif // QWIDGETLIST_H
