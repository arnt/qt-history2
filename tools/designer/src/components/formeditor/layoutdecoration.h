/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef LAYOUTDECORATION_H
#define LAYOUTDECORATION_H

#include <extension.h>
#include <QObject>
#include <qpair.h>

class QPoint;

struct ILayoutDecoration
{
    enum InsertMode
    {
        InsertWidgetMode,
        InsertRowMode,
        InsertColumnMode
    };

    virtual ~ILayoutDecoration() {}

    virtual InsertMode currentInsertMode() const = 0;
    virtual int currentIndex() const = 0;
    virtual QPair<int, int> currentCell() const = 0;
    virtual void insertWidget(QWidget *widget, const QPair<int, int> &cell) = 0;
    virtual void removeWidget(QWidget *widget) = 0;

    virtual void insertRow(int row) = 0;
    virtual void insertColumn(int column) = 0;
    virtual void simplify() = 0;

    virtual int findItemAt(const QPoint &pos) const = 0;
    virtual void adjustIndicator(const QPoint &pos, int index) = 0;
};
Q_DECLARE_EXTENSION_INTERFACE(ILayoutDecoration, "http://trolltech.com/Qt/IDE/LayoutDecoration")

#endif // LAYOUTDECORATION_H
