#ifndef Q4MAINWINDOWLAYOUT_P_H
#define Q4MAINWINDOWLAYOUT_P_H

#include "q4mainwindow.h"

#include <qlayout.h>

class Q4ToolBar;
class Q4DockWindow;
class Q4DockWindowLayout;

class Q4MainWindowLayout : public QLayout
{
    Q_OBJECT

 public:
    Q4MainWindowLayout(Q4MainWindow *mainwindow);
    ~Q4MainWindowLayout();

    QLayoutItem *statusbar;
    QStatusBar *statusBar() const;
    void setStatusBar(QStatusBar *sb);

    QWidget *centerWidget() const;
    void setCenterWidget(QWidget *cw);

    Q4DockWindowLayout *layoutForArea(DockWindowArea area);

    void add(Q4ToolBar *toolbar, ToolBarArea area, bool linebreak);
    void add(Q4ToolBar *toolbar, int where, bool linebreak, const QPoint &offset = QPoint());

    // QLayout interface
    void addItem(QLayoutItem *item);
    void setGeometry(const QRect &r);
    QLayoutItem *itemAt(int index) const;
    QLayoutItem *takeAt(int index);

    QSize sizeHint() const;
    QSize minimumSize() const;

    void invalidate();

    void removeRecursive(Q4DockWindow *dockwindow);




    // utility functions

    QInternal::RelayoutType relayout_type;
    void relayout(QInternal::RelayoutType type = QInternal::RelayoutNormal);

    void saveLayoutInfo();
    void resetLayoutInfo();
    void discardLayoutInfo();

    void beginConstrain();
    void endConstrain();
    int constrain(Q4DockWindowLayout *dock, int delta);

    int locateDockWindow(Q4DockWindow *dockwindow, const QPoint &mouse) const;
    QRect placeDockWindow(Q4DockWindow *dockwindow, const QRect &r, const QPoint &mouse);
    void dropDockWindow(Q4DockWindow *dockwindow, const QRect &r, const QPoint &mouse);

    int locateToolBar(const QPoint &mouse) const;
    QRect placeToolBar(Q4ToolBar *toolbar, const QPoint &mouse, const QPoint &offset);
    void dropToolBar(Q4ToolBar *toolbar, const QPoint &mouse, const QPoint &offset);

    struct ToolBarLayoutInfo;
    void placeToolBarInfo(const ToolBarLayoutInfo &newinfo);
    void removeToolBarInfo(Q4ToolBar *toolbar);

    // dock/center-widget layout data
    DockWindowArea corners[4];
    struct Q4MainWindowLayoutInfo
    {
	QLayoutItem *item;
	QLayoutItem *sep;
	QSize size;
	uint is_dummy : 1;
    };
    QVector<Q4MainWindowLayoutInfo> layout_info, *save_layout_info;

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

#endif // Q4MAINWINDOWLAYOUT_P_H
