/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qwidgetlist.h#8 $
**
** Definition of QWidgetList
**
** Created : 950116
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QWIDGETLIST_H
#define QWIDGETLIST_H

#ifndef QT_H
#include "qwidget.h"
#include "qlist.h"
#endif // QT_H


#if defined(Q_TEMPLATEDLL)
template class Q_EXPORT QList<QWidget>;
template class Q_EXPORT QListIterator<QWidget>;
#endif


class Q_EXPORT QWidgetList : public QList<QWidget>
{
public:
    QWidgetList() : QList<QWidget>() {}
    QWidgetList( const QWidgetList &list ) : QList<QWidget>(list) {}
   ~QWidgetList() { clear(); }
    QWidgetList &operator=(const QWidgetList &list)
	{ return (QWidgetList&)QList<QWidget>::operator=(list); }
};

class Q_EXPORT QWidgetListIt : public QListIterator<QWidget>
{
public:
    QWidgetListIt( const QWidgetList &list ) : QListIterator<QWidget>(list) {}
    QWidgetListIt &operator=(const QWidgetListIt &list)
	{ return (QWidgetListIt&)QListIterator<QWidget>::operator=(list); }
};


#endif // QWIDGETLIST_H
