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
class QSignalMapper;

class MainWindow : public QMainWindow
{
    Q_OBJECT

    QList<ToolBar*> toolBars;
    QMenu *dockWidgetMenu;
    QMenu *mainWindowMenu;
    QSignalMapper *mapper;

public:
    MainWindow(QWidget *parent = 0, Qt::WindowFlags flags = 0);

public slots:
    void actionTriggered(QAction *action);
    void saveLayout();
    void loadLayout();
    void setCorner(int id);
    void switchLayoutDirection();
    void setDockOptions();

private:
    void setupToolBar();
    void setupMenuBar();
    void setupDockWidgets();
};

#endif
