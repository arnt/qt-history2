/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qobjectlist.h#9 $
**
** Definition of QObjectList
**
** Created : 940807
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

#ifndef QOBJECTLIST_H
#define QOBJECTLIST_H

#ifndef QT_H
#include "qobject.h"
#include "qlist.h"
#endif // QT_H


#if defined(Q_TEMPLATEDLL)
template class Q_EXPORT QList<QObject>;
template class Q_EXPORT QListIterator<QObject>;
#endif


class Q_EXPORT QObjectList : public QList<QObject>
{
public:
    QObjectList() : QList<QObject>() {}
    QObjectList( const QObjectList &list ) : QList<QObject>(list) {}
   ~QObjectList() { clear(); }
    QObjectList &operator=(const QObjectList &list)
	{ return (QObjectList&)QList<QObject>::operator=(list); }
};

class Q_EXPORT QObjectListIt : public QListIterator<QObject>
{
public:
    QObjectListIt( const QObjectList &l ) : QListIterator<QObject>(l) {}
    QObjectListIt &operator=(const QObjectListIt &i)
	{ return (QObjectListIt&)QListIterator<QObject>::operator=(i); }
};


#endif // QOBJECTLIST_H
