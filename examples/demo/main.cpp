/****************************************************************************
** $Id: //depot/qt/main/examples/demo/main.cpp#19 $
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "frame.h"
#include "graph.h"
#include "display.h"
#include "icons.h"

#include "textdrawing/textedit.h"
#include "textdrawing/helpwindow.h"

#if defined(QT_MODULE_OPENGL)
#include "opengl/glworkspace.h"
#endif

#include <stdlib.h>

#include <qapplication.h>
#include <qimage.h>
#include <qwindowsstyle.h>
#include <qtabwidget.h>
#include <qfont.h>
#include <qworkspace.h>

int main( int argc, char **argv )
{
    QApplication a( argc, argv );
    Frame frame;

    QImage img( "../listboxcombo/qtlogo.png" );
    QPixmap pix;
    pix.convertFromImage( img.smoothScale( 48, 48 ) );

    QPixmap dbpix(dbicon);
    QPixmap textpix(texticon);
    QPixmap widgetpix(widgeticon);
    QPixmap twodpix(twodicon);
    QPixmap threedpix(threedicon);

    // example 1
    QTabWidget *tab = new QTabWidget();
    QWidget *w = new QWidget( tab );
    tab->addTab( w, "Widget" );
    frame.addCategory( tab, widgetpix, "Widgets" );

    // example 2
    tab = new QTabWidget();
    w = new QWidget( tab );
    tab->addTab( w, "Database" );
    frame.addCategory( tab, dbpix, "Database" );

    // 2D Graphics
    tab = new QTabWidget();
    w = new GraphWidget( tab );
    tab->addTab( w, "Graph Drawing" );
    tab->addTab( new DisplayWidget(), "Display" );
    frame.addCategory( tab, twodpix, "2D Graphics" );

#if defined(QT_MODULE_OPENGL)
    // 3D Graphics
    tab = new QTabWidget();
    GLWorkspace *ws = new GLWorkspace( tab );
    tab->addTab( ws, "3D Demo" );
    frame.addCategory( tab, threedpix, "3D Graphics" );
#endif
    // example 4
    tab = new QTabWidget();
    w = new TextEdit( tab );
    tab->addTab( w, "Richtext Editor" );
    QString home = QString(getenv("QTDIR")) + "/doc/html/index.html";
    w = new HelpWindow( home, ".", 0, "helpviewer" );
    tab->addTab( w, "Help Browser" );
    frame.addCategory( tab, textpix, "Text Drawing/Editing" );

    a.setMainWidget( &frame );
    frame.show();

    return a.exec();
}
