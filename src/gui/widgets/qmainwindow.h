#ifndef QMAINWINDOW_H
#define QMAINWINDOW_H

#include <qwidget.h>

class QDockWindow;
class QMainWindowPrivate;
class QMenuBar;
class QStatusBar;
class QToolBar;

class Q_GUI_EXPORT QMainWindow : public QWidget
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QMainWindow);

public:
    QMainWindow(QWidget *parent = 0, WFlags flags = 0);
    ~QMainWindow();

    QMenuBar *menuBar() const;
    void setMenuBar(QMenuBar *menubar);

    QStatusBar *statusBar() const;
    void setStatusBar(QStatusBar *statusbar);

    QWidget *centerWidget() const;
    void setCenterWidget(QWidget *widget);

    void setCorner(Qt::Corner corner, Qt::DockWindowArea area);
    Qt::DockWindowArea corner(Qt::Corner corner) const;

    void setDockWindowState(const QString &state);
    QString dockWindowState() const;

protected:
    void childEvent(QChildEvent *event);
    bool event(QEvent *event);
};

#endif // QMAINWINDOW_H
