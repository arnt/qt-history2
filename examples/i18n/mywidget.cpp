/****************************************************************************
** $Id: //depot/qt/main/examples/i18n/mywidget.cpp#1 $
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qlabel.h>
#include <qlistbox.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qhbox.h>
#include <qapplication.h>

#include "mywidget.h"

MyWidget::MyWidget( QWidget* parent, const QString &language, const char* name )
	: QVBox( parent, name )
{
	setCaption( tr( "Internationalization Example" ) ); 

	setMargin( 5 ); 
	setSpacing( 5 ); 

	QHBox *lang = new QHBox( this ); 
	lang->setFrameStyle( QFrame::Panel | QFrame::Sunken ); 
	lang->setSpacing( 5 ); 
	lang->setMargin( 5 ); 
	(void)new QLabel( tr( "Language:" ), lang ); 
	QLabel *l = new QLabel( lang ); 
	if ( language == "de" )
		l->setText( tr( "German" ) ); 
	else if ( language == "en" )
		l->setText( tr( "English" ) ); 
	else if ( language == "no" )
		l->setText( tr( "Norwegian" ) ); 

	lang->setMaximumHeight( l->sizeHint().height() + 10 ); 

	( void )new QLabel( tr( "The Main Window" ), this ); 

	QButtonGroup* gbox = new QButtonGroup( 1, QGroupBox::Horizontal, 
										  tr( "View" ), this ); 
	(void)new QRadioButton( tr( "Perspective" ), gbox ); 
	(void)new QRadioButton( tr( "Isometric" ), gbox ); 
	(void)new QRadioButton( tr( "Oblique" ), gbox ); 

	initChoices(); 
}

static const char* choices[] = {
	QT_TRANSLATE_NOOP( "MyWidget", "First" ), 
	QT_TRANSLATE_NOOP( "MyWidget", "Second" ), 
	QT_TRANSLATE_NOOP( "MyWidget", "Third" ), 
	0
}; 

void MyWidget::initChoices()
{
	QListBox* lb = new QListBox( this ); 
	for ( int i = 0; choices[i]; i++ )
		lb->insertItem( tr( choices[i] ) ); 
}
