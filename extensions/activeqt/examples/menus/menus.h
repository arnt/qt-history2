/****************************************************************************
** $Id: $
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#ifndef MENUS_H
#define MENUS_H

#include <qmainwindow.h>

class QTextEdit;

class QMenus : public QMainWindow
{
    Q_OBJECT

public:
    QMenus( QWidget *parent = 0, const char *name = 0 );

public slots:
    void fileOpen();
    void fileSave();

    void editNormal();
    void editBold();
    void editUnderline();

    void helpAbout();
    void helpAboutQt();

private:
    QTextEdit *editor;
};

#endif // MENUS_H
