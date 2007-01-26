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
        QLayoutItem *widgetItem;
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

//    QMainWindow *findMainWindow(QWidget *widget) const;
    QRect undockedGeometry;

    void mousePressEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void setWindowState(bool floating, bool unplug = false, const QRect &rect = QRect());
    void nonClientAreaMouseEvent(QMouseEvent *event);
    void initDrag(const QPoint &pos, bool nca);
    void startDrag();
    void endDrag(bool abort = false);
    void moveEvent(QMoveEvent *event);

    void unplug(const QRect &rect);
    void plug(const QRect &rect);
};

class QDockWidgetLayout : public QLayout
{
public:
    QDockWidgetLayout(QWidget *parent = 0);
    void addItem(QLayoutItem *item);
    QLayoutItem *itemAt(int index) const;
    QLayoutItem *takeAt(int index);
    int count() const;

    QSize minimumSize() const;
    QSize sizeHint() const;

    QSize sizeFromContent(const QSize &content, bool floating) const;

    void setGeometry(const QRect &r);

    enum Role { Content, CloseButton, FloatButton, RoleCount };
    QWidget *widget(Role r) const;
    void setWidget(Role r, QWidget *w);
    QLayoutItem *item(Role r) const;

    QRect titleArea() const { return _titleArea; }

    int minimumTitleWidth() const;
    int titleHeight() const;
    void updateMaxSize();

    void setVerticalTitleBars(bool b);

    bool verticalTitleBar;

private:
    QVector<QLayoutItem*> item_list;
    QRect _titleArea;
};

/* The size hints of a QDockWidget will depend on wether it is docked or not.
   This layout item always returns the size hints as if the dock widget was docked. */

class QDockWidgetItem : public QWidgetItem
{
public:
    QDockWidgetItem(QDockWidget *dockWidget);
    QSize minimumSize() const;
    QSize maximumSize() const;
    QSize sizeHint() const;

private:
    inline QLayoutItem *dockWidgetChildItem() const;
    inline QDockWidgetLayout *dockWidgetLayout() const;
};

inline QLayoutItem *QDockWidgetItem::dockWidgetChildItem() const
{
    if (QDockWidgetLayout *layout = dockWidgetLayout())
        return layout->item(QDockWidgetLayout::Content);
    return 0;
}

inline QDockWidgetLayout *QDockWidgetItem::dockWidgetLayout() const
{
    QWidget *w = const_cast<QDockWidgetItem*>(this)->widget();
    if (w != 0)
        return qobject_cast<QDockWidgetLayout*>(w->layout());
    return 0;
}

#endif // QT_NO_DOCKWIDGET

#endif // QDYNAMICDOCKWIDGET_P_H
