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

#ifndef QTABLEVIEW_P_H
#define QTABLEVIEW_P_H

#include <private/qabstractitemview_p.h>

class QTableViewPrivate : public QAbstractItemViewPrivate
{
    Q_DECLARE_PUBLIC(QTableView)
public:
    QTableViewPrivate()
        : showGrid(true), gridStyle(Qt::SolidLine), horizontalHeader(0), verticalHeader(0) {}
    void init();

    bool showGrid;
    Qt::PenStyle gridStyle;
    QHeaderView *horizontalHeader;
    QHeaderView *verticalHeader;
    QModelIndex topLeft, bottomRight; // Used for optimization in setSelection
};

#endif
