/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qwidgetlist.h#10 $
**
** Definition of QWidgetList
**
** Created : 950116
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
    QWidgetListIt( const QWidgetList &l ) : QListIterator<QWidget>(l) {}
    QWidgetListIt &operator=(const QWidgetListIt &i)
	{ return (QWidgetListIt&)QListIterator<QWidget>::operator=(i); }
};


#endif // QWIDGETLIST_H
