/****************************************************************************
** $Id: //depot/qt/main/examples/application/application.cpp#14 $
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "application.h"
#include <qapplication.h>

#include <qpopupmenu.h>
#include <qmenubar.h>
#include <qstatusbar.h>
#include <qmessagebox.h>
#include <qxembed.h>
#include <stdio.h>

ApplicationWindow::ApplicationWindow()
    : QMainWindow( 0, "example application main window", WDestructiveClose )
{
    QPopupMenu * file = new QPopupMenu( this );
    menuBar()->insertItem( "&File", file );
    file->insertItem( "&Quit", qApp, SLOT( closeAllWindows() ), CTRL+Key_Q );

    QPopupMenu * help = new QPopupMenu( this );
    menuBar()->insertSeparator();
    menuBar()->insertItem( "&Help", help );

    help->insertItem( "&About", this, SLOT(about()), Key_F1 );
    help->insertItem( "About &Qt", this, SLOT(aboutQt()) );

    e = new QXEmbed( this, "shell" );
    e->setFocus();
    setCentralWidget( e );
    statusBar()->message( "Ready", 2000 );
    resize( 450, 600 );
    printf("QXEmbed Window ID is %d\n", e->winId() );
    printf("Pass this as a command line option to the 'component' program.\n" );
}


ApplicationWindow::~ApplicationWindow()
{
}


void ApplicationWindow::about()
{
    QMessageBox::about( this, "QXEmbed Example",
			QString("<p>This example demonstrates how to use QXEmbed "
				"for embedding external Qt applications under "
				"the X Window System.</p>"
				"<p>Run the <tt>command</tt> example program as "
				"<pre>   command -embed " 
				+ QString::number(e->winId()) + "</pre> to see it "
				"how it works.</p>") );
}


void ApplicationWindow::aboutQt()
{
    QMessageBox::aboutQt( this, "QXEmbed Example" );
}
