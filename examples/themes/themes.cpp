/****************************************************************************
** $Id: //depot/qt/main/examples/themes/themes.cpp#1 $
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "themes.h"
#include "wood.h"
#include "metal.h"

#include "../buttons_groups/buttons_groups.h"
#include "../linedits/linedits.h"
#include "../listbox_combo/listbox_combo.h"
#include "../checklists/checklists.h"
#include "../progressbar/progressbar.h"
#include "../rangecontrols/rangecontrols.h"
#include "../richtext/richtext.h"

#include <qtabwidget.h>
#include <qapplication.h>
#include <qpopupmenu.h>
#include <qmenubar.h>

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

	qApp->setStyle( new MetalStyle );

	QPopupMenu *style = new QPopupMenu( this );
	menuBar()->insertItem( "&Style" , style );

	sMetal = style->insertItem( "&Metal", this, SLOT( styleMetal() ) );
	sWood = style->insertItem( "&Norwegian Wood", this, SLOT( styleWood() ) );
	sPlatinum = style->insertItem( "&Platinum" , this ,SLOT( stylePlatinum() ) );
	sWindows = style->insertItem( "&Windows", this, SLOT( styleWindows() ) );
	sCDE = style->insertItem( "&CDE", this, SLOT( styleCDE() ) );
	sMotif = style->insertItem( "M&otif", this, SLOT( styleMotif() ) );

}

void Themes::styleWood()
{
	qApp->setStyle( new NorwegianWoodStyle );
}

void Themes::styleMetal()
{
	qApp->setStyle( new MetalStyle );
}

void Themes::stylePlatinum()
{
	qApp->setStyle( new QPlatinumStyle );
}

void Themes::styleWindows()
{
	qApp->setStyle( new QWindowsStyle );
}

void Themes::styleCDE()
{
	qApp->setStyle( new QCDEStyle( TRUE ) );
}

void Themes::styleMotif()
{
	qApp->setStyle( new QMotifStyle( TRUE ) );
}

