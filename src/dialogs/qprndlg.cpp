/****************************************************************************
** $Id: //depot/qt/main/src/dialogs/qprndlg.cpp#32 $
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

RCSTAG("$Id: //depot/qt/main/src/dialogs/qprndlg.cpp#32 $");


struct QPrintDialogPrivate
{
    QPrinter * printer;

    QButtonGroup * printerOrFile;

    bool outputToFile;
    QListView * printers;
    QLineEdit * fileName;
    QPushButton * browse;

    QButtonGroup * printRange;
    QLabel * firstPageLabel;
    QSpinBox * firstPage;
    QLabel * lastPageLabel;
    QSpinBox * lastPage;
    QRadioButton * printAllButton;
    QRadioButton * printRangeButton;

    QButtonGroup * paperSize;
    QPrinter::PageSize pageSize;

    QButtonGroup * orient;
    QPrinter::Orientation orientation;

    QButtonGroup * pageOrder;
    QPrinter::PageOrder pageOrder2;

    QButtonGroup * colorMode;
    QPrinter::ColorMode colorMode2;

    int numCopies;
};


static void perhapsAddPrinter( QListView * printers, const char * name,
			       const char * host, const char * comment )
{
    const QListViewItem * i = printers->firstChild();
    while( i && qstrcmp( i->text( 0 ), name ) )
	i = i->nextSibling();
    if ( !i )
	(void)new QListViewItem( printers, name,
				 host ? host : "locally connected",
				 comment ? comment : "" );
}


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
	if ( *line == '#' )
	    *line = '\0';
	if ( lineLength >= 2 && line[lineLength-2] == '\\' ) {
	    line[lineLength-2] = '\0';
	    printerDesc += line;
	} else {
	    printerDesc += line;
	    printerDesc = printerDesc.simplifyWhiteSpace();
	    int i = printerDesc.find( ':' );
	    QString printerName, printerComment, printerHost;
	    if ( i >= 0 ) {
		// have : want |
		int j = printerDesc.findRev( '|', i-1 );
		printerName = printerDesc.mid( j+1, i-j-1 );
		if ( j > 0 ) {
		    // try extracting a comment from the aliases...
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
		i = printerDesc.find( QRegExp( ": *rm *=" ) );
		if ( i >= 0 ) {
		    // point k at the end of remote host name
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
	    if ( printerName.length() )
		perhapsAddPrinter( printers, printerName, printerHost,
				   printerComment );
	    // chop away the line, for processing the next one
	    printerDesc = 0;
	}
    }
    delete[] line;
}


static void parseEtcLp( QListView * printers )
{
    QDir lp( "/etc/lp/printers" );
    const QFileInfoList * dirs = lp.entryInfoList();
    if ( !dirs )
	return;

    QFileInfoListIterator it( *dirs );
    QFileInfo *printer;
    QString tmp;
    while ( (printer = it.current()) != 0 ) {
	++it;
	if ( printer->isDir() ) {
	    tmp.sprintf( "/etc/lp/printers/%s/configuration",
			 printer->fileName().data() );
	    QFile configuration( tmp );
	    int ll;
	    char * line = new char[1025];
	    QRegExp remote( "^Remote:" );
	    QRegExp contentType( "^Content types:" );
	    QString printerHost;
	    bool canPrintPostscript = FALSE;
	    if ( configuration.open( IO_ReadOnly ) ) {
		while( !configuration.atEnd() &&
		       (ll=configuration.readLine( line, 1024 )) > 0 ) {
		    if ( remote.match( line ) == 0 ) {
			const char * p = line;
			while( *p != ':' )
			    p++;
			p++;
			while( isspace(*p) )
			    p++;
			printerHost = p;
			printerHost.simplifyWhiteSpace();
		    } else if ( contentType.match( line ) == 0 ) {
			char * p = line;
			while( *p != ':' )
			    p++;
			p++;
			char * e;
			while( *p ) {
			    while( isspace(*p) )
				p++;
			    if ( *p ) {
				char s;
				e = p;
				while( isalnum(*e) )
				    e++;
				s = *e;
				*e = '\0';
				if ( !qstrcmp( p, "postscript" ) ||
				     !qstrcmp( p, "any" ) )
				    canPrintPostscript = TRUE;
				*e = s;
				if ( s == ',' )
				    e++;
				p = e;
			    }
			}
		    }
		}
		if ( canPrintPostscript )
		    perhapsAddPrinter( printers, printer->fileName().data(),
				       printerHost, 0 );
	    }
	    delete[] line;
	}
    }
}


static QPrintDialog * globalPrintDialog = 0;
static void deleteGlobalPrinterDialog()
{
    delete globalPrintDialog;
    globalPrintDialog = 0;
}


/*! \class QPrintDialog qprndlg.h

  \brief The QPrintDialog class provides a dialog for specifying
  print-out details.

  \ingroup dialogs

  It encompasses both the sort of details needed for doing a simple
  print-out and some print configuration setup.

  At present, the only easy way to use the class is through the static
  function getPrinterSetup().
*/


/*! Creates a new modal printer dialog that configures \a prn and is a
  child of \a parent named \a name.
*/

QPrintDialog::QPrintDialog( QPrinter *prn, QWidget *parent, const char *name )
    : QDialog( parent, name, TRUE )
{
    d = new QPrintDialogPrivate;
    d->numCopies = 1;

    QBoxLayout * tll = new QBoxLayout( this, QBoxLayout::Down, 12, 0 );

    QGroupBox * g;
    g = setupDestination();
    tll->addWidget( g, 1 );
    tll->addSpacing( 12 );

    QBoxLayout * horiz = new QBoxLayout( QBoxLayout::LeftToRight );
    tll->addLayout( horiz, 0 );

    g = setupOptions();
    horiz->addWidget( g );
    horiz->addSpacing( 12 );

    g = setupPaper();
    horiz->addWidget( g );

    tll->addSpacing( 12 );

    horiz = new QBoxLayout( QBoxLayout::LeftToRight );
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

    setPrinter( prn, TRUE );
    d->printers->setFocus();
}


/*! Destroys the object and frees any allocated resources.  Does not
  delete the associated QPrinter object.
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

    QBoxLayout * tll = new QBoxLayout( g, QBoxLayout::Down, 12, 0 );
    tll->addSpacing( 8 );

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
    horiz->addSpacing( 19 );

    d->printers = new QListView( g, "list of printers" );
    d->printers->setAllColumnsShowFocus( TRUE );
    d->printers->addColumn( "Printer", 125 );
    d->printers->addColumn( "Host", 125 );
    d->printers->addColumn( "Comment", 150 );
    d->printers->setFrameStyle( QFrame::WinPanel + QFrame::Sunken );

#if defined(UNIX)
    char * etcLpDefault = 0;

    QFileInfo f;
    f.setFile( "/etc/printcap" );
    if ( f.isFile() && f.isReadable() )
	parsePrintcap( d->printers );
    f.setFile( "/etc/lp/printers/" );
   if ( f.isDir() ) {
	parseEtcLp( d->printers );
	QFile def( "/etc/lp/default" );
	if ( def.open( IO_ReadOnly ) ) {
	    etcLpDefault = new char[1025];
	    def.readLine( etcLpDefault, 1024 );
	    char * p = etcLpDefault;
	    while( p && *p ) {
		if ( !isprint(*p) || isspace(*p) )
		    *p = 0;
		else
		    p++;
	    }
	}
    }

    char * dollarPrinter;
    dollarPrinter = getenv( "PRINTER" );
    int quality = 0;

    const QListViewItem * lvi = d->printers->firstChild();
    d->printers->setCurrentItem( (QListViewItem *)lvi );
    while( lvi ) {
	QRegExp ps1( "[^a-z]ps[^a-z]" );
	QRegExp ps2( "[^a-z]ps$" );
	QRegExp lp1( "[^a-z]lp[^a-z]" );
	QRegExp lp2( "[^a-z]lp$" );
	if ( quality < 4 &&
	     !qstrcmp( lvi->text( 0 ), dollarPrinter ) ) {
	    d->printers->setCurrentItem( (QListViewItem *)lvi );
	    quality = 4;
	} else if ( quality < 3 && etcLpDefault &&
		    !qstrcmp( lvi->text( 0 ), etcLpDefault ) ) {
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

    delete[] etcLpDefault;
#endif

    d->printers->setMinimumSize( 404, fontMetrics().height() * 5 );
    horiz->addWidget( d->printers, 3 );

    tll->addSpacing( 6 );

    // file radio button, edit/browse
    rb = new QRadioButton( tr( "Print to File:" ), g, "file" );
    rb->setMinimumSize( rb->sizeHint() );
    tll->addWidget( rb );
    d->printerOrFile->insert( rb, 1 );

    horiz = new QBoxLayout( QBoxLayout::LeftToRight );
    tll->addLayout( horiz );
    horiz->addSpacing( 19 );

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

    QBoxLayout * tll = new QBoxLayout( g, QBoxLayout::Down, 12, 0 );
    tll->addSpacing( 8 );

    d->printRange = new QButtonGroup( (QWidget *)0 );
    connect( d->printRange, SIGNAL(clicked(int)),
	     this, SLOT(printRangeSelected(int)) );

    d->pageOrder = new QButtonGroup( (QWidget *)0 );
    connect( d->pageOrder, SIGNAL(clicked(int)),
	     this, SLOT(pageOrderSelected(int)) );

    d->colorMode = new QButtonGroup( (QWidget *)0 );
    connect( d->pageOrder, SIGNAL(clicked(int)),
	     this, SLOT(colorModeSelected(int)) );

    d->printAllButton = new QRadioButton( tr("Print all"), g, "print all" );
    d->printAllButton->setMinimumSize( d->printAllButton->sizeHint() );
    d->printRange->insert( d->printAllButton, 0 );
    tll->addWidget( d->printAllButton );

    d->printRangeButton = new QRadioButton( tr("Print Range:"),
					    g, "print range" );
    d->printRangeButton->setMinimumSize( d->printRangeButton->sizeHint() );
    d->printRange->insert( d->printRangeButton, 0 );
    tll->addWidget( d->printRangeButton );

    QBoxLayout * horiz = new QBoxLayout( QBoxLayout::LeftToRight );
    tll->addLayout( horiz );

    d->firstPageLabel = new QLabel( tr("From page"), g, "first page" );
    horiz->addSpacing( 19 );
    horiz->addWidget( d->firstPageLabel );

    d->firstPage = new QSpinBox( 1, 9999, 1, g, "first page" );
    d->firstPage->setValue( 1 );
    d->firstPage->setMinimumSize( d->firstPage->sizeHint() );
    horiz->addWidget( d->firstPage, 1 );
    connect( d->firstPage, SIGNAL(valueChanged(int)),
	     this, SLOT(setFirstPage(int)) );

    horiz = new QBoxLayout( QBoxLayout::LeftToRight );
    tll->addLayout( horiz );

    d->lastPageLabel = new QLabel( tr("To page"), g, "last page" );
    horiz->addSpacing( 19 );
    horiz->addWidget( d->lastPageLabel );

    d->lastPage = new QSpinBox( 1, 9999, 1, g, "last page" );
    d->lastPage->setValue( 9999 );
    d->lastPage->setMinimumSize( d->lastPage->sizeHint() );
    horiz->addWidget( d->lastPage, 1 );
    connect( d->lastPage, SIGNAL(valueChanged(int)),
	     this, SLOT(setLastPage(int)) );

    QFrame * divider = new QFrame( g, "divider", 0, TRUE );
    divider->setFrameStyle( QFrame::HLine + QFrame::Sunken );
    divider->setMinimumHeight( 6 );
    tll->addWidget( divider, 1 );

    // print order
    QRadioButton * rb = new QRadioButton( tr("Print first page first"),
					  g, "first page first" );
    rb->setMinimumSize( rb->sizeHint() );
    tll->addWidget( rb );
    d->pageOrder->insert( rb, QPrinter::FirstPageFirst );
    rb->setChecked( TRUE );

    rb = new QRadioButton( tr("Print last page first"),
			   g, "last page first" );
    rb->setMinimumSize( rb->sizeHint() );
    tll->addWidget( rb );
    d->pageOrder->insert( rb, QPrinter::LastPageFirst );

    divider = new QFrame( g, "divider", 0, TRUE );
    divider->setFrameStyle( QFrame::HLine + QFrame::Sunken );
    divider->setMinimumHeight( 6 );
    tll->addWidget( divider, 1 );

    // color mode
    rb = new QRadioButton( tr("Print in color if available"),
			   g, "color" );
    rb->setMinimumSize( rb->sizeHint() );
    tll->addWidget( rb );
    d->colorMode->insert( rb, QPrinter::Color );
    rb->setChecked( TRUE );

    rb = new QRadioButton( tr("Print in grayscale"),
			   g, "graysacle" );
    rb->setMinimumSize( rb->sizeHint() );
    tll->addWidget( rb );
    d->colorMode->insert( rb, QPrinter::GrayScale );

    divider = new QFrame( g, "divider", 0, TRUE );
    divider->setFrameStyle( QFrame::HLine + QFrame::Sunken );
    divider->setMinimumHeight( 6 );
    tll->addWidget( divider, 1 );

    // copies

    horiz = new QBoxLayout( QBoxLayout::LeftToRight );
    tll->addLayout( horiz );

    QLabel * l = new QLabel( tr("Number of copies"), g, "Number of copies" );
    horiz->addWidget( l );

    QSpinBox * spin = new QSpinBox( 1, 99, 1, g, "copies" );
    spin->setValue( 1 );
    spin->setMinimumSize( spin->sizeHint() );
    horiz->addWidget( spin, 1 );
    connect( spin, SIGNAL(valueChanged(int)),
	     this, SLOT(setNumCopies(int)) );

    QSize s = d->firstPageLabel->sizeHint()
	      .expandedTo( d->lastPageLabel->sizeHint() )
	      .expandedTo( l->sizeHint() );
    d->firstPageLabel->setMinimumSize( s );
    d->lastPageLabel->setMinimumSize( s );
    l->setMinimumSize( s.width() + 19, s.height() );

    tll->activate();

    return g;
}


QGroupBox * QPrintDialog::setupPaper()
{
    QGroupBox * g = new QGroupBox( tr( "Paper format"),
				   this, "Paper format" );

    QBoxLayout * tll = new QBoxLayout( g, QBoxLayout::Down, 12, 0 );
    tll->addSpacing( 8 );

    d->orient = new QButtonGroup( (QWidget*) 0 );
    connect( d->orient, SIGNAL(clicked(int)),
	     this, SLOT(orientSelected(int)) );

    d->paperSize = new QButtonGroup( (QWidget *)0 );
    connect( d->paperSize, SIGNAL(clicked(int)),
	     this, SLOT(paperSizeSelected(int)) );

    // page orientation
    QRadioButton * rb = new QRadioButton( "Portrait", g, "portrait format" );
    rb->setMinimumSize( rb->sizeHint() );
    d->orient->insert( rb, (int)QPrinter::Portrait );
    tll->addWidget( rb );

    rb->setChecked( TRUE );
    d->orientation = QPrinter::Portrait;

    rb = new QRadioButton( "Landscape", g, "landscape format" );
    rb->setMinimumSize( rb->sizeHint() );
    d->orient->insert( rb, (int)QPrinter::Landscape );
    tll->addWidget( rb );

    QFrame * divider = new QFrame( g, "divider", 0, TRUE );
    divider->setFrameStyle( QFrame::HLine + QFrame::Sunken );
    divider->setMinimumHeight( 6 );
    tll->addWidget( divider, 1 );

    // paper size
    rb = new QRadioButton( "A4 (210 x 297 mm)", g, "A4" );
    rb->setMinimumSize( rb->sizeHint() );
    d->paperSize->insert( rb, 3 ); // 3
    rb->setChecked( TRUE );
    d->pageSize = QPrinter::A4;
    tll->addWidget( rb );

    rb = new QRadioButton( "B5", g, "B5" );
    rb->setMinimumSize( rb->sizeHint() );
    d->paperSize->insert( rb, 0 );
    tll->addWidget( rb );

    rb = new QRadioButton( "Letter (8½ x 11in)", g, "Letter" );
    rb->setMinimumSize( rb->sizeHint() );
    d->paperSize->insert( rb, 4 );
    tll->addWidget( rb );

    rb = new QRadioButton( "Legal", g, "Letter" );
    rb->setMinimumSize( rb->sizeHint() );
    d->paperSize->insert( rb, 1 );
    tll->addWidget( rb );

    rb = new QRadioButton( "Executive", g, "Letter" );
    rb->setMinimumSize( rb->sizeHint() );
    d->paperSize->insert( rb, 2 );
    tll->addWidget( rb );

    tll->activate();

    return g;
}


/*!
  Display a dialog and allow the user to configure the QPrinter \a
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

    globalPrintDialog->setPrinter( p );
    bool r = globalPrintDialog->exec() == QDialog::Accepted;
    globalPrintDialog->setPrinter( 0 );
    return r;
}


void QPrintDialog::printerOrFileSelected( int id )
{
    d->outputToFile = id ? TRUE : FALSE;
    if ( d->outputToFile ) {
	d->browse->setEnabled( TRUE );
	d->fileName->setEnabled( TRUE );
	d->fileName->setFocus();
	d->printers->setEnabled( FALSE );
    } else {
	d->printers->setEnabled( TRUE );
	if ( d->fileName->hasFocus() || d->browse->hasFocus() )
	    d->printers->setFocus();
	d->browse->setEnabled( FALSE );
	d->fileName->setEnabled( FALSE );
    }
}


void QPrintDialog::landscapeSelected( int id )
{
    d->orientation = (QPrinter::Orientation)id;
}


void QPrintDialog::paperSizeSelected( int id )
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
	break;
    case 4:
	d->pageSize = QPrinter::Letter;
	break;
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
    d->printer->setColorMode( d->colorMode2 );

    accept();
}


void QPrintDialog::printRangeSelected( int id )
{
    bool enable = id ? TRUE : FALSE;
    d->firstPage->setEnabled( enable );
    d->lastPage->setEnabled( enable );
}


void QPrintDialog::setFirstPage( int fp )
{
    if ( d->printer )
	d->lastPage->setRange( fp, d->printer->maxPage() );
}


void QPrintDialog::setLastPage( int lp )
{
    if ( d->printer )
	d->firstPage->setRange( d->printer->minPage(), lp );
}


/*!  Sets this dialog to configure \a p, or no printer if \a p is
  FALSE.  If \a pickUpSettings is TRUE, the dialog reads most of its
  settings from \a printer.  If \a pickUpSettings is FALSE (the
  default) the dialog keeps its old settings. */

void QPrintDialog::setPrinter( QPrinter * p, bool pickUpSettings )
{
    d->printer = p;

    if ( p && pickUpSettings ) {
	// top to botton in the old dialog.
	// printer or file
	d->printerOrFile->setButton( p->outputToFile() );
	// printer name
	if ( p->printerName() ) {
	    QListViewItem * i = d->printers->firstChild();
	    while( i && qstrcmp( i->text( 0 ), p->printerName() ) )
		i = i->nextSibling();
	    if ( i )
		d->printers->setSelected( i, TRUE );
		
	}
	// print command does not exist any more
	// file name
	d->fileName->setText( p->outputFileName() );
	// orientation
	d->orient->setButton( (int)p->orientation() );
	// page size
	switch( p->pageSize() ) {
	case QPrinter::B5:
	    d->paperSize->setButton( 0 );
	    break;
	case QPrinter::Legal:
	    d->paperSize->setButton( 1 );
	    break;
	case QPrinter::Executive:
	    d->paperSize->setButton( 2 );
	    break;
	case QPrinter::A4:
	default:
	    d->paperSize->setButton( 3 );
	    break;
	case QPrinter::Letter:
	    d->paperSize->setButton( 4 );
	    break;
	}	
	// also some new stuff.
	d->pageOrder->setButton( (int)p->pageOrder() );
	// more new stuff can be set, but it'll be difficult to get
	// right.
    }

    if ( p && p->maxPage() ) {
	d->printRangeButton->setEnabled( TRUE );
	d->firstPage->setEnabled( TRUE );
	d->firstPage->setRange( p->minPage(), p->maxPage() );
	d->lastPage->setEnabled( TRUE );
	d->lastPage->setRange( p->minPage(), p->maxPage() );
	d->firstPageLabel->setEnabled( TRUE );
	d->lastPageLabel->setEnabled( TRUE );
	d->firstPage->setValue( p->minPage() );
	d->lastPage->setValue( p->maxPage() );
    } else {
	d->printRange->setButton( 0 );	
	d->printRangeButton->setEnabled( FALSE );
	d->firstPage->setEnabled( FALSE );
	d->lastPage->setEnabled( FALSE );
	d->firstPageLabel->setEnabled( FALSE );
	d->lastPageLabel->setEnabled( FALSE );
	d->firstPage->setValue( 1 );
	d->lastPage->setValue( 1 );
    }
}


/*!  Returns a pointer to the printer this dialog configures, or 0 if
  this dialog does not operate on any printer. */

QPrinter * QPrintDialog::printer() const
{
    return d->printer;
}


void QPrintDialog::colorModeSelected( int id )
{
    d->colorMode2 = (QPrinter::ColorMode)id;
}
