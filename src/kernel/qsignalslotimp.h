/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qsignalslotimp.h#6 $
**
** Definition of signal/slot collections etc.
**
** Created : 980821
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

#ifndef QSIGNALSLOTIMP_H
#define QSIGNALSLOTIMP_H

#ifndef QT_H
#include "qconnection.h"
#include "qlist.h"
#include "qdict.h"
#endif // QT_H


#if defined(Q_TEMPLATEDLL)
template class Q_EXPORT QList<QConnection>;
template class Q_EXPORT QListIterator<QConnection>;
#endif


class Q_EXPORT QConnectionList : public QList<QConnection>
{
public:
    QConnectionList() : QList<QConnection>() {}
    QConnectionList( const QConnectionList &list ) : QList<QConnection>(list) {}
   ~QConnectionList() { clear(); }
    QConnectionList &operator=(const QConnectionList &list)
	{ return (QConnectionList&)QList<QConnection>::operator=(list); }
};

class Q_EXPORT QConnectionListIt : public QListIterator<QConnection>
{
public:
    QConnectionListIt( const QConnectionList &list ) : QListIterator<QConnection>(list) {}
    QConnectionListIt &operator=(const QConnectionListIt &list)
	{ return (QConnectionListIt&)QListIterator<QConnection>::operator=(list); }
};


#if defined(Q_TEMPLATEDLL)
template class Q_EXPORT QDict<QConnectionList>;
template class Q_EXPORT QDictIterator<QConnectionList>;
#endif


class Q_EXPORT QSignalDict : public QDict<QConnectionList>
{
public:
    QSignalDict(int size=17,bool cs=TRUE,bool ck=TRUE) :
	QDict<QConnectionList>(size,cs,ck) {}
    QSignalDict( const QSignalDict &dict ) : QDict<QConnectionList>(dict) {}
   ~QSignalDict() { clear(); }
    QSignalDict &operator=(const QSignalDict &dict)
	{ return (QSignalDict&)QDict<QConnectionList>::operator=(dict); }
};

class Q_EXPORT QSignalDictIt : public QDictIterator<QConnectionList>
{
public:
    QSignalDictIt( const QSignalDict &dict ) : QDictIterator<QConnectionList>(dict) {}
    QSignalDictIt &operator=(const QSignalDictIt &dict)
	{ return (QSignalDictIt&)QDictIterator<QConnectionList>::operator=(dict); }
};


#endif // QSIGNALSLOTIMP_H
