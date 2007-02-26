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

#ifndef QDYNAMICTOOLBAR_P_H
#define QDYNAMICTOOLBAR_P_H

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

#include "qtoolbar.h"
#include "QtGui/qaction.h"
#include "private/qwidget_p.h"

#ifndef QT_NO_TOOLBAR

class QToolBarLayout;

class QToolBarPrivate : public QWidgetPrivate
{
    Q_DECLARE_PUBLIC(QToolBar)

public:
    inline QToolBarPrivate()
        : explicitIconSize(false), explicitToolButtonStyle(false), movable(false),
          allowedAreas(Qt::AllToolBarAreas), orientation(Qt::Horizontal),
          toolButtonStyle(Qt::ToolButtonIconOnly),
          layout(0), state(0)
    { }

    void init();
    void actionTriggered();
    void _q_toggleView(bool b);
    void _q_updateIconSize(const QSize &sz);
    void _q_updateToolButtonStyle(Qt::ToolButtonStyle style);

    bool explicitIconSize;
    bool explicitToolButtonStyle;
    bool movable;
    Qt::ToolBarAreas allowedAreas;
    Qt::Orientation orientation;
    Qt::ToolButtonStyle toolButtonStyle;
    QSize iconSize;

    QAction *toggleViewAction;

    QToolBarLayout *layout;

    struct DragState {
        QPoint pressPos;
        bool dragging;
        QLayoutItem *widgetItem;
    };
    DragState *state;

    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);

    void setWindowState(bool floating, bool unplug = false, const QRect &rect = QRect());
    void initDrag(const QPoint &pos);
    void startDrag();
    void endDrag();

    void unplug(const QRect &r);
    void plug(const QRect &r);
};

#endif // QT_NO_TOOLBAR

#endif // QDYNAMICTOOLBAR_P_H
