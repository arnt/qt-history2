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

#ifndef QGENERICTABLEVIEW_P_H
#define QGENERICTABLEVIEW_P_H

#include <private/qabstractitemview_p.h>

class QGenericTableViewPrivate : public QAbstractItemViewPrivate
{
    Q_DECLARE_PUBLIC(QGenericTableView)
public:
    QGenericTableViewPrivate()
        : showGrid(true), gridStyle(Qt::SolidLine), horizontalHeader(0), verticalHeader(0) {}
    void init();

    bool showGrid;
    Qt::PenStyle gridStyle;
    QGenericHeader *horizontalHeader;
    QGenericHeader *verticalHeader;
    QModelIndex topLeft, bottomRight; // Used for optimization in setSelection
};

#endif
