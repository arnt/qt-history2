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

#ifndef QDRAG_H
#define QDRAG_H

#include "qglobal.h"

class QMimeData;
class QDragPrivate;
class QWidget;
class QPixmap;
class QPoint;
class QDragManager;

class Q_GUI_EXPORT QDrag
{
public:
    QDrag(QWidget *dragSource);
    ~QDrag();

    enum DragOperation {
        NoDrag = 0,
        CopyDrag = 0x1,
        MoveDrag = 0x2,
        LinkDrag = 0x4,
        CopyOrMoveDrag = 0x3,
        DefaultDrag = 0xffff
    };
    Q_DECLARE_FLAGS(DragOperations, DragOperation)

    void setMimeData(QMimeData *data);
    QMimeData *mimeData() const;

    void setPixmap(const QPixmap &);
    QPixmap pixmap() const;

    void setHotSpot(const QPoint& hotspot);
    QPoint hotSpot() const;

    void setAllowedOperations(DragOperations actions);
    DragOperations allowedOperations() const;

    QWidget *source() const;
    QWidget *target() const;

    DragOperation start();

private:
    QDragPrivate *d;
    friend class QDragManager;
    Q_DISABLE_COPY(QDrag)
};

#endif
