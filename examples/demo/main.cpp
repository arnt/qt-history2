/****************************************************************************
** $Id: //depot/qt/main/examples/demo/main.cpp#5 $
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "frame.h"
#include "graph.h"
#include "textdrawing/textedit.h"
#include "textdrawing/helpwindow.h"

#include <stdlib.h>

#include <qapplication.h>
#include <qimage.h>
#include <qwindowsstyle.h>
#include <qtabwidget.h>
#include <qfont.h>

int main( int argc, char **argv )
{
    QApplication a( argc, argv );
    QFont fnt;
    fnt.setFamily("tahoma");
    fnt.setCharSet( QFont::Unicode );
    if ( fnt.substitute( "tahoma" ) == "tahoma" )
	a.setFont( fnt );

    Frame frame;

    QImage img( "../listboxcombo/qtlogo.png" );
    QPixmap pix;
    pix.convertFromImage( img.smoothScale( 48, 48 ) );


    // example 1
    QTabWidget *tab = new QTabWidget();
    QWidget *w = new QWidget( tab );
    tab->addTab( w, "Widget" );
    frame.addCategory( tab, pix, "Widgets" );

    // example 2
    tab = new QTabWidget();
    w = new QWidget( tab );
    tab->addTab( w, "Database" );
    frame.addCategory( tab, pix, "Database" );

    // example 3
    tab = new QTabWidget();
    w = new GraphWidget( tab );
    tab->addTab( w, "Graph Drawing" );
    frame.addCategory( tab, pix, "2D Graphics" );

    // example 4
    tab = new QTabWidget();
    w = new TextEdit( tab );
    tab->addTab( w, "Richtext Editor" );
    QString home = QString(getenv("QTDIR")) + "/doc/html/index.html";
    w = new HelpWindow( home, ".", 0, "helpviewer" );
    tab->addTab( w, "Help Browser" );
    frame.addCategory( tab, pix, "Text Drawing/Editing" );

    a.setMainWidget( &frame );
    frame.show();

    a.exec();
}
