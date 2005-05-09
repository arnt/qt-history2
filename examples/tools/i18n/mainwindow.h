/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ Trolltech AS. All rights reserved.
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

#include <QMainWindow>

class QAction;
class QGroupBox;
class QLabel;
class QListWidget;
class QMenu;
class QRadioButton;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();

private:
    void createGroupBox();

    QWidget *centralWidget;
    QLabel *label;
    QGroupBox *groupBox;
    QListWidget *listWidget;
    QRadioButton *perspectiveRadioButton;
    QRadioButton *isometricRadioButton;
    QRadioButton *obliqueRadioButton;
    QMenu *fileMenu;
    QAction *exitAction;
};

#endif
