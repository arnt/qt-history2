/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qobjectlist.h#1 $
**
** Definition of QObjectList
**
** Created : 940807
**
** Copyright (C) 1994-1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QOBJECTLIST_H
#define QOBJECTLIST_H

#ifndef QT_H
#include "qobject.h"
#include "qlist.h"
#endif // QT_H


// QObject collections

typedef Q_DECLARE(QListM,QObject)	    QObjectList;
typedef Q_DECLARE(QListIteratorM,QObject)   QObjectListIt;


#endif // QOBJECTLIST_H

