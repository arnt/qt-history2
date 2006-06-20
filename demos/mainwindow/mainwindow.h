/****************************************************************************
**
** Copyright (C) 2004-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class ToolBar;
class QMenu;

class MainWindow : public QMainWindow
{
    Q_OBJECT

    ToolBar *toolbar;
    QMenu *dockWidgetMenu;

public:
    MainWindow(QWidget *parent = 0, Qt::WFlags flags = 0);

public slots:
    void actionTriggered(QAction *action);
    void saveLayout();
    void loadLayout();

private:
    void setupToolBar();
    void setupMenuBar();
    void setupDockWidgets();
};

#endif
