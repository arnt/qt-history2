/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qfocusdata.h#3 $
**
**		      ***   INTERNAL HEADER FILE   ***
**
**		This file is NOT a part of the Qt interface!
**
** Definition of internal QFocusData class
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
    QWidget* inFocus() const { return it.current(); }

    // List-iteration
    QWidget* current() { focusWidgets.find(it.current()); return focusWidgets.current(); }
    QWidget* first() { return focusWidgets.first(); }
    QWidget* last() { return focusWidgets.last(); }
    QWidget* next() { return focusWidgets.next(); }
    QWidget* prev() { return focusWidgets.prev(); }
    int count() { return focusWidgets.count(); }

private:
    friend class QWidget;
    QFocusData()
	: it(focusWidgets) {}
    QList<QWidget> focusWidgets;
    QListIterator<QWidget> it;
};

#endif // QFOCUSDATA_H
