/****************************************************************************
** $Id: //depot/qt/main/tests/mdi/application.h#3 $
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
#include <qlist.h>

class QMultiLineEdit;
class QToolBar;
class QPopupMenu;
class QWorkspace;
class QPopupMenu;

class MDIWindow: public QMainWindow
{
    Q_OBJECT
public:
    MDIWindow( QWidget* parent, const char* name, int wflags );
    ~MDIWindow();
protected:
    void closeEvent( QCloseEvent* );

private:
    QMultiLineEdit* medit;
};


class ApplicationWindow: public QMainWindow
{
    Q_OBJECT
public:
    ApplicationWindow();
    ~ApplicationWindow();

protected:
    void closeEvent( QCloseEvent* );

private slots:
    void newDoc();
    void load();
    void load( const char *fileName );
    void save();
    void saveAs();
    void print();

    void about();
    void aboutQt();

    void windowsMenuAboutToShow();
    void windowsMenuActivated( int id );

    void childDestroyed();

private:
    QPrinter *printer;
    QWorkspace* ws;
    QToolBar *fileTools;
    QString filename;
    QPopupMenu* windowsMenu;
    QList<QWidget> windows;
};


#endif
