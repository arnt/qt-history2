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

#ifndef QT_NO_MAINWINDOW

#include <qlayout.h>
#include <qvector.h>

class QToolBar;
class QDockWidget;
class QDockWidgetLayout;

class QMainWindowLayout : public QLayout
{
    Q_OBJECT

public:
    explicit QMainWindowLayout(QMainWindow *mainwindow);
    ~QMainWindowLayout();

    QLayoutItem *statusbar;
#ifndef QT_NO_STATUSBAR
    QStatusBar *statusBar() const;
    void setStatusBar(QStatusBar *sb);
#endif
    
    QWidget *centralWidget() const;
    void setCentralWidget(QWidget *cw);

#ifndef QT_NO_TOOLBAR
    void addToolBarBreak(Qt::ToolBarArea area);
    void insertToolBarBreak(QToolBar *before);
    void addToolBar(Qt::ToolBarArea area, QToolBar *toolbar, bool needAddChildWidget = true);
    void insertToolBar(QToolBar *before, QToolBar *toolbar);
    Qt::ToolBarArea toolBarArea(QToolBar *toolbar) const;
#endif
    
#ifndef QT_NO_DOCKWIDGET    
    QDockWidgetLayout *layoutForArea(Qt::DockWidgetArea area);
    void addDockWidget(Qt::DockWidgetArea area, QDockWidget *dockwidget,
                       Qt::Orientation orientation);
    void splitDockWidget(QDockWidget *after, QDockWidget *dockwidget,
                         Qt::Orientation orientation);
    Qt::DockWidgetArea dockWidgetArea(QDockWidget *dockwidget) const;
#endif
    enum { // sentinel values used to validate state data
        VersionMarker = 0xff,
        ToolBarStateMarker = 0xfe,
        DockWidgetStateMarker = 0xfd
    };
    void saveState(QDataStream &stream) const;
    bool restoreState(QDataStream &stream);

    // QLayout interface
    void addItem(QLayoutItem *item);
    void setGeometry(const QRect &r);
    QLayoutItem *itemAt(int index) const;
    QLayoutItem *takeAt(int index);
    int count() const;

    QSize sizeHint() const;
    QSize minimumSize() const;
    mutable QSize szHint;
    mutable QSize minSize;

    void invalidate();

    void removeRecursive(QDockWidget *dockwidget);


    // utility functions

    QInternal::RelayoutType relayout_type;
    void relayout(QInternal::RelayoutType type = QInternal::RelayoutNormal);

    void saveLayoutInfo();
    void resetLayoutInfo();
    void discardLayoutInfo();

    void beginConstrain();
    void endConstrain();
    int constrain(QDockWidgetLayout *dock, int delta);

    Qt::DockWidgetArea locateDockWidget(QDockWidget *dockwidget, const QPoint &mouse) const;
    QRect placeDockWidget(QDockWidget *dockwidget, const QRect &r, const QPoint &mouse);
    void dropDockWidget(QDockWidget *dockwidget, const QRect &r, const QPoint &mouse);

#ifndef QT_NO_TOOLBAR
    int locateToolBar(QToolBar *toolbar, const QPoint &mouse) const;
    void dropToolBar(QToolBar *toolbar, const QPoint &mouse, const QPoint &offset);

    void removeToolBarInfo(QToolBar *toolbar);
#endif
    
    // dock/center-widget layout data
    Qt::DockWidgetArea corners[4];
    struct QMainWindowLayoutInfo
    {
	QLayoutItem *item;
	QLayoutItem *sep;
	QSize size;
	uint is_dummy : 1;
    };
    QVector<QMainWindowLayoutInfo> layout_info, *save_layout_info;

#ifndef QT_NO_TOOLBAR
    // toolbar layout data
    struct ToolBarLayoutInfo
    {
	QLayoutItem *item;
	QPoint pos;
        QSize size;
	QPoint offset;
    };

    struct ToolBarLineInfo
    {
        int pos;
        QList<ToolBarLayoutInfo> list;
    };

    /*
      helpers to return the index of next/prev visible toolbar... they
      return -1 if none is found
    */
    static int nextVisible(int index, const ToolBarLineInfo &lineInfo);
    static int prevVisible(int index, const ToolBarLineInfo &lineInfo);

    QList<ToolBarLineInfo> tb_layout_info, *save_tb_layout_info;
#endif
};

#endif // QT_NO_MAINWINDOW
#endif // QMAINWINDOWLAYOUT_P_H
