/****************************************************************************
**
** Definition of internal QFocusData class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QFOCUSDATA_H
#define QFOCUSDATA_H

#ifndef QT_H
#include "qptrlist.h"
#endif // QT_H

class QWidget;

class Q_EXPORT QFocusData {
public:
    QWidget* focusWidget() const { return it.current(); }

    QWidget* home();
    QWidget* next();
    QWidget* prev();
    QWidget* first() const;
    QWidget* last() const;
    int count() const { return focusWidgets.count(); }

private:
    friend class QWidget;

    QFocusData()
	: it(focusWidgets) {}
    QPtrList<QWidget> focusWidgets;
    QPtrListIterator<QWidget> it;
};


#endif // QFOCUSDATA_H
