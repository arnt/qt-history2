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

#ifndef QDYNAMICDOCKWIDGET_P_H
#define QDYNAMICDOCKWIDGET_P_H

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

#include "QtGui/qstyleoption.h"
#include "private/qwidget_p.h"
#include "QtGui/qboxlayout.h"

#ifndef QT_NO_DOCKWIDGET

class QGridLayout;
class QWidgetResizeHandler;
class QRubberBand;
class QDockWidgetTitleButton;
class QSpacerItem;
class QDockWidgetItem;

class QDockWidgetPrivate : public QWidgetPrivate
{
    Q_DECLARE_PUBLIC(QDockWidget)

    struct DragState {
        QPoint pressPos;
        bool dragging;
        QWidgetItem *widgetItem;
        QList<int> pathToGap;
        bool ownWidgetItem;
        bool nca;
    };

public:
    inline QDockWidgetPrivate()
	: QWidgetPrivate(), state(0),
          features(QDockWidget::DockWidgetClosable
                   | QDockWidget::DockWidgetMovable
                   | QDockWidget::DockWidgetFloatable),
          allowedAreas(Qt::AllDockWidgetAreas)
    { }

    void init();
    void _q_toggleView(bool); // private slot
    void _q_toggleTopLevel(); // private slot

    void updateButtons();
    DragState *state;

    QDockWidget::DockWidgetFeatures features;
    Qt::DockWidgetAreas allowedAreas;

#ifdef Q_WS_X11
    QWidgetResizeHandler *resizer;
#endif

#ifndef QT_NO_ACTION
    QAction *toggleViewAction;
#endif

    QMainWindow *findMainWindow(QWidget *widget) const;

    void mousePressEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void setWindowState(bool floating, bool unplug = false, const QRect &rect = QRect());
    void nonClientAreaMouseEvent(QMouseEvent *event);
    void initDrag(const QPoint &pos, bool nca);
    void startDrag();
    void endDrag();
    void moveEvent(QMoveEvent *event);

    void unplug(const QRect &rect);
    void plug(const QRect &rect);
};

#endif // QT_NO_DOCKWIDGET

#endif // QDYNAMICDOCKWIDGET_P_H
