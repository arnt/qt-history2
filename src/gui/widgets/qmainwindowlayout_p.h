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

#ifndef QMAINWINDOWLAYOUT_P_H
#define QMAINWINDOWLAYOUT_P_H

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

#include "qmainwindow.h"

#include <qlayout.h>
#include <qvector.h>

class QToolBar;
class QDockWindow;
class QDockWindowLayout;

class QMainWindowLayout : public QLayout
{
    Q_OBJECT

public:
    explicit QMainWindowLayout(QMainWindow *mainwindow);
    ~QMainWindowLayout();

    QLayoutItem *statusbar;
    QStatusBar *statusBar() const;
    void setStatusBar(QStatusBar *sb);

    QWidget *centralWidget() const;
    void setCentralWidget(QWidget *cw);

    void addToolBarBreak(Qt::ToolBarArea area);
    void insertToolBarBreak(QToolBar *before);
    void addToolBar(Qt::ToolBarArea area, QToolBar *toolbar);
    void insertToolBar(QToolBar *before, QToolBar *toolbar);
    Qt::ToolBarArea toolBarArea(QToolBar *toolbar) const;

    QDockWindowLayout *layoutForArea(Qt::DockWindowArea area);
    void addDockWindow(Qt::DockWindowArea area, QDockWindow *dockwindow,
                       Qt::Orientation orientation);
    void splitDockWindow(QDockWindow *after, QDockWindow *dockwindow,
                         Qt::Orientation orientation);
    Qt::DockWindowArea dockWindowArea(QDockWindow *dockwindow) const;

    enum { // sentinel values used to validate state data
        VersionMarker = 0xff,
        ToolBarStateMarker = 0xfe,
        DockWindowStateMarker = 0xfd
    };
    void saveState(QDataStream &stream) const;
    bool restoreState(QDataStream &stream);

    // QLayout interface
    void addItem(QLayoutItem *item);
    void setGeometry(const QRect &r);
    QLayoutItem *itemAt(int index) const;
    QLayoutItem *takeAt(int index);

    QSize sizeHint() const;
    QSize minimumSize() const;
    mutable QSize szHint;
    mutable QSize minSize;

    void invalidate();

    void removeRecursive(QDockWindow *dockwindow);


    // utility functions

    QInternal::RelayoutType relayout_type;
    void relayout(QInternal::RelayoutType type = QInternal::RelayoutNormal);

    void saveLayoutInfo();
    void resetLayoutInfo();
    void discardLayoutInfo();

    void beginConstrain();
    void endConstrain();
    int constrain(QDockWindowLayout *dock, int delta);

    Qt::DockWindowAreas locateDockWindow(QDockWindow *dockwindow, const QPoint &mouse) const;
    QRect placeDockWindow(QDockWindow *dockwindow, const QRect &r, const QPoint &mouse);
    void dropDockWindow(QDockWindow *dockwindow, const QRect &r, const QPoint &mouse);

    int locateToolBar(QToolBar *toolbar, const QPoint &mouse) const;
    void dropToolBar(QToolBar *toolbar, const QPoint &mouse, const QPoint &offset);

    void removeToolBarInfo(QToolBar *toolbar);

    // dock/center-widget layout data
    Qt::DockWindowArea corners[4];
    struct QMainWindowLayoutInfo
    {
	QLayoutItem *item;
	QLayoutItem *sep;
	QSize size;
	uint is_dummy : 1;
    };
    QVector<QMainWindowLayoutInfo> layout_info, *save_layout_info;

    // toolbar layout data
    struct ToolBarLayoutInfo
    {
	QLayoutItem *item;
	QPoint pos;
	QSize size;
	QPoint offset;
        uint is_dummy : 1;
    };

    struct ToolBarLineInfo
    {
        int pos;
        QList<ToolBarLayoutInfo> list;
    };

    QList<ToolBarLineInfo> tb_layout_info, *save_tb_layout_info;
};

#endif // QMAINWINDOWLAYOUT_P_H
