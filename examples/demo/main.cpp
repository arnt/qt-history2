/****************************************************************************
** $Id: //depot/qt/main/examples/demo/main.cpp#3 $
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "frame.h"
#include "graph.h"

#include <qapplication.h>
#include <qimage.h>
#include <qwindowsstyle.h>
#include <qtabwidget.h>

int main( int argc, char **argv )
{
    QApplication a( argc, argv );

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

    a.setMainWidget( &frame );
    frame.show();

    a.exec();
}
