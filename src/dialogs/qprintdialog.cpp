/****************************************************************************
** $Id: //depot/qt/main/src/dialogs/qprintdialog.cpp#17 $
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

#include "qfile.h"

#include "qcombo.h"
#include "qframe.h"
#include "qlabel.h"
#include "qlined.h"
#include "qpushbt.h"
#include "qprinter.h"
#include "qlistview.h"
#include "qlayout.h"
#include "qbttngrp.h"
#include "qradiobt.h"
#include "qspinbox.h"
#include "qapp.h"

#include "qstring.h"
#include "qregexp.h"

#include <ctype.h>
#include <stdlib.h>

RCSTAG("$Id: //depot/qt/main/src/dialogs/qprintdialog.cpp#17 $");


struct QPrintDialogPrivate
{
    QPrinter * printer;

    QButtonGroup * printerOrFile;

    bool outputToFile;
    QListView * printers;
    QLineEdit * fileName;
    QPushButton * browse;

    QButtonGroup * paperSize;
    QComboBox * otherSizeCombo;
    QPrinter::PageSize pageSize;

    QButtonGroup * orient;
    QPrinter::Orientation orientation;

    QButtonGroup * pageOrder;
    QPrinter::PageOrder pageOrder2;

    int numCopies;
};


static void parsePrintcap( QListView * printers )
{
    QFile printcap( "/etc/printcap" );
    if ( !printcap.open( IO_ReadOnly ) )
	return;

    char * line = new char[1025];
    line[1024] = '\0';

    QString printerDesc;
    int lineLength;

    while( !printcap.atEnd() &&
	   (lineLength=printcap.readLine( line, 1024 )) > 0 ) {
	if ( lineLength >= 2 && line[lineLength-2] == '\\' ) {
	    line[lineLength-2] = '\0';
	    printerDesc += line;
	} else {
	    printerDesc += line;
	    // strip away the comment stuff
	    int i = printerDesc.find( '#' );
	    if ( i >= 0 )
		printerDesc.truncate( i );
	    printerDesc = printerDesc.simplifyWhiteSpace();
	    i = printerDesc.find( ':' );
	    QString printerName, printerComment, printerHost;
	    if ( i >= 0 ) {
		// have : want |
		int j = printerDesc.findRev( '|', i-1 );
		if ( j < 0 )
		    j = 0;
		printerName = printerDesc.mid( j+1, i-j-1 );
		if ( j > 0 ) {
		    // try hacking up a comment from the aliases...
		    printerComment = "Aliases: ";
		    printerComment += printerDesc.mid( 0, j );
		    for( j=printerComment.length(); j>-1; j-- )
			if ( printerComment[j] == '|' )
			    printerComment[j] = ',';
		}
		// then look for a real comment
		j = i+1;
		while( printerDesc[j] && isspace(printerDesc[j]) )
		    j++;
		if ( printerDesc[j] != ':' ) {
		    printerComment = printerDesc.mid( i, j-i );
		    printerComment.simplifyWhiteSpace();
		}
		// look for signs of this being a remote printer
		i = printerDesc.find( QRegExp( ": ?rm ?=", TRUE ) );
		if ( i >= 0 ) {
		    // point k at the end of remote host name
		    j = 0;
		    while( printerDesc[i] != '=' )
			i++;
		    while( printerDesc[i] == '=' || isspace( printerDesc[i] ) )
			i++;
		    j = i;
		    while( printerDesc[j] != ':' && printerDesc[j] )
			j++;

		    // and stuff that into the string
		    printerHost = printerDesc.mid( i, j-i );
		}
	    }
	    if ( printerHost.isNull() )
		printerHost = "locally connected";
	    if ( printerName.length() )
		(void)new QListViewItem( printers, printerName, printerHost,
					 printerComment );
	    // chop away the line, for processing the next one
	    printerDesc = 0;
	}
    }
    delete[] line;
}


static void parseEtcLp( QListView * )
{
    // later
}


static QPrintDialog * globalPrintDialog = 0;
static void deleteGlobalPrinterDialog()
{
    delete globalPrintDialog;
    globalPrintDialog = 0;
}


QPrintDialog::QPrintDialog( QPrinter *prn, QWidget *parent, const char *name )
    : QDialog( parent, name, TRUE )
{
    d = new QPrintDialogPrivate;
    d->numCopies = 1;
    d->printer = prn;

    QBoxLayout * tll = new QBoxLayout( this, QBoxLayout::Down, 12, 0 );

    QGroupBox * g;
    g = setupDestination();
    tll->addWidget( g, 1 );
    tll->addSpacing( 12 );

    g = setupOptions();
    tll->addWidget( g );
    tll->addSpacing( 12 );

    QBoxLayout * horiz = new QBoxLayout( QBoxLayout::LeftToRight );
    tll->addLayout( horiz );

    if ( style() != MotifStyle )
	horiz->addStretch( 1 );

    QPushButton * ok = new QPushButton( this, "ok" );
    ok->setText( "Ok" );
    ok->setAutoDefault( TRUE );
    ok->setDefault( TRUE );
    horiz->addWidget( ok );
    if ( style() == MotifStyle )
	horiz->addStretch( 1 );
    horiz->addSpacing( 6 );

    QPushButton * cancel = new QPushButton( this, "cancel" );
    cancel->setText( "Cancel" );
    cancel->setAutoDefault( TRUE );
    horiz->addWidget( cancel );

    QSize s1 = ok->sizeHint();
    QSize s2 = cancel->sizeHint();
    s1 = QSize( QMAX(s1.width(), s2.width()),
		QMAX(s1.height(), s2.height()) );

    ok->setFixedSize( s1 );
    cancel->setFixedSize( s1 );
    
    tll->activate();

    connect( ok, SIGNAL(clicked()), SLOT(okClicked()) );
    connect( cancel, SIGNAL(clicked()), SLOT(reject()) );
    
    QSize ms( minimumSize() );
    QSize ss( QApplication::desktop()->size() );
    if ( ms.height() < 512 && ss.height() >= 600 )
	ms.setHeight( 512 );
    else if ( ms.height() < 460 && ss.height() >= 480 )
	ms.setHeight( 460 );
    resize( ms );
}


/*! Destroys the object and frees any allocated resources.
*/

QPrintDialog::~QPrintDialog()
{
    delete d->printerOrFile;
    delete d->paperSize;
    delete d->pageOrder;
    delete d;
    if ( this == globalPrintDialog )
	globalPrintDialog = 0;
}


QGroupBox * QPrintDialog::setupDestination()
{
    QGroupBox * g = new QGroupBox( tr( "Print Destination"),
				   this, "destination group box" );

    QBoxLayout * tll = new QBoxLayout( g, QBoxLayout::Down, 6 );
    tll->addSpacing( 12 );

    d->printerOrFile = new QButtonGroup( (QWidget *)0 );
    connect( d->printerOrFile, SIGNAL(clicked(int)),
	     this, SLOT(printerOrFileSelected(int)) );

    // printer radio button, list
    QRadioButton * rb = new QRadioButton( tr( "Print to Printer:" ), g,
					  "printer" );
    rb->setMinimumSize( rb->sizeHint() );
    tll->addWidget( rb );
    d->printerOrFile->insert( rb, 0 );
    rb->setChecked( TRUE );
    d->outputToFile = FALSE;

    QBoxLayout * horiz = new QBoxLayout( QBoxLayout::LeftToRight );
    tll->addLayout( horiz, 3 );
    horiz->addSpacing( 20 );

    d->printers = new QListView( g, "list of printers" );
    d->printers->setAllColumnsShowFocus( TRUE );
    d->printers->setColumn( "Printer", 150 );
    d->printers->setColumn( "Host", 150 );
    d->printers->setColumn( "Comment", 100 );

#if defined(UNIX)
    QFileInfo f;
    f.setFile( "/etc/printcap" );
    if ( f.isFile() && f.isReadable() )
	parsePrintcap( d->printers );
    f.setFile( "/etc/lp" );
    if ( f.isDir() )
	parseEtcLp( d->printers );

    char * dollarPrinter;
    dollarPrinter = getenv( "PRINTER" );
    int quality = 0;

    const QListViewItem * lvi = d->printers->firstChild();
    while( lvi ) {
	QRegExp ps1( "[^a-z]ps[^a-z]" );
	QRegExp ps2( "[^a-z]ps$" );
	QRegExp lp1( "[^a-z]lp[^a-z]" );
	QRegExp lp2( "[^a-z]lp$" );
	if ( quality < 3 &&
	     !qstrcmp( lvi->text( 0 ), dollarPrinter ) ) {
	    d->printers->setCurrentItem( (QListViewItem *)lvi );
	    quality = 3;
	} else if ( quality < 2 &&
		    ( !qstrcmp( lvi->text( 0 ), "ps" ) ||
		      ps1.match( lvi->text( 2 ) ) > -1 ||
		      ps2.match( lvi->text( 2 ) ) > -1 ) ) {
	    d->printers->setCurrentItem( (QListViewItem *)lvi );
	    quality = 2;
	} else if ( quality < 1 &&
		    ( !qstrcmp( lvi->text( 0 ), "lp" ) ||
		      lp1.match( lvi->text( 2 ) ) > -1 ||
		      lp2.match( lvi->text( 2 ) ) > -1 ) ) {
	    d->printers->setCurrentItem( (QListViewItem *)lvi );
	    quality = 1;
	}
	lvi = lvi->nextSibling();
    }
    if ( d->printers->currentItem() )
	d->printers->setSelected( d->printers->currentItem(), TRUE );
#endif
    
    d->printers->setMinimumSize( 400, fontMetrics().height() * 5 );
    horiz->addWidget( d->printers, 3 );

    tll->addSpacing( 6 );

    // file radio button, edit/browse
    rb = new QRadioButton( tr( "Print to File:" ), g, "file" );
    rb->setMinimumSize( rb->sizeHint() );
    tll->addWidget( rb );
    d->printerOrFile->insert( rb, 0 );

    horiz = new QBoxLayout( QBoxLayout::LeftToRight );
    tll->addLayout( horiz );
    horiz->addSpacing( 20 );

    d->fileName = new QLineEdit( g, "file name" );
    d->fileName->setMinimumSize( d->fileName->sizeHint() );
    horiz->addWidget( d->fileName, 1 );
    horiz->addSpacing( 6 );
    d->browse = new QPushButton( tr("Browse"), g, "browse files" );
    d->browse->setMinimumSize( d->browse->sizeHint() );
    connect( d->browse, SIGNAL(clicked()),
	     this, SLOT(browseClicked()) );
    horiz->addWidget( d->browse );

    d->fileName->setEnabled( FALSE );
    d->browse->setEnabled( FALSE );

    tll->activate();

    return g;
}


QGroupBox * QPrintDialog::setupOptions()
{
    QGroupBox * g = new QGroupBox( tr( "Options"),
				   this, "options group box" );

    QBoxLayout * tll = new QBoxLayout( g, QBoxLayout::Down, 6 );
    tll->addSpacing( 12 );

    d->pageOrder = new QButtonGroup( (QWidget *)0 );
    connect( d->pageOrder, SIGNAL(clicked(int)),
	     this, SLOT(pageOrderSelected(int)) );
    d->paperSize = new QButtonGroup( (QWidget *)0 );
    connect( d->paperSize, SIGNAL(clicked(int)),
	     this, SLOT(paperSizeSelectedRadio(int)) );
    d->orient = new QButtonGroup( (QWidget *)0 );
    connect( d->orient, SIGNAL(clicked(int)),
	     this, SLOT(orientSelected(int)) );

    // print preferences
    QGridLayout * grid = new QGridLayout( 10, 4 );
    tll->addLayout( grid );

    grid->addColSpacing( 0, 20 );
    grid->setColStretch( 1, 1 );
    grid->addColSpacing( 2, 20 );
    grid->setColStretch( 3, 1 );

    int i;
    for( i=0; i<10; i++ )
	grid->setRowStretch( i, 0 );

    grid->addRowSpacing( 3, 6 );
    grid->addRowSpacing( 6, 6 );

    // page sizes
    QLabel * l = new QLabel( "Page Size", g, "page size" );
    l->setFixedHeight( l->sizeHint().height() );
    grid->addMultiCellWidget( l, 0, 0, 0, 3 );

    QRadioButton * rb = new QRadioButton( "A4 (210 x 297 mm)", g, "A4" );
    rb->setMinimumSize( rb->sizeHint() );
    grid->addWidget( rb, 1, 1 );
    d->paperSize->insert( rb, 3 ); // 3
    rb->setChecked( TRUE );
    d->pageSize = QPrinter::A4;
	      
    rb = new QRadioButton( "Letter (8½ x 11in)", g, "Letter" );
    rb->setMinimumSize( rb->sizeHint() );
    grid->addWidget( rb, 2, 1 );
    d->paperSize->insert( rb, 4 ); // 4

    rb = new QRadioButton( "Other size:", g, "other size" );
    rb->setMinimumSize( rb->sizeHint() );
    grid->addWidget( rb, 1, 3 );
    d->paperSize->insert( rb, 9999 ); // magic

    QBoxLayout * horiz = new QBoxLayout( QBoxLayout::LeftToRight );
    grid->addLayout( horiz, 2, 3 );
    horiz->addSpacing( 20 );

    d->otherSizeCombo = new QComboBox( FALSE, g, "other paper sizes" );
    d->otherSizeCombo->insertItem( "B5" ); // 0
    d->otherSizeCombo->insertItem( "Legal" ); // 1
    d->otherSizeCombo->insertItem( "Executive" ); // 2
    d->otherSizeCombo->insertItem( "A4" ); // 3
    d->otherSizeCombo->insertItem( "Letter" ); // 4
    d->otherSizeCombo->setMinimumSize( d->otherSizeCombo->sizeHint() );
    horiz->addWidget( d->otherSizeCombo );
    horiz->addStretch( 1 );
    d->otherSizeCombo->setEnabled( FALSE );
    connect( d->otherSizeCombo, SIGNAL(activated(int)),
	     this, SLOT(paperSizeSelectedCombo(int)) );

    // page orientation
    l = new QLabel( tr("Orientation"), g, "orientation" );
    l->setFixedHeight( l->sizeHint().height() );
    grid->addMultiCellWidget( l, 4, 4, 0, 1 );

    rb = new QRadioButton( "Portrait", g, "portrait format" );
    rb->setMinimumSize( rb->sizeHint() );
    grid->addWidget( rb, 5, 1 );
    d->orient->insert( rb, (int)QPrinter::Portrait );
    rb->setChecked( TRUE );
    d->orientation = QPrinter::Portrait;

    rb = new QRadioButton( "Landscape", g, "landscape format" );
    rb->setMinimumSize( rb->sizeHint() );
    grid->addWidget( rb, 6, 1 );
    d->orient->insert( rb, (int)QPrinter::Landscape );

    // print order
    l = new QLabel( tr("Start with"), g, "what page first?" );
    l->setFixedHeight( l->sizeHint().height() );
    grid->addMultiCellWidget( l, 4, 4, 2, 3 );

    rb = new QRadioButton( tr("First page"), g, "first page first" );
    rb->setMinimumSize( rb->sizeHint() );
    grid->addWidget( rb, 5, 3 );
    rb->setChecked( TRUE );
    d->pageOrder->insert( rb, QPrinter::FirstPageFirst );

    rb = new QRadioButton( tr("Last page"), g, "last page first" );
    rb->setMinimumSize( rb->sizeHint() );
    grid->addWidget( rb, 6, 3 );
    d->pageOrder->insert( rb, QPrinter::LastPageFirst );

    // copies
    l = new QLabel( tr("Number of copies"), g, "copies?" );
    l->setFixedHeight( l->sizeHint().height() );
    grid->addMultiCellWidget( l, 7, 7, 0, 3 );

    horiz = new QBoxLayout( QBoxLayout::LeftToRight );
    grid->addLayout( horiz, 8, 1 );
    QSpinBox * spin = new QSpinBox( 1, 99, 1, g, "copies" );
    spin->setValue( 1 );
    spin->setMinimumSize( spin->sizeHint() );
    horiz->addWidget( spin );
    horiz->addStretch( 1 );
    connect( spin, SIGNAL(valueChanged(int)),
	     this, SLOT(setNumCopies(int)) );

    // more

    tll->activate();

    return g;
}


/*!  Display a dialog and allow the user to configure the QPrinter \a
  p.  Returns TRUE if the user clicks OK or presses Enter, FALSE if
  the user clicks Cancel or presses Escape.
  
  getPrinterSetup() remembers the settings and provides the same
  settings the next time the dialog is shown.
*/

bool QPrintDialog::getPrinterSetup( QPrinter * p )
{
    if ( !globalPrintDialog ) {
	globalPrintDialog = new QPrintDialog( 0, 0, "global print dialog" );
	qAddPostRoutine( deleteGlobalPrinterDialog );
    }

    globalPrintDialog->d->printer = p;
    bool r = globalPrintDialog->exec() == QDialog::Accepted;
    globalPrintDialog->d->printer = 0;
    return r;
}


void QPrintDialog::printerOrFileSelected( int id )
{
    d->outputToFile = id ? TRUE : FALSE;
    d->printers->setEnabled( !d->outputToFile );
    d->browse->setEnabled( d->outputToFile );
    d->fileName->setEnabled( d->outputToFile );
}


void QPrintDialog::landscapeSelected( int id )
{
    d->orientation = (QPrinter::Orientation)id;
}


void QPrintDialog::paperSizeSelectedCombo( int id )
{
    switch( id ) {
    case 0:
	d->pageSize = QPrinter::B5;
	break;
    case 1:
	d->pageSize = QPrinter::Legal;
	break;
    case 2:
	d->pageSize = QPrinter::Executive;
	break;
    case 3:
	d->pageSize = QPrinter::A4;
	d->paperSize->setButton( 3 );
	break;
    case 4:
	d->pageSize = QPrinter::Letter;
	d->paperSize->setButton( 4 );
	break;
    }
}

void QPrintDialog::paperSizeSelectedRadio( int id )
{
    if ( id == 9999 ) {
	d->otherSizeCombo->setEnabled( TRUE );
    } else if ( id != d->pageSize && ( id == 3 || id == 4 ) ) {
	if ( id == 3 )
	    d->pageSize = QPrinter::A4;
	else
	    d->pageSize = QPrinter::Letter;
	d->otherSizeCombo->setCurrentItem( id );
	d->otherSizeCombo->setEnabled( FALSE );
    }
}


void QPrintDialog::orientSelected( int id )
{
    d->orientation = (QPrinter::Orientation)id;
}


void QPrintDialog::pageOrderSelected( int id )
{
    d->pageOrder2 = (QPrinter::PageOrder)id;
}


void QPrintDialog::setNumCopies( int copies )
{
    d->numCopies = copies;
}


void QPrintDialog::browseClicked()
{
    QString fn = QFileDialog::getSaveFileName();
    if ( !fn.isNull() )
	d->fileName->setText( fn );
}


void QPrintDialog::okClicked()
{
    if ( d->outputToFile ) {
	d->printer->setOutputToFile( TRUE );
	d->printer->setOutputFileName( d->fileName->text() );
    } else {
	d->printer->setOutputToFile( FALSE );
	QListViewItem * l = d->printers->currentItem();
	if ( l )
	    d->printer->setPrinterName( l->text( 0 ) );
    }

    d->printer->setOrientation( d->orientation );
    d->printer->setPageSize( d->pageSize );
    d->printer->setPageOrder( d->pageOrder2 );
    accept();
}
