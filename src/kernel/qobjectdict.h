/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qobjectdict.h#10 $
**
** Definition of QObjectDictionary
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

#ifndef QOBJECTDICT_H
#define QOBJECTDICT_H

#ifndef QT_H
#include "qmetaobject.h"
#include "qasciidict.h"
#endif // QT_H


//
// The object dictionary is a collection of QMetaObjects
//


#if defined(Q_TEMPLATEDLL)
template class Q_EXPORT QAsciiDict<QMetaObject>;
#endif


class Q_EXPORT QObjectDictionary : public QAsciiDict<QMetaObject>
{
public:
    QObjectDictionary(int size=17,bool cs=TRUE,bool ck=TRUE)
	: QAsciiDict<QMetaObject>(size,cs,ck) {}
    QObjectDictionary( const QObjectDictionary &dict )
	: QAsciiDict<QMetaObject>(dict) {}
   ~QObjectDictionary() { clear(); }
    QObjectDictionary &operator=(const QObjectDictionary &dict)
	{ return (QObjectDictionary&)QAsciiDict<QMetaObject>::operator=(dict);}
};

// Global object dictionary defined in qmetaobject.cpp

extern Q_EXPORT QObjectDictionary *objectDict;


#endif // QOBJECTDICT_H
