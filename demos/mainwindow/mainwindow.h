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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <qmainwindow.h>

class ToolBar;

class MainWindow : public QMainWindow
{
    Q_OBJECT

    ToolBar *toolbar;
    QAction *dockWindowActions;

public:
    MainWindow(QWidget *parent = 0, Qt::WFlags flags = 0);

public slots:
    void actionTriggered(QAction *action);

private:
    void setupActions();
    void setupToolBar();
    void setupMenuBar();
    void setupDockWindows();
};

#endif // MAINWINDOW_H
