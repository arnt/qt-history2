/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qwidcoll.h#3 $
**
** Definition of QWidget collections
**
** Author  : Haavard Nord
** Created : 950116
**
** Copyright (C) 1995-1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QWIDCOLL_H
#define QWIDCOLL_H

#include "qwidget.h"
#include "qlist.h"
#include "qintdict.h"


typedef declare(QListM,QWidget)		QWidgetList;
typedef declare(QListIteratorM,QWidget) QWidgetListIt;

typedef declare(QIntDictM,QWidget)	   QWidgetIntDict;
typedef declare(QIntDictIteratorM,QWidget) QWidgetIntDictIt;


#endif // QWIDCOLL_H
