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
    QMainWindow(QWidget *parent = 0, Qt::WFlags flags = 0);
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

#ifdef QT_COMPAT
    QT_COMPAT_CONSTRUCTOR QMainWindow(QWidget *parent, const char *name, Qt::WFlags flags = 0);
    inline QT_COMPAT void setCentralWidget(QWidget *w) { setCenterWidget(w); }
    inline QT_COMPAT QWidget *centralWidget() const { return centerWidget(); }
#endif
protected:
    void childEvent(QChildEvent *event);
    bool event(QEvent *event);
};

#endif // QMAINWINDOW_H
