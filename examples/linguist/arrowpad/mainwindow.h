/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ $TROLLTECH$. All rights reserved.
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

QT_DECLARE_CLASS(QAction)
QT_DECLARE_CLASS(QMenu)
class ArrowPad;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();

private:
    ArrowPad *arrowPad;
    QMenu *fileMenu;
    QAction *exitAct;
};

#endif
