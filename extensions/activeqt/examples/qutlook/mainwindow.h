/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of an example program for Qt.
** EDITIONS: NOLIMITS
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef AB_MAINWINDOW_H
#define AB_MAINWINDOW_H

#include <qmainwindow.h>
#include <qstring.h>

class QToolBar;
class QPopupMenu;
class ABCentralWidget;

class ABMainWindow: public QMainWindow
{
    Q_OBJECT

public:
    ABMainWindow();
    ~ABMainWindow();

protected slots:
    void fileNew();
    void fileOpen();
    void fileSave();
    void fileSaveAs();
    void filePrint();
    void closeWindow();

protected:
    void setupMenuBar();
    void setupFileTools();
    void setupStatusBar();
    void setupCentralWidget();

    QToolBar *fileTools;
    QString filename;
    ABCentralWidget *view;

};


#endif
