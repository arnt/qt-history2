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
#include <qdialog.h>
#include <qptrlist.h>
#include <qgl.h>

class QMultiLineEdit;
class QToolBar;
class QPopupMenu;
class QWorkspace;
class QCheckBox;

class GLInfo;
class MDIWindow: public QWidget
{
    Q_OBJECT
public:
    MDIWindow( const QGLFormat &f, QWidget* parent, const char* name );
    ~MDIWindow();

private:
    QGLWidget *gl;
};

class ControlWindow: public QWidget
{
    Q_OBJECT
public:
    ControlWindow( QWidget* parent, const char* name );
    ~ControlWindow() {};
protected slots:
    void create();
signals:
    void newCreate( const QGLFormat &f ); 
protected:
    QCheckBox *db, *depth, *rgba, *alpha, *accum, *stencil;
};

class ApplicationWindow: public QMainWindow
{
    Q_OBJECT
public:
    ApplicationWindow();
    ~ApplicationWindow();

private slots:
    MDIWindow* newDoc( const QGLFormat &f );
    void info(); 
    void about();
    void aboutQt();

private:
    QPrinter *printer;
    QWorkspace* ws;
    QPopupMenu* windowsMenu;
    GLInfo * glInfo;
};


#endif
