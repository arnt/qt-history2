/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qsignalslotimp.h#9 $
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
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
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
#include "qasciidict.h"
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
    QConnectionListIt( const QConnectionList &l ) : QListIterator<QConnection>(l) {}
    QConnectionListIt &operator=(const QConnectionListIt &i)
	{ return (QConnectionListIt&)QListIterator<QConnection>::operator=(i); }
};


#if defined(Q_TEMPLATEDLL)
template class Q_EXPORT QAsciiDict<QConnectionList>;
template class Q_EXPORT QAsciiDictIterator<QConnectionList>;
#endif


class Q_EXPORT QSignalDict : public QAsciiDict<QConnectionList>
{
public:
    QSignalDict(int size=17,bool cs=TRUE,bool ck=TRUE)
	: QAsciiDict<QConnectionList>(size,cs,ck) {}
    QSignalDict( const QSignalDict &dict )
	: QAsciiDict<QConnectionList>(dict) {}
   ~QSignalDict() { clear(); }
    QSignalDict &operator=(const QSignalDict &dict)
	{ return (QSignalDict&)QAsciiDict<QConnectionList>::operator=(dict); }
};

class Q_EXPORT QSignalDictIt : public QAsciiDictIterator<QConnectionList>
{
public:
    QSignalDictIt( const QSignalDict &d )
	: QAsciiDictIterator<QConnectionList>(d) {}
    QSignalDictIt &operator=(const QSignalDictIt &i)
	{ return (QSignalDictIt&)QAsciiDictIterator<QConnectionList>::operator=(i); }
};


#endif // QSIGNALSLOTIMP_H
