/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qwidgetintdict.h#1 $
**
** Definition of QWidgetIntDict
**
** Created : 950116
**
** Copyright (C) 1995-1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QWIDINTDICT_H
#define QWIDINTDICT_H

#ifndef QT_H
#include "qwidget.h"
#include "qintdict.h"
#endif // QT_H


typedef Q_DECLARE(QIntDictM,QWidget)		QWidgetIntDict;
typedef Q_DECLARE(QIntDictIteratorM,QWidget)	QWidgetIntDictIt;


#endif // QWIDINTDICT_H
