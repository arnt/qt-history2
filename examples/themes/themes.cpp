/****************************************************************************
** $Id: //depot/qt/main/examples/themes/themes.cpp#3 $
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "themes.h"
#include "wood.h"
#include "metal.h"

#include "../buttons_groups/buttongroups.h"
#include "../lineedits/lineedits.h"
#include "../listboxcombo/listboxcombo.h"
#include "../checklists/checklists.h"
#include "../progressbar/progressbar.h"
#include "../rangecontrols/rangecontrols.h"
#include "../richtext/richtext.h"

#include <qtabwidget.h>
#include <qapplication.h>
#include <qpopupmenu.h>
#include <qmenubar.h>
#include <qmessagebox.h>

#include <qwindowsstyle.h>
#include <qplatinumstyle.h>
#include <qmotifstyle.h>
#include <qcdestyle.h>

Themes::Themes( QWidget *parent, const char *name, WFlags f )
	: QMainWindow( parent, name, f )
{
	tabwidget = new QTabWidget( this );

	tabwidget->addTab( new ButtonsGroups( tabwidget ), "Buttons/Groups" );
	QHBox *hbox = new QHBox( tabwidget );
	hbox->setMargin( 5 );
	(void)new LineEdits( hbox );
	(void)new ProgressBar( hbox );
	tabwidget->addTab( hbox, "Lineedits/Progressbar" );
	tabwidget->addTab( new ListBoxCombo( tabwidget ), "Listboxes/Comboboxes" );
 	tabwidget->addTab( new CheckLists( tabwidget ), "Listviews" );
 	tabwidget->addTab( new RangeControls( tabwidget ), "Rangecontrols" );
 	tabwidget->addTab( new MyRichText( tabwidget ), "Fortune" );

	setCentralWidget( tabwidget );

	QPopupMenu *style = new QPopupMenu( this );
	style->setCheckable( TRUE );
	menuBar()->insertItem( "&Style" , style );

	sMetal = style->insertItem( "&Metal", this, SLOT( styleMetal() ) );
	sWood = style->insertItem( "&Norwegian Wood", this, SLOT( styleWood() ) );
	sPlatinum = style->insertItem( "&Platinum" , this ,SLOT( stylePlatinum() ) );
	sWindows = style->insertItem( "&Windows", this, SLOT( styleWindows() ) );
	sCDE = style->insertItem( "&CDE", this, SLOT( styleCDE() ) );
	sMotif = style->insertItem( "M&otif", this, SLOT( styleMotif() ) );
	style->insertSeparator();
	style->insertItem("&Quit", qApp, SLOT( quit() ), CTRL | Key_Q );

	QPopupMenu * help = new QPopupMenu( this );
	menuBar()->insertSeparator();
	menuBar()->insertItem( "&Help", help );
	help->insertItem( "&About", this, SLOT(about()), Key_F1);
	help->insertItem( "About&Qt", this, SLOT(aboutQt()));

	qApp->setStyle( new MetalStyle );
	menuBar()->setItemChecked( sMetal, TRUE );
}

void Themes::styleWood()
{
	qApp->setStyle( new NorwegianWoodStyle );
	selectStyleMenu( sWood );
}

void Themes::styleMetal()
{
	qApp->setStyle( new MetalStyle );
	selectStyleMenu( sMetal );
}

void Themes::stylePlatinum()
{
	qApp->setStyle( new QPlatinumStyle );
	selectStyleMenu( sPlatinum );
}

void Themes::styleWindows()
{
	qApp->setStyle( new QWindowsStyle );
	selectStyleMenu( sWindows );
}

void Themes::styleCDE()
{
	qApp->setStyle( new QCDEStyle( TRUE ) );
	selectStyleMenu( sCDE );
}

void Themes::styleMotif()
{
	qApp->setStyle( new QMotifStyle( TRUE ) );
	selectStyleMenu( sMotif );
}


void Themes::about()
{
    QMessageBox::about( this, "Qt Themes Example",
			"<p>This example demonstrates the concept of "
			"<b>generalized GUI styles </b> first introduced "
			" with the 2.0 release of Qt.</p>" );
}


void Themes::aboutQt()
{
    QMessageBox::aboutQt( this, "Qt Themes Example" );
}


void Themes::selectStyleMenu( int s )
{
    menuBar()->setItemChecked( sWood, FALSE );
    menuBar()->setItemChecked( sMetal, FALSE );
    menuBar()->setItemChecked( sPlatinum, FALSE );
    menuBar()->setItemChecked( sWindows, FALSE );
    menuBar()->setItemChecked( sCDE, FALSE );
    menuBar()->setItemChecked( sMotif, FALSE );
    menuBar()->setItemChecked( s, TRUE );
}
