/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qobjectdict.h#1 $
**
** Definition of QObjectDictionary
**
** Created : 940807
**
** Copyright (C) 1994-1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QOBJECTDICT_H
#define QOBJECTDICT_H

#ifndef QT_H
#include "qmetaobject.h"
#include "qdict.h"
#endif // QT_H


// QMetaObject collections

typedef Q_DECLARE(QDictM,QMetaObject)	    QObjectDictionary;
typedef QObjectDictionary		    QObjectDict;

extern QObjectDictionary *objectDict;		// global object dictionary
						// defined in qmetaobject.cpp

#endif // QOBJECTDICT_H

