/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qfocusdata.h#9 $
**
** Definition of internal QFocusData class
**
** Created : 980405
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QFOCUSDATA_H
#define QFOCUSDATA_H

#ifndef QT_H
#include "qwidgetlist.h"
#endif // QT_H


class Q_EXPORT QFocusData {
public:
    QWidget* focusWidget() const { return it.current(); }

    // List-iteration
    QWidget* home();
    QWidget* next();
    QWidget* prev();
    int count() const { return focusWidgets.count(); }

private:
    friend class QWidget;
    QFocusData()
	: it(focusWidgets) {}
    QWidgetList	  focusWidgets;
    QWidgetListIt it;
};


#endif // QFOCUSDATA_H
