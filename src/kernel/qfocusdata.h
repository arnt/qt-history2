/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qfocusdata.h#10 $
**
** Definition of internal QFocusData class
**
** Created : 980405
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.  This file is part of the kernel
** module and therefore may only be used if the kernel module is specified
** as Licensed on the Licensee's License Certificate.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
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
