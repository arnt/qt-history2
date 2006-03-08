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

#ifndef QDOCKWIDGET_P_H
#define QDOCKWIDGET_P_H

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

//We need access to insertItem and addChildWidget in QDockWidget
class QDockWidgetBoxLayout : public QVBoxLayout
{
public:
#ifdef Q_NO_USING_KEYWORD
    inline void addChildWidget(QWidget *widget) { QVBoxLayout::addChildWidget(widget); }
    inline void insertItem(int index, QLayoutItem *item) { QVBoxLayout::insertItem(index, item); }
#else
    using QVBoxLayout::addChildWidget;
    using QVBoxLayout::insertItem;
#endif
};

class QDockWidgetPrivate : public QWidgetPrivate
{
    Q_DECLARE_PUBLIC(QDockWidget)

    struct DragState {
        QRubberBand *rubberband;
        QRect origin;   // starting position
        QRect current;  // current size of the dockwidget (can be either placed or floating)
        QPoint offset;
        bool canDrop;
    };

public:
    inline QDockWidgetPrivate()
	: QWidgetPrivate(), state(0), widget(0),
          features(QDockWidget::DockWidgetClosable
                   | QDockWidget::DockWidgetMovable
                   | QDockWidget::DockWidgetFloatable),
          allowedAreas(Qt::AllDockWidgetAreas), top(0), box(0),
          topSpacer(0), floatButton(0), closeButton(0), resizer(0)
    { }

    void init();
    void _q_toggleView(bool); // private slot
    void _q_toggleTopLevel(); // private slot

    QStyleOptionDockWidget getStyleOption();

    void updateButtons();
    void relayout();
    DragState *state;

    QWidget *widget;

    QDockWidget::DockWidgetFeatures features;
    Qt::DockWidgetAreas allowedAreas;

    QGridLayout *top;
    QDockWidgetBoxLayout *box;
    QSpacerItem *topSpacer;
    QRect titleArea;
    QDockWidgetTitleButton *floatButton;
    QDockWidgetTitleButton *closeButton;

    QWidgetResizeHandler *resizer;
#ifndef QT_NO_ACTION
    QAction *toggleViewAction;
#endif

    QMainWindow *findMainWindow(QWidget *widget) const;

    void mousePressEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
};

//This class adds the margin space to dockwidgets when they are not floating.
//We do this to ensure that the minimum sizeHint does not change when floating
class QDockWidgetItem : public QWidgetItem
{
public:
    inline QDockWidgetItem(QWidget *w)
        : QWidgetItem(w)
    { }

    QSize adjusted(QSize s) const
    {
        QDockWidgetItem *w = const_cast<QDockWidgetItem *>(this);
        QDockWidget * dockWidget = qobject_cast<QDockWidget *>(w->widget()->parentWidget());
        Q_ASSERT_X(dockWidget != 0, "QDockWidget", "Internal error");
        if(!dockWidget->isFloating() || (dockWidget->d_func()->state && dockWidget->d_func()->state->canDrop )) {
            int fw = dockWidget->style()->pixelMetric(QStyle::PM_DockWidgetFrameWidth);
            s += QSize(fw*2, fw*2);
        }
        return s;
    }

    inline QSize sizeHint() const
    {
        return adjusted(QWidgetItem::sizeHint());
    }

    inline QSize minimumSize() const
    {
        return adjusted(QWidgetItem::minimumSize());
    }
};

#endif // QT_NO_DOCKWIDGET

#endif // QDOCKWIDGET_P_H
