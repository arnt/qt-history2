/****************************************************************************
** $Id: //depot/qt/main/tests/mainwindow/application.h#1 $
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#ifndef APPLICATION_H
#define APPLICATION_H

#include <qmainwindow.h>

class QMultiLineEdit;
class QToolBar;
class QPopupMenu;

class ApplicationWindow: public QMainWindow
{
    Q_OBJECT
public:
    ApplicationWindow();
    ~ApplicationWindow();


private slots:
    void newDoc();
    void load();
    void load( const char *fileName );
    void save();
    void saveAs();
    void print();

    void load2();
    void save2();
    void print2();


    void toggleFullScreen();
    void toggleJust();
    void toggleBigpix();
    void toggleTextLabel();

    void about();
    void aboutQt();

private:
    QToolBar *createToolbar( const QString &name, bool nl );

    QMultiLineEdit *e;
    QString filename;

    int fullScreenId;
    int justId;
    int bigpixId;
    int textlabelid;

};


#endif
