/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qobjcoll.h#1 $
**
** Definition of QObject and QMetaObject collections
**
** Author  : Haavard Nord
** Created : 940807
**
** Copyright (C) 1994 by Troll Tech as.	 All rights reserved.
**
** --------------------------------------------------------------------------
** There is an application global object dictionary, objectDict, which stores
** all loaded meta objects.
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
						// defined in qmetaobj.C

#endif // QOBJCOLL_H

