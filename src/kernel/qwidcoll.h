/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qwidcoll.h#9 $
**
** Definition of QWidget collections
**
** Created : 950116
**
** Copyright (C) 1995-1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QWIDCOLL_H
#define QWIDCOLL_H

#ifndef QT_H
#include "qwidget.h"
#include "qlist.h"
#include "qintdict.h"
#endif // QT_H


typedef Q_DECLARE(QListM,QWidget)		QWidgetList;
typedef Q_DECLARE(QListIteratorM,QWidget)	QWidgetListIt;

typedef Q_DECLARE(QIntDictM,QWidget)		QWidgetIntDict;
typedef Q_DECLARE(QIntDictIteratorM,QWidget)	QWidgetIntDictIt;


#endif // QWIDCOLL_H
