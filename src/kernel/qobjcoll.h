/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qobjcoll.h#7 $
**
** Definition of QObject and QMetaObject collections
**
** Created : 940807
**
** Copyright (C) 1994-1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QOBJCOLL_H
#define QOBJCOLL_H

#include "qmetaobj.h"
#include "qlist.h"
#include "qdict.h"


// QObject collections

declare(QListM,QObject);
declare(QListIteratorM,QObject);
typedef QListM(QObject) QObjectList;		// object list
typedef QListIteratorM(QObject) QObjectListIt;	// object list iterator


// QMetaObject collections

declare(QDictM,QMetaObject);
typedef QDictM(QMetaObject) QObjectDictionary;	// meta object dictionary

extern QObjectDictionary *objectDict;		// global object dictionary
						// defined in qmetaobj.cpp

#endif // QOBJCOLL_H

