/****************************************************************************
** $Id: //depot/qt/main/examples/qbrowser/helpwindow.cpp#6 $
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "helpwindow.h"
#include <qstatusbar.h>
#include <qpixmap.h>
#include <qpopupmenu.h>
#include <qmenubar.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qiconset.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qstylesheet.h>
#include <qmessagebox.h>
#include <qfiledialog.h>

HelpWindow::HelpWindow( const QString& home_, const QString& path, QWidget* parent, const char *name )
    : QMainWindow( parent, name )
{

    browser = new QTextBrowser( this );
    browser->mimeSourceFactory()->setFilePath( path );
    browser->setFrameStyle( QFrame::Panel | QFrame::Sunken );
    connect( browser, SIGNAL( textChanged() ),
	     this, SLOT( textChanged() ) );

    setCentralWidget( browser );

    if ( !home_.isEmpty() )
	browser->setSource( home_ );

    connect( browser, SIGNAL( highlighted( const QString&) ),
	     statusBar(), SLOT( message( const QString&)) );

    resize( 640,700 );

    QPopupMenu* file = new QPopupMenu( this );
    file->insertItem( tr("&Open"), this, SLOT( open() ) );
    file->insertSeparator();
    file->insertItem( tr("&Close"), this, SLOT( close() ) );

    QPopupMenu* navigate = new QPopupMenu( this );
    backwardId = navigate->insertItem( QPixmap("back.xpm"),
				       tr("&Backward"), browser, SLOT( backward() ),
				       ALT | Key_Left );
    forwardId = navigate->insertItem( QPixmap("forward.xpm"),
				      tr("&Forward"), browser, SLOT( forward() ),
				       ALT | Key_Right );
    navigate->insertItem( QPixmap("home.xpm"), tr("&Home"), browser, SLOT( home() ) );

    QPopupMenu* help = new QPopupMenu( this );
    help->insertItem( tr("&About ..."), this, SLOT( about() ) );
    help->insertItem( tr("About &Qt ..."), this, SLOT( aboutQt() ) );

    menuBar()->insertItem( tr("&File"), file );
    menuBar()->insertItem( tr("&Navigate"), navigate );
    menuBar()->insertItem( tr("&Help"), help );

    menuBar()->setItemEnabled( forwardId, FALSE);
    menuBar()->setItemEnabled( backwardId, FALSE);
    connect( browser, SIGNAL( backwardAvailable( bool ) ),
	     this, SLOT( setBackwardAvailable( bool ) ) );
    connect( browser, SIGNAL( forwardAvailable( bool ) ),
	     this, SLOT( setForwardAvailable( bool ) ) );


    QToolBar* toolbar = new QToolBar( this );
    addToolBar( toolbar, "Toolbar");
    QToolButton* button;

    button = new QToolButton( QPixmap("back.xpm"), tr("Backward"), "", browser, SLOT(backward()), toolbar );
    connect( browser, SIGNAL( backwardAvailable(bool) ), button, SLOT( setEnabled(bool) ) );
    button->setEnabled( FALSE );
    button = new QToolButton( QPixmap("forward.xpm"), tr("Forward"), "", browser, SLOT(forward()), toolbar );
    connect( browser, SIGNAL( forwardAvailable(bool) ), button, SLOT( setEnabled(bool) ) );
    button->setEnabled( FALSE );
    button = new QToolButton( QPixmap("home.xpm"), tr("Home"), "", browser, SLOT(home()), toolbar );

    browser->setFocus();
}


void HelpWindow::setBackwardAvailable( bool b)
{
    menuBar()->setItemEnabled( backwardId, b);
}

void HelpWindow::setForwardAvailable( bool b)
{
    menuBar()->setItemEnabled( forwardId, b);
}


void HelpWindow::textChanged()
{
    if ( browser->documentTitle().isNull() )
	setCaption( browser->context() );
    else
	setCaption( browser->documentTitle() ) ;
}

HelpWindow::~HelpWindow()
{
}

void HelpWindow::about()
{
    QMessageBox::about( this, "QBrowser Example",
			"<p>This example implements a simple HTML browser "
			"using Qt's rich text capabilities</p>"
			"<p>It's just about 100 lines of C++ code, so don't expect too much :-)</p>"
			);
}


void HelpWindow::aboutQt()
{
    QMessageBox::aboutQt( this, "QBrowser" );
}

void HelpWindow::open()
{
    QString fn = QFileDialog::getOpenFileName( QString::null, QString::null, this );
    if ( !fn.isEmpty() )
	browser->setSource( fn );
}
