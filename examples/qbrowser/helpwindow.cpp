/****************************************************************************
** $Id: //depot/qt/main/examples/qbrowser/helpwindow.cpp#2 $
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

HelpWindow::HelpWindow( const QString& home_, const QString& path, QWidget* parent = 0, const char *name=0 )
    : QMainWindow( parent, name )
{

    browser = new QTextBrowser( this );
    browser->mimeSourceFactory()->setFilePath( path );
    browser->setFrameStyle( QFrame::Panel | QFrame::Sunken );
    connect( browser, SIGNAL( textChanged() ),
	     this, SLOT( textChanged() ) );

    setCentralWidget( browser );

    if ( !home_.isEmpty() )
	browser->setText( home_ );

    connect( browser, SIGNAL( highlighted( const QString&) ),
	     statusBar(), SLOT( message( const QString&)) );

    resize( 640,700 );

    QPopupMenu* file = new QPopupMenu( this );
    file->insertItem( tr("&Close"), this, SLOT( close() ) );

    QPopupMenu* navigate = new QPopupMenu( this );
    backwardId = navigate->insertItem( tr("&Backward"), browser, SLOT( backward() ) );
    forwardId = navigate->insertItem( tr("&Forward"), browser, SLOT( forward() ) );
    navigate->insertItem( tr("&Home"), browser, SLOT( home() ) );

    menuBar()->insertItem( tr("&File"), file );
    menuBar()->insertItem( tr("&Navigate"), navigate );

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
	setCaption( tr("QBrowser") );
    else
	setCaption( browser->documentTitle() ) ;
}

HelpWindow::~HelpWindow()
{
}


void HelpWindow::setupSlideshow( const QString& file)
{
    QColorGroup g = browser->paperColorGroup();
    g.setColor( QColorGroup::Text, white );
    g.setColor( QColorGroup::Foreground, white );
    g.setColor( QColorGroup::Base, darkBlue );
    browser->setPaperColorGroup( g );
    //browser->setPaper( QBrush( darkBlue, QPixmap("bg.gif") ));

    QStyleSheet::defaultSheet()->item("qml")->setFontSize( 24 );
    QStyleSheet::defaultSheet()->item("h1")->setFontSize( 32 );
    QStyleSheet::defaultSheet()->item("h2")->setFontSize( 24 );
    QStyleSheet::defaultSheet()->item("qml")->setMargin( QStyleSheetItem::MarginHorizontal, 50 );
    QStyleSheet::defaultSheet()->item("li")->setMargin( QStyleSheetItem::MarginVertical, 4 );
    QStyleSheetItem* style = new QStyleSheetItem( QStyleSheet::defaultSheet(), "heading" );
    style->setDisplayMode(QStyleSheetItem::DisplayBlock);
    style->setFontItalic( TRUE );
    style->setFontSize( 14 );
    style->setAlignment( QStyleSheetItem::AlignRight );
    resize(800-8,600-28);

    QFile f (file );
    if ( f.open( IO_ReadOnly) ) {
	int n = 0;
	QTextStream t(&f);
	while ( !t.atEnd() ) {
	    QString line = t.readLine();
	    if ( !line.isEmpty() && line[0] != '#') {
		browser->setText( line );
		n++;
	    }
	}
	f.close();
	while ( n-- > 1) {
	    browser->backward();
	}
    }
}
