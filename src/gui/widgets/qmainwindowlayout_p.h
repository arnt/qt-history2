#ifndef QMAINWINDOWLAYOUT_P_H
#define QMAINWINDOWLAYOUT_P_H

#include "q4mainwindow.h"

#include <qlayout.h>

class QToolBar;
class QDockWindow;
class QDockWindowLayout;

class QMainWindowLayout : public QLayout
{
    Q_OBJECT

 public:
    QMainWindowLayout(QMainWindow *mainwindow);
    ~QMainWindowLayout();

    QLayoutItem *statusbar;
    QStatusBar *statusBar() const;
    void setStatusBar(QStatusBar *sb);

    QWidget *centerWidget() const;
    void setCenterWidget(QWidget *cw);

    QDockWindowLayout *layoutForArea(DockWindowArea area);

    void add(QToolBar *toolbar, ToolBarArea area, bool linebreak);
    void add(QToolBar *toolbar, int where, bool linebreak, const QPoint &offset = QPoint());

    // QLayout interface
    void addItem(QLayoutItem *item);
    void setGeometry(const QRect &r);
    QLayoutItem *itemAt(int index) const;
    QLayoutItem *takeAt(int index);

    QSize sizeHint() const;
    QSize minimumSize() const;

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

    int locateDockWindow(QDockWindow *dockwindow, const QPoint &mouse) const;
    QRect placeDockWindow(QDockWindow *dockwindow, const QRect &r, const QPoint &mouse);
    void dropDockWindow(QDockWindow *dockwindow, const QRect &r, const QPoint &mouse);

    int locateToolBar(const QPoint &mouse) const;
    QRect placeToolBar(QToolBar *toolbar, const QPoint &mouse, const QPoint &offset);
    void dropToolBar(QToolBar *toolbar, const QPoint &mouse, const QPoint &offset);

    struct ToolBarLayoutInfo;
    void placeToolBarInfo(const ToolBarLayoutInfo &newinfo);
    void removeToolBarInfo(QToolBar *toolbar);

    // dock/center-widget layout data
    DockWindowArea corners[4];
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
	int where;

	uint linebreak : 1;
	uint is_dummy : 1;
    };

    typedef QList<ToolBarLayoutInfo> ToolBarLineInfo;
    QList<ToolBarLineInfo> tb_layout_info, *save_tb_layout_info;
};

#endif // QMAINWINDOWLAYOUT_P_H
