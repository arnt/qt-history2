/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qobjcoll.h#11 $
**
** Definition of QObject and QMetaObject collections
**
** Created : 940807
**
** Copyright (C) 1994-1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QOBJCOLL_H
#define QOBJCOLL_H

#include "qmetaobj.h"
#include "qlist.h"
#include "qdict.h"


// QObject collections

typedef Q_DECLARE(QListM,QObject)	    QObjectList;
typedef Q_DECLARE(QListIteratorM,QObject)   QObjectListIt;


// QMetaObject collections

typedef Q_DECLARE(QDictM,QMetaObject)	    QObjectDictionary;

extern QObjectDictionary *objectDict;		// global object dictionary
						// defined in qmetaobj.cpp

#endif // QOBJCOLL_H

