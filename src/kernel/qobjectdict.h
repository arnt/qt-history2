/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qobjectdict.h#8 $
**
** Definition of QObjectDictionary
**
** Created : 940807
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

#ifndef QOBJECTDICT_H
#define QOBJECTDICT_H

#ifndef QT_H
#include "qmetaobject.h"
#include "qdict.h"
#endif // QT_H


//
// The object dictionary is a collection of QMetaObjects
//


#if defined(Q_TEMPLATEDLL)
template class Q_EXPORT QDict<QMetaObject>;
#endif


class Q_EXPORT QObjectDictionary : public QDict<QMetaObject>
{
public:
    QObjectDictionary(int size=17,bool cs=TRUE,bool ck=TRUE) :
	QDict<QMetaObject>(size,cs,ck) {}
    QObjectDictionary( const QObjectDictionary &dict ) : QDict<QMetaObject>(dict) {}
   ~QObjectDictionary() { clear(); }
    QObjectDictionary &operator=(const QObjectDictionary &dict)
	{ return (QObjectDictionary&)QDict<QMetaObject>::operator=(dict); }
};

// Global object dictionary defined in qmetaobject.cpp

extern Q_EXPORT QObjectDictionary *objectDict;


#endif // QOBJECTDICT_H
