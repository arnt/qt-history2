/****************************************************************************
** $Id: //depot/qt/main/examples/qfileiconview/mainwindow.h#1 $
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#ifndef MAINWIN_H
#define MAINWIN_H

#include <qmainwindow.h>

class QtFileIconView;
class DirectoryView;

class FileMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    FileMainWindow();

    QtFileIconView *fileView() { return fileview; }
    DirectoryView *dirList() { return dirlist; }

    void show();
    
protected:
    void setup();

    QtFileIconView *fileview;
    DirectoryView *dirlist;

protected slots:
    void directoryChanged( const QString & );

};

#endif
