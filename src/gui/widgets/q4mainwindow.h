#ifndef QMAINWINDOW_H
#define QMAINWINDOW_H

#include <qwidget.h>

class Q4DockWindow;
class Q4MainWindowPrivate;
class QMenuBar;
class QStatusBar;
class Q4ToolBar;

class Q_GUI_EXPORT Q4MainWindow : public QWidget
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Q4MainWindow);

public:
    Q4MainWindow(QWidget *parent = 0, WFlags flags = 0);
    ~Q4MainWindow();

    QMenuBar *menuBar() const;
    void setMenuBar(QMenuBar *menubar);

    QStatusBar *statusBar() const;
    void setStatusBar(QStatusBar *statusbar);

    QWidget *centerWidget() const;
    void setCenterWidget(QWidget *widget);

    void setCorner(Corner corner, DockWindowArea area);
    DockWindowArea corner(Corner corner) const;

    void setDockWindowState(const QString &state);
    QString dockWindowState() const;

protected:
    void childEvent(QChildEvent *event);
    bool event(QEvent *event);
};

#endif // QMAINWINDOW_H
