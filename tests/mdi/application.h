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

#ifndef APPLICATION_H
#define APPLICATION_H

#include <qmainwindow.h>
#include <qptrlist.h>

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
    
    void load( const QString& fn );
    void save();
    void saveAs();
    void print( QPrinter* );
    
signals:
    void message(const QString&, int );

private:
    QMultiLineEdit* medit;
    QString filename;
};


class ApplicationWindow: public QMainWindow
{
    Q_OBJECT
public:
    ApplicationWindow();
    ~ApplicationWindow();

private slots:
    MDIWindow* newDoc();
    void load();
    void save();
    void saveAs();
    void print();
    void closeClient();

    void about();
    void aboutQt();

    void windowsMenuAboutToShow();
    void windowsMenuActivated( int id );

private:
    QPrinter *printer;
    QWorkspace* ws;
    QToolBar *fileTools;
    QPopupMenu* windowsMenu;
};


#endif
