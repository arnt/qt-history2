/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qwidgetlist.h#1 $
**
** Definition of QWidgetList
**
** Created : 950116
**
** Copyright (C) 1995-1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QWIDGETLIST_H
#define QWIDGETLIST_H

#ifndef QT_H
#include "qwidget.h"
#include "qlist.h"
#endif // QT_H


typedef Q_DECLARE(QListM,QWidget)		QWidgetList;
typedef Q_DECLARE(QListIteratorM,QWidget)	QWidgetListIt;

#endif // QWIDGETLIST_H
