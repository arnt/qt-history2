/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qobjcoll.h#9 $
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

Q_DECLARE(QListM,QObject);
Q_DECLARE(QListIteratorM,QObject);
typedef QListM(QObject) QObjectList;		// object list
typedef QListIteratorM(QObject) QObjectListIt;	// object list iterator


// QMetaObject collections

Q_DECLARE(QDictM,QMetaObject);
typedef QDictM(QMetaObject) QObjectDictionary;	// meta object dictionary

extern QObjectDictionary *objectDict;		// global object dictionary
						// defined in qmetaobj.cpp

#endif // QOBJCOLL_H

