/****************************************************************************
** $Id: //depot/qt/main/examples/qfileiconview/mainwindow.h#2 $
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
class QProgressBar;
class QLabel;

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
    QProgressBar *progress;
    QLabel *label;
    
protected slots:
    void directoryChanged( const QString & );
    void slotStartReadDir( int dirs );
    void slotReadNextDir();
    void slotReadDirDone();
    
};

#endif
