/****************************************************************************
** $Id: //depot/qt/main/tests/mainwindow/application.h#1 $
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
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
class QComboBox;
class QPushButton;

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
    void toggleOpaque();
    void hideToolbar();
    
    void about();
    void aboutQt();

    void orientationChanged();

private:
    QToolBar *createToolbar( const QString &name, bool nl );

    QComboBox *cb;
    QPushButton *pb;

    QMultiLineEdit *e;
    QString filename;

    int fullScreenId;
    int justId;
    int bigpixId;
    int textlabelid;
    int opaqueId;

};


#endif
