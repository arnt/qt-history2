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

#ifndef MAINWIN_H
#define MAINWIN_H

#include <qmainwindow.h>

class QtFileIconView;
class DirectoryView;
class QProgressBar;
class QLabel;
class QComboBox;
class QToolButton;

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
    void setPathCombo();

    QtFileIconView *fileview;
    DirectoryView *dirlist;
    QProgressBar *progress;
    QLabel *label;
    QComboBox *pathCombo;
    QToolButton *upButton, *mkdirButton;

protected slots:
    void directoryChanged( const QString & );
    void slotStartReadDir( int dirs );
    void slotReadNextDir();
    void slotReadDirDone();
    void cdUp();
    void newFolder();
    void changePath( const QString &path );
    void enableUp();
    void disableUp();
    void enableMkdir();
    void disableMkdir();

};

#endif
