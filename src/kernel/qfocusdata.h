/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qfocusdata.h#1 $
**
** Definition of QFocusData class
**
** Created : 980405
**
** Copyright (C) 1997-1998 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QFOCUSDATA_H
#define QFOCUSDATA_H

#ifndef QT_H
#include "qlist.h"
#include "qwidget.h"
#endif // QT_H


class QFocusData {
public:
    QWidget* current() const { return it.current(); }
    QWidget* first() { return focusWidgets.first(); }
    QWidget* last() { return focusWidgets.last(); }
    QWidget* next() { return focusWidgets.next(); }
    QWidget* prev() { return focusWidgets.prev(); }

private:
    friend QWidget;
    QFocusData()
	: it(focusWidgets) {}
    QList<QWidget> focusWidgets;
    QListIterator<QWidget> it;
};

#endif // QFOCUSDATA_H
