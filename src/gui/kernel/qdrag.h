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

#include "QtCore/qobject.h"

class QMimeData;
class QDragPrivate;
class QWidget;
class QPixmap;
class QPoint;
class QDragManager;

class Q_GUI_EXPORT QDrag : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QDrag)
public:
    QDrag(QWidget *dragSource);
    ~QDrag();

    enum DropAction {
        CopyAction = 0x1,
        MoveAction = 0x2,
        LinkAction = 0x4,
        ActionMask = 0xff,
        TargetMoveAction = 0x8002,
        IgnoreAction = 0x0
    };
    Q_DECLARE_FLAGS(DropActions, DropAction)

    void setMimeData(QMimeData *data);
    QMimeData *mimeData() const;

    void setPixmap(const QPixmap &);
    QPixmap pixmap() const;

    void setHotSpot(const QPoint& hotspot);
    QPoint hotSpot() const;

    QWidget *source() const;
    QWidget *target() const;

    DropAction start(DropActions supportedActions = CopyAction);

signals:
    void actionChanged(DropAction action);
    void targetChanged(QWidget *newTarget);

private:
#ifdef Q_WS_MAC
    friend class QWidgetPrivate;
#endif
    friend class QDragManager;
    Q_DISABLE_COPY(QDrag)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QDrag::DropActions)
#endif
