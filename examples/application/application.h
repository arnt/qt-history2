/****************************************************************************
** $Id: //depot/qt/main/examples/application/application.h#2 $
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
#include <qstring.h>

class QMultiLineEdit;
class QToolBar;
class QPopupMenu;

class ApplicationWindow: public QMainWindow
{
    Q_OBJECT
public:
    ApplicationWindow();
    ~ApplicationWindow();

protected:
    bool eventFilter( QObject *, QEvent * );

private slots:
    void newDoc();
    void load();
    void load( const QString &fileName );
    void save();
    void saveAs();
    void print();
    void closeDoc();

    void about();
    void aboutQt();

    void toggleMenuBar();
    void toggleStatusBar();
    void toggleToolBar();

private:
    QPrinter *printer;
    QMultiLineEdit *e;
    QToolBar *fileTools;
    QPopupMenu *controls;
    int mb, tb, sb;
    QString filename;
};


#endif
