/****************************************************************************
** $Id: //depot/qt/main/src/dialogs/qprintdialog.cpp#16 $
**
** Implementation of internal print dialog (X11) used by QPrinter::select().
**
** Created : 950829
**
** Copyright (C) 1995-1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qprndlg.h"
#include "qfiledlg.h"
#include "qcombo.h"
#include "qframe.h"
#include "qlabel.h"
#include "qlined.h"
#include "qpushbt.h"
#include "qprinter.h"

RCSTAG("$Id: //depot/qt/main/src/dialogs/qprintdialog.cpp#16 $");


//
// This file contains experimental use of some widget-query macros.
//

#include "qobjcoll.h"

#define QUERY_WIDGET_TYPE(parent,classname,operation)			\
{									\
    QObjectList *list = parent->queryList( classname );			\
    if ( list ) {							\
	QObjectListIt it( *list );					\
	QWidget *w;							\
	while ( (w=(QWidget*)it.current()) ) {				\
	    ++it;							\
	    w->operation;						\
	}								\
    }									\
    delete list;							\
}

static QObject *find_child( QObject *parent, const char *objname )
{
    const QObjectList *list = parent->children();
    if ( list ) {
	QObjectListIt it( *list );
	QObject *obj;
	while ( (obj=it.current()) ) {
	    if ( strcmp(objname,obj->name()) == 0 )
		return obj;
	    ++it;
	}
    }
    return 0;
}

#define WIDGET(parent,type,name) ((type*)find_child(parent,name))


QPrintDialog::QPrintDialog( QPrinter *prn, QWidget *parent, const char *name )
    : QDialog( parent, name, TRUE )
{
    QComboBox	*combo;
    QFrame	*frame;
    QLabel	*label;
    QLineEdit	*lined;
    QPushButton *button;

    printer = prn;

    QFont font( "Helvetica", 12, QFont::Bold );

    label = new QLabel( this );
    label->setText( "Print To:" );
    label->setAlignment( AlignRight|AlignVCenter );
    label->setGeometry( 10,20, 120,30 );

    combo = new QComboBox( this, "printTo" );
    combo->insertItem( "Printer" );
    combo->insertItem( "File" );
    combo->setAutoResize( TRUE );
    combo->adjustSize();
    combo->move( 140,25 );
    combo->setCurrentItem( printer->outputToFile() ? 1 : 0 );
    connect( combo, SIGNAL(activated(int)), SLOT(printerOrFileSelected(int)) );

    label = new QLabel( this, "printerNameLabel" );
    label->setText( "Printer Name:" );
    label->setAlignment( AlignRight|AlignVCenter );
    label->setGeometry( 10,60, 120,30 );

    lined = new QLineEdit( this, "printerName" );
    lined->setText( printer->printerName() );
    lined->setGeometry( 140,65, 130,25 );
    connect( lined, SIGNAL(returnPressed()), SLOT(okClicked()) );

    label = new QLabel( this, "printCommandLabel" );
    label->setText( "Print Command:" );
    label->setAlignment( AlignRight|AlignVCenter );
    label->setGeometry( 10,100, 120,30 );

    lined = new QLineEdit( this, "printCommand" );
    lined->setText( printer->printProgram() );
    lined->setGeometry( 140,105, 130,25 );
    connect( lined, SIGNAL(returnPressed()), SLOT(okClicked()) );

    label = new QLabel( this, "printFileLabel" );
    label->setText( "File Name:" );
    label->setAlignment( AlignRight|AlignVCenter );
    label->setGeometry( 10,140, 120,30 );

    lined = new QLineEdit( this, "printFile" );
    lined->setText( printer->outputFileName() );
    lined->setGeometry( 140,145, 130,25 );
    connect( lined, SIGNAL(returnPressed()), SLOT(okClicked()) );

    button = new QPushButton( this, "browseButton" );
    button->setText( "Browse..." );
    button->setGeometry( 300,140, 80,30 );
    connect( button, SIGNAL(clicked()), SLOT(browseClicked()) );

    frame = new QFrame( this );
    frame->setFrameStyle( QFrame::HLine | QFrame::Sunken );
    frame->setGeometry( 10,190, 380,10 );

    label = new QLabel( this );
    label->setText( "Orientation:" );
    label->setAlignment( AlignRight|AlignVCenter );
    label->setGeometry( 10,210, 120,30 );

    combo = new QComboBox( this, "orientation" );
    combo->insertItem( "Portrait" );
    combo->insertItem( "Landscape" );
    combo->setAutoResize( TRUE );
    combo->adjustSize();
    combo->move( 140,215 );
    combo->setCurrentItem( (int)printer->orientation() );

    label = new QLabel( this );
    label->setText( "Page Size:" );
    label->setAlignment( AlignRight|AlignVCenter );
    label->setGeometry( 10,250, 120,30 );

    combo = new QComboBox( this, "pageSize" );
    combo->insertItem( "A4" );
    combo->insertItem( "B5" );
    combo->insertItem( "Letter" );
    combo->insertItem( "Legal" );
    combo->insertItem( "Executive" );
    combo->setAutoResize( TRUE );
    combo->adjustSize();
    combo->move( 140,255 );
    combo->setCurrentItem( (int)printer->pageSize() );

    frame = new QFrame( this );
    frame->setFrameStyle( QFrame::HLine | QFrame::Sunken );
    frame->setGeometry( 10,290, 380,10 );

    button = new QPushButton( this, "okButton" );
    button->setText( "Ok" );
    button->setGeometry( 20,310, 80,30 );
    connect( button, SIGNAL(clicked()), SLOT(okClicked()) );

    button = new QPushButton( this, "cancelButton" );
    button->setText( "Cancel" );
    button->setGeometry( 300,310, 80,30 );
    connect( button, SIGNAL(clicked()), SLOT(reject()) );

    QUERY_WIDGET_TYPE( this, "QLabel",	setFont(font) );
    QUERY_WIDGET_TYPE( this, "QButton", setFont(font) );

    font.setWeight( QFont::Normal );
    QUERY_WIDGET_TYPE( this, "QComboBox", setFont(font) );
    QUERY_WIDGET_TYPE( this, "QLineEdit", setFont(font) );

    printerOrFileSelected( printer->outputToFile() ? 1 : 0 );
}


void QPrintDialog::printerOrFileSelected( int index )
{
    QLabel	*printerNameL	= WIDGET(this,QLabel,"printerNameLabel");
    QLineEdit	*printerName	= WIDGET(this,QLineEdit,"printerName");
    QLabel	*printCommandL	= WIDGET(this,QLabel,"printCommandLabel");
    QLineEdit	*printCommand	= WIDGET(this,QLineEdit,"printCommand");
    QLabel	*printFileL	= WIDGET(this,QLabel,"printFileLabel");
    QLineEdit	*printFile	= WIDGET(this,QLineEdit,"printFile");
    QPushButton *browseButton	= WIDGET(this,QPushButton,"browseButton");
    bool	 toPrinter = index == 0;

    printerNameL ->setEnabled( toPrinter );
    printerName	 ->setEnabled( toPrinter );
    printCommandL->setEnabled( toPrinter );
    printCommand ->setEnabled( toPrinter );
    printFileL	 ->setEnabled( !toPrinter );
    printFile	 ->setEnabled( !toPrinter );
    browseButton ->setEnabled( !toPrinter );
}


void QPrintDialog::browseClicked()
{
    QString dir  = QDir::currentDirPath();
    QString file = QFileDialog::getOpenFileName( 0, 0, this );
    if ( !file.isEmpty() ) {
	if ( file.find(dir) == 0 )		// get relative file name
	    file.remove( 0, dir.length()+1 );
	WIDGET(this,QLineEdit,"printFile")->setText( file );
    }
}


void QPrintDialog::okClicked()
{
    QLineEdit	*printerName	= WIDGET(this,QLineEdit,"printerName");
    QLineEdit	*printCommand	= WIDGET(this,QLineEdit,"printCommand");
    QLineEdit	*printFile	= WIDGET(this,QLineEdit,"printFile");
    QComboBox	*orientation	= WIDGET(this,QComboBox,"orientation");
    QComboBox	*pageSize	= WIDGET(this,QComboBox,"pageSize");
    printer->setPrinterName( printerName->text() );
    printer->setPrintProgram( printCommand->text() );
    printer->setOutputFileName( printFile->text() );
    printer->setOutputToFile( printFile->isEnabled() );
    printer->setOrientation((QPrinter::Orientation)orientation->currentItem());
    printer->setPageSize( (QPrinter::PageSize)pageSize->currentItem() );
    accept();
}
