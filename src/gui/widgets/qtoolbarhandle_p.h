/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QDYNAMICTOOLBARHANDLE_P_H
#define QDYNAMICTOOLBARHANDLE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "QtGui/qwidget.h"
#include "QtGui/qlayoutitem.h"
#ifndef QT_NO_TOOLBAR

class QStyleOption;
class QToolBar;
class QLayoutItem;

class QToolBarHandle : public QWidget
{
    Q_OBJECT
public:
    Qt::Orientation orient;
    struct DragState {
        QPoint pressPos;
        bool dragging;
        QLayoutItem *widgetItem;
    };
    DragState *state;

    explicit QToolBarHandle(QToolBar *parent);

    Qt::Orientation orientation() const;

    QSize sizeHint() const;

    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void paintEvent(QPaintEvent *e);
    void initStyleOption(QStyleOption *option) const;

    void setWindowState(bool floating, bool unplug = false, const QRect &rect = QRect());
    void initDrag(const QPoint &pos);
    void startDrag();
    void endDrag();

    void unplug(const QRect &r);
    void plug(const QRect &r);

public Q_SLOTS:
    void setOrientation(Qt::Orientation orientation);
};

/* A QToolBar uses a regular QBoxLayout. So its sizeHint() and minimumSize() return values
   pertaining to the currently visible buttons, not the correct values for the tool bar.
   This layout item returns the correct sizes. */

class QToolBarWidgetItem : public QWidgetItem
{
public:
    QToolBarWidgetItem(QToolBar *toolBar);
    virtual QSize sizeHint() const;
    virtual QSize minimumSize() const;
};

#endif // QT_NO_TOOLBAR

#endif // QDYNAMICTOOLBARHANDLE_P_H
