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

#ifndef APPLICATION_H
#define APPLICATION_H

#include <qmainwindow.h>
#include <qptrlist.h>

class QTextEdit;
class QToolBar;
class QPopupMenu;
class QWorkspace;
class QPopupMenu;
class QMovie;

class MDIWindow: public QMainWindow
{
    Q_OBJECT
public:
    MDIWindow( QWidget* parent, const char* name, Qt::WFlags wflags );
    ~MDIWindow();

    void load( const QString& fn );
    void save();
    void saveAs();
    void print( QPrinter* );

protected:
    void closeEvent( QCloseEvent * );

signals:
    void message(const QString&, int );

private:
    QTextEdit* medit;
    QMovie * mmovie;
    QString filename;
};


class ApplicationWindow: public QMainWindow
{
    Q_OBJECT
public:
    ApplicationWindow();
    ~ApplicationWindow();

protected:
    void closeEvent( QCloseEvent * );

private slots:
    MDIWindow* newDoc();
    void load();
    void save();
    void saveAs();
    void print();
    void closeWindow();
    void tileHorizontal();

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
