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

#include "qobject.h"

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
        DefaultAction,
        AskAction,
        CopyAction,
        MoveAction,
        LinkAction,
        PrivateAction,
        NoAction
    };

    void setMimeData(QMimeData *data);
    QMimeData *mimeData() const;

    void setPixmap(const QPixmap &);
    QPixmap pixmap() const;

    void setHotSpot(const QPoint& hotspot);
    QPoint hotSpot() const;

    QWidget *source() const;
    QWidget *target() const;

    DropAction start(DropAction request = DefaultAction);

signals:
    void defaultActionChanged(DropAction action);

private:
    friend class QDragManager;
    Q_DISABLE_COPY(QDrag)
};

#endif
