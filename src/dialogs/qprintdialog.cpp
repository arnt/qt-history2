/****************************************************************************
** $Id: //depot/qt/main/src/dialogs/qprintdialog.cpp#82 $
**
** Implementation of internal print dialog (X11) used by QPrinter::select().
**
** Created : 950829
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qprintdialog.h"

#include "qfiledialog.h"

#include "qfile.h"
#include "qtextstream.h"

#include "qcombobox.h"
#include "qframe.h"
#include "qlabel.h"
#include "qlineedit.h"
#include "qpushbutton.h"
#include "qprinter.h"
#include "qlistview.h"
#include "qlayout.h"
#include "qbuttongroup.h"
#include "qradiobutton.h"
#include "qspinbox.h"
#include "qapplication.h"
#include "qheader.h"

#include "qstring.h"
#include "qregexp.h"

#include <ctype.h>
#include <stdlib.h>

// REVISED: warwick

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

    QSpinBox * copies;
    int numCopies;
};


static void perhapsAddPrinter( QListView * printers, const QString &name,
			       QString host, QString comment )
{
    const QListViewItem * i = printers->firstChild();
    while( i && i->text( 0 ) != name )
	i = i->nextSibling();
    if ( i )
	return;
    if ( host.isEmpty() )
	host = qApp->translate( "QPrintDialog", "locally connected" );
    (void)new QListViewItem( printers,
			     name.simplifyWhiteSpace(),
			     host.simplifyWhiteSpace(),
			     comment.simplifyWhiteSpace() );
}


static void parsePrintcap( QListView * printers )
{
    QFile printcap( QString::fromLatin1("/etc/printcap") );
    if ( !printcap.open( IO_ReadOnly ) )
	return;

    char * line = new char[1025];
    line[1024] = '\0';

    QString printerDesc;
    int lineLength = 0;

    while( !printcap.atEnd() &&
	   (lineLength=printcap.readLine( line, 1024 )) > 0 ) {
	if ( *line == '#' ) {
	    *line = '\0';
	    lineLength = 0;
	}
	if ( lineLength >= 2 && line[lineLength-2] == '\\' ) {
	    line[lineLength-2] = '\0';
	    printerDesc += QString::fromLocal8Bit(line);
	} else {
	    printerDesc += QString::fromLocal8Bit(line);
	    printerDesc = printerDesc.simplifyWhiteSpace();
	    int i = printerDesc.find( ':' );
	    QString printerName, printerComment, printerHost;
	    if ( i >= 0 ) {
		// have : want |
		int j = printerDesc.find( '|' );
		printerName = printerDesc.left( j > 0 ? j : i );
		if ( j > 0 ) {
		    // try extracting a comment from the aliases...
		    printerComment = qApp->translate( "QPrintDialog",
						      "Aliases: " );
		    printerComment += printerDesc.mid( 0, j );
		    j=printerComment.length();
		    while( j > 0 ) {
			j--;
			if ( printerComment[j] == '|' )
			    printerComment[j] = ',';
		    }
		}
		// then look for a real comment
		j = i+1;
		while( printerDesc[j].isSpace() )
		    j++;
		if ( printerDesc[j] != ':' ) {
		    printerComment = printerDesc.mid( i, j-i );
		    printerComment = printerComment.simplifyWhiteSpace();
		}
 		// look for lprng psuedo all printers entry
 		i = printerDesc.find( QRegExp( ": *all *=" ) );
 		if ( i >= 0 )
		    printerName = "";
		// look for signs of this being a remote printer
		i = printerDesc.find(
			QRegExp( QString::fromLatin1(": *rm *=") ) );
		if ( i >= 0 ) {
		    // point k at the end of remote host name
		    while( printerDesc[i] != '=' )
			i++;
		    while( printerDesc[i] == '=' || printerDesc[i].isSpace() )
			i++;
		    j = i;
		    while( j < (int)printerDesc.length() && printerDesc[j] != ':' )
			j++;

		    // and stuff that into the string
		    printerHost = printerDesc.mid( i, j-i );
		}
	    }
	    if ( printerName.length() )
		perhapsAddPrinter( printers, printerName, printerHost,
				   printerComment );
	    // chop away the line, for processing the next one
	    printerDesc = "";
	}
    }
    delete[] line;
}


// solaris, not 2.6
static void parseEtcLpPrinters( QListView * printers )
{
    QDir lp( QString::fromLatin1("/etc/lp/printers") );
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
			 printer->fileName().ascii() );
	    QFile configuration( tmp );
	    char * line = new char[1025];
	    QRegExp remote( QString::fromLatin1("^Remote:") );
	    QRegExp contentType( QString::fromLatin1("^Content types:") );
	    QString printerHost;
	    bool canPrintPostscript = FALSE;
	    if ( configuration.open( IO_ReadOnly ) ) {
		while( !configuration.atEnd() &&
		       configuration.readLine( line, 1024 ) > 0 ) {
		    if ( remote.match( QString::fromLatin1(line) ) == 0 ) {
			const char * p = line;
			while( *p != ':' )
			    p++;
			p++;
			while( isspace(*p) )
			    p++;
			printerHost = QString::fromLocal8Bit(p);
			printerHost = printerHost.simplifyWhiteSpace();
		    } else if ( contentType.match( QString::fromLatin1(line) ) == 0 ) {
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
		    perhapsAddPrinter( printers, printer->fileName(),
				       printerHost, QString::fromLatin1("") );
	    }
	    delete[] line;
	}
    }
}


// solaris 2.6
static char * parsePrintersConf( QListView * printers )
{
    QFile pc( QString::fromLatin1("/etc/printers.conf") );
    if ( !pc.open( IO_ReadOnly ) )
	return 0;

    char * line = new char[1025];
    line[1024] = '\0';

    QString printerDesc;
    int lineLength = 0;

    char * defaultPrinter = 0;

    while( !pc.atEnd() &&
	   (lineLength=pc.readLine( line, 1024 )) > 0 ) {
	if ( *line == '#' ) {
	    *line = '\0';
	    lineLength = 0;
	}
	if ( lineLength >= 2 && line[lineLength-2] == '\\' ) {
	    line[lineLength-2] = '\0';
	    printerDesc += QString::fromLocal8Bit(line);
	} else {
	    printerDesc += QString::fromLocal8Bit(line);
	    printerDesc = printerDesc.simplifyWhiteSpace();
	    int i = printerDesc.find( ':' );
	    QString printerName, printerHost, printerComment;
	    if ( i >= 0 ) {
		// have : want |
		int j = printerDesc.find( '|', 0 );
		if ( j >= i )
		    j = -1;
		printerName = printerDesc.mid( 0, j < 0 ? i : j );
		if ( printerName == QString::fromLatin1("_default") ) {
		    i = printerDesc.find(
			QRegExp( QString::fromLatin1(": *use *=") ) );
		    while( printerDesc[i] != '=' )
			i++;
		    while( printerDesc[i] == '=' || printerDesc[i].isSpace() )
			i++;
		    j = i;
		    while( j < (int)printerDesc.length() &&
			   printerDesc[j] != ':' &&
			   printerDesc[j] != ',' )
			j++;
		    // that's our default printer
		    defaultPrinter = qstrdup( printerDesc.mid( i, j-i ).ascii() );
		    printerName = "";
		    printerDesc = "";
		} else if ( printerName == QString::fromLatin1("_all") ) {
		    // skip it.. any other cases we want to skip?
		    printerName = "";
		    printerDesc = "";
		}

		if ( j > 0 ) {
		    // try extracting a comment from the aliases...
		    printerComment = qApp->translate( "QPrintDialog",
						      "Aliases: " );
		    printerComment += printerDesc.mid( j+1, i-j-1 );
		    for( j=printerComment.length(); j>-1; j-- )
			if ( printerComment[j] == '|' )
			    printerComment[j] = ',';
		}
		// look for signs of this being a remote printer
		i = printerDesc.find(
		    QRegExp( QString::fromLatin1(": *bsdaddr *=") ) );
		if ( i >= 0 ) {
		    // point k at the end of remote host name
		    while( printerDesc[i] != '=' )
			i++;
		    while( printerDesc[i] == '=' || printerDesc[i].isSpace() )
			i++;
		    j = i;
		    while( j < (int)printerDesc.length() &&
			   printerDesc[j] != ':' &&
			   printerDesc[j] != ',' )
			j++;
		    // and stuff that into the string
		    printerHost = printerDesc.mid( i, j-i );
		    // maybe stick the remote printer name into the comment
		    if ( printerDesc[j] == ',' ) {
			i = ++j;
			while( printerDesc[i].isSpace() )
			    i++;
			j = i;
			while( j < (int)printerDesc.length() &&
			       printerDesc[j] != ':' &&
			       printerDesc[j] != ',' )
			    j++;
			if ( printerName != printerDesc.mid( i, j-i ) ) {
			    printerComment = QString::fromLatin1("Remote name: ");
			    printerComment += printerDesc.mid( i, j-i );
			}
		    }
		}
	    }
	    if ( printerName.length() )
		perhapsAddPrinter( printers, printerName, printerHost,
				   printerComment );
	    // chop away the line, for processing the next one
	    printerDesc = "";
	}
    }
    delete[] line;
    return defaultPrinter;
}



// HP-UX
static void parseEtcLpMember( QListView * printers )
{
    QDir lp( QString::fromLatin1("/etc/lp/member") );
    if ( !lp.exists() )
	return;
    const QFileInfoList * dirs = lp.entryInfoList();
    if ( !dirs )
	return;

    QFileInfoListIterator it( *dirs );
    QFileInfo *printer;
    QString tmp;
    while ( (printer = it.current()) != 0 ) {
	++it;
	// uglehack.
	// I haven't found any real documentation, so I'm guessing that
	// since lpstat uses /etc/lp/member rather than one of the
	// other directories, it's the one to use.  I did not find a
	// decent way to locate aliases and remote printers.
	if ( printer->isFile() )
	    perhapsAddPrinter( printers, printer->fileName(),
			       qApp->translate( "QPrintDialog","unknown"),
				QString::fromLatin1("") );
    }
}

// IRIX 6.x
static void parseSpoolInterface( QListView * printers )
{
    QDir lp( QString::fromLatin1("/usr/spool/lp/interface") );
    if ( !lp.exists() )
	return;
    const QFileInfoList * files = lp.entryInfoList();
    if( !files )
	return;

    QFileInfoListIterator it( *files );
    QFileInfo *printer;
    while ( (printer = it.current()) != 0) {
	++it;

	if ( !printer->isFile() )
	    continue;

	// parse out some information
	QFile configFile( printer->filePath() );
	if ( !configFile.open( IO_ReadOnly ) )
	    continue;

	QCString line(1025);
	QString hostName;
	QString hostPrinter;
	QString printerType;

	QRegExp typeKey(QString::fromLatin1("^TYPE="));
	QRegExp hostKey(QString::fromLatin1("^HOSTNAME="));
	QRegExp hostPrinterKey(QString::fromLatin1("^HOSTPRINTER="));
	int length;

	while( !configFile.atEnd() &&
	    (configFile.readLine( line.data(), 1024 )) > 0 ) {

	    if(typeKey.match(line, 0, &length) == 0) {
		printerType = line.mid(length, line.length()-length);
		printerType = printerType.simplifyWhiteSpace();
	    }
	    if(hostKey.match(line, 0, &length) == 0) {
		hostName = line.mid(length, line.length()-length);
		hostName = hostName.simplifyWhiteSpace();
	    }
	    if(hostPrinterKey.match(line, 0, &length) == 0) {
		hostPrinter = line.mid(length, line.length()-length);
		hostPrinter = hostPrinter.simplifyWhiteSpace();
	    }
	}
	configFile.close();

	printerType = printerType.stripWhiteSpace();
	if ( !printerType.isEmpty() && qstricmp( printerType.ascii(), "postscript" ))
	    continue;

	if(hostName.isEmpty() || hostPrinter.isEmpty())
	{
	    perhapsAddPrinter( printers, printer->fileName(),
		QString::fromLatin1(""), QString::fromLatin1(""));
	} else
	{
	    QString comment = QString::fromLatin1("Remote name: ");
	    comment += hostPrinter;
	    perhapsAddPrinter( printers, printer->fileName(),
		hostName, comment);
	}
    }
}


// Every unix must have its own.  It's a standard.  Here is AIX.
static void parseQconfig( QListView * printers )
{
    QFile qconfig( QString::fromLatin1("/etc/qconfig") );
    if ( !qconfig.open( IO_ReadOnly ) )
	return;

    QTextStream ts( &qconfig );
    QString line;

    QString stanzaName; // either a queue or a device name
    bool up = TRUE; // queue up?  default TRUE, can be FALSE
    QString remoteHost; // null if local
    QString deviceName; // null if remote

    // our basic strategy here is to process each line, detecting new
    // stanzas.  each time we see a new stanza, we check if the
    // previous stanza was a valid queue for a) a remote printer or b)
    // a local printer.  if it wasn't, we assume that what we see is
    // the start of the first stanza, or that the previous stanza was
    // a device stanza, or that there is some syntax error (we don't
    // report those).

    do {
	line = ts.readLine();
	bool indented = line[0].isSpace();
	line = line.simplifyWhiteSpace();

	if ( indented && line.contains( '=' ) ) { // line in stanza
	
	    int i = line.find( '=' );
	    QString variable = line.left( i ).simplifyWhiteSpace();
	    QString value=line.mid( i+1, line.length() ).simplifyWhiteSpace();
	    if ( variable == QString::fromLatin1("device") )
		deviceName = value;
	    else if ( variable == QString::fromLatin1("host") )
		remoteHost = value;
	    else if ( variable == QString::fromLatin1("up") )
		up = !(value.lower() == QString::fromLatin1("false"));
	} else if ( line[0] == '*' ) { // comment
	    // nothing to do
	} else if ( ts.atEnd() || // end of file, or beginning of new stanza
		    ( !indented &&
		      line.contains(
			QRegExp( QString::fromLatin1("^[0-z][0-z]*:$") ) ) ) ) {
	    if ( up && stanzaName.length() > 0 && stanzaName.length() < 21 ) {
		if ( remoteHost.length() ) // remote printer
		    perhapsAddPrinter( printers, stanzaName, remoteHost,
				       QString::null );
		else if ( deviceName.length() ) // local printer
		    perhapsAddPrinter( printers, stanzaName, QString::null,
				       QString::null );
	    }
	    line.truncate( line.length()-1 );
	    if ( line.length() >= 1 && line.length() <= 20 )
		stanzaName = line;
	    up = TRUE;
	    remoteHost = QString::null;
	    deviceName = QString::null;
	} else {
	    // syntax error?  ignore.
	}
    } while( !ts.atEnd() );
}


static QPrintDialog * globalPrintDialog = 0;

static void deleteGlobalPrintDialog()
{
    delete globalPrintDialog;
    globalPrintDialog = 0;
}

/*!
  \class QPrintDialog qprintdialog.h

  \brief The QPrintDialog class provides a dialog for specifying
  print-out details.

  \ingroup dialogs

  \warning This class is not present on all platforms; use
  QPrinter::setup() instead for portability.

  \internal
  THIS DOCUMENTATION IS Not Revised. It must be revised before
  becoming public API.

  It encompasses both the sort of details needed for doing a simple
  print-out and some print configuration setup.

  At present, the only easy way to use the class is through the static
  function getPrinterSetup().  You can however also call the global
  QPrintDialog::getPrinterConfigure(), or subclass in order to extend
  one of the group boxes.

  Note that in 1.40 the printer dialog is a little too tall for
  comfortable use on a small-screen machine.  This will be improved on
  in 2.1.

  <img src="printerdialog.png"><br clear=all>
  The printer dialog, on a large screen, in Motif style.
*/


/*! Constructs a new modal printer dialog that configures \a prn and is a
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
    ok->setText( tr("OK") );
    ok->setDefault( TRUE );
    horiz->addWidget( ok );
    if ( style() == MotifStyle )
	horiz->addStretch( 1 );
    horiz->addSpacing( 6 );

    QPushButton * cancel = new QPushButton( this, "cancel" );
    cancel->setText( tr("Cancel") );
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

    setFontPropagation( SameFont );
    setPalettePropagation( SamePalette );
}


/*! Destroys the object and frees any allocated resources.  Does not
  delete the associated QPrinter object.
*/

QPrintDialog::~QPrintDialog()
{
    if ( this == globalPrintDialog )
	globalPrintDialog = 0;
    delete d;
}


QGroupBox * QPrintDialog::setupDestination()
{
    QGroupBox * g = new QGroupBox( tr( "Print destination"),
				   this, "destination group box" );

    QBoxLayout * tll = new QBoxLayout( g, QBoxLayout::Down, 12, 0 );
    tll->addSpacing( 8 );

    d->printerOrFile = new QButtonGroup( this );
    d->printerOrFile->hide();
    connect( d->printerOrFile, SIGNAL(clicked(int)),
	     this, SLOT(printerOrFileSelected(int)) );

    // printer radio button, list
    QRadioButton * rb = new QRadioButton( tr( "Print to printer:" ), g,
					  "printer" );
    tll->addWidget( rb );
    d->printerOrFile->insert( rb, 0 );
    rb->setChecked( TRUE );
    d->outputToFile = FALSE;

    QBoxLayout * horiz = new QBoxLayout( QBoxLayout::LeftToRight );
    tll->addLayout( horiz, 3 );
    horiz->addSpacing( 19 );

    d->printers = new QListView( g, "list of printers" );
    d->printers->setAllColumnsShowFocus( TRUE );
    d->printers->addColumn( tr("Printer"), 125 );
    d->printers->addColumn( tr("Host"), 125 );
    d->printers->addColumn( tr("Comment"), 150 );
    d->printers->setFrameStyle( QFrame::WinPanel + QFrame::Sunken );

#if defined(UNIX)
    char * etcLpDefault = 0;

    parsePrintcap( d->printers );
    parseEtcLpMember( d->printers );
    parseSpoolInterface( d->printers );
    parseQconfig( d->printers );

    QFileInfo f;
    f.setFile( QString::fromLatin1("/etc/lp/printers") );
    if ( f.isDir() ) {
	parseEtcLpPrinters( d->printers );
	QFile def( QString::fromLatin1("/etc/lp/default") );
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

    f.setFile( QString::fromLatin1("/etc/printers.conf") );
    if ( f.isFile() ) {
	char * def = parsePrintersConf( d->printers );
	if ( def ) {
	    if ( etcLpDefault )
		delete[] etcLpDefault;
	    etcLpDefault = def;
	}
    }

    // all printers hopefully known.  try to find a good default
    QString dollarPrinter;
    {
	char * t;
	t = getenv( "PRINTER" );
	if ( !t || !*t )
	    t = getenv( "LPDEST" );
	dollarPrinter = QString::fromLatin1(t);
    }
    int quality = 0;

    // bang the best default into the listview
    const QListViewItem * lvi = d->printers->firstChild();
    d->printers->setCurrentItem( (QListViewItem *)lvi );
    while( lvi ) {
	QRegExp ps1( QString::fromLatin1("[^a-z]ps[^a-z]") );
	QRegExp ps2( QString::fromLatin1("[^a-z]ps$") );
	QRegExp lp1( QString::fromLatin1("[^a-z]lp[^a-z]") );
	QRegExp lp2( QString::fromLatin1("[^a-z]lp$") );
	if ( quality < 4 &&
	     lvi->text( 0 ) == dollarPrinter ) {
	    d->printers->setCurrentItem( (QListViewItem *)lvi );
	    quality = 4;
	} else if ( quality < 3 && etcLpDefault &&
		    lvi->text( 0 ) == QString::fromLatin1(etcLpDefault) ) {
	    d->printers->setCurrentItem( (QListViewItem *)lvi );
	    quality = 3;
	} else if ( quality < 2 &&
		    ( lvi->text( 0 ) == QString::fromLatin1("ps") ||
		      ps1.match( lvi->text( 2 ) ) > -1 ||
		      ps2.match( lvi->text( 2 ) ) > -1 ) ) {
	    d->printers->setCurrentItem( (QListViewItem *)lvi );
	    quality = 2;
	} else if ( quality < 1 &&
		    ( lvi->text( 0 ) == QString::fromLatin1("lp") ||
		      lp1.match( lvi->text( 2 ) ) > -1 ||
		      lp2.match( lvi->text( 2 ) ) > -1 ) ) {
	    d->printers->setCurrentItem( (QListViewItem *)lvi );
	    quality = 1;
	}
	lvi = lvi->nextSibling();
    }
    if ( d->printers->currentItem() )
	d->printers->setSelected( d->printers->currentItem(), TRUE );

    if ( etcLpDefault )			// Avoid purify complaint
	delete[] etcLpDefault;
#endif

    int h = fontMetrics().height();
    if ( d->printers->firstChild() )
	h = d->printers->firstChild()->height();
    d->printers->setMinimumSize( d->printers->sizeHint().width(),
				 d->printers->header()->height() +
				  3 * h );
    horiz->addWidget( d->printers, 3 );

    tll->addSpacing( 6 );

    // file radio button, edit/browse
    rb = new QRadioButton( tr( "Print to file:" ), g, "file" );
    tll->addWidget( rb );
    d->printerOrFile->insert( rb, 1 );

    horiz = new QBoxLayout( QBoxLayout::LeftToRight );
    tll->addLayout( horiz );
    horiz->addSpacing( 19 );

    d->fileName = new QLineEdit( g, "file name" );
    horiz->addWidget( d->fileName, 1 );
    horiz->addSpacing( 6 );
    d->browse = new QPushButton( tr("Browse..."), g, "browse files" );
    d->browse->setAutoDefault( FALSE );
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

    QBoxLayout * tll = new QBoxLayout( g, QBoxLayout::Down, 12, 2 );
    tll->addSpacing( 8 );

    d->printRange = new QButtonGroup( this );
    d->printRange->hide();
    connect( d->printRange, SIGNAL(clicked(int)),
	     this, SLOT(printRangeSelected(int)) );

    d->pageOrder = new QButtonGroup( this );
    d->pageOrder->hide();
    connect( d->pageOrder, SIGNAL(clicked(int)),
	     this, SLOT(pageOrderSelected(int)) );

    d->colorMode = new QButtonGroup( this );
    d->colorMode->hide();
    connect( d->colorMode, SIGNAL(clicked(int)),
	     this, SLOT(colorModeSelected(int)) );

    d->printAllButton = new QRadioButton( tr("Print all"), g, "print all" );
    d->printRange->insert( d->printAllButton, 0 );
    tll->addWidget( d->printAllButton );

    d->printRangeButton = new QRadioButton( tr("Print range"),
					    g, "print range" );
    d->printRange->insert( d->printRangeButton, 1 );
    tll->addWidget( d->printRangeButton );

    QBoxLayout * horiz = new QBoxLayout( QBoxLayout::LeftToRight );
    tll->addLayout( horiz );

    d->firstPageLabel = new QLabel( tr("From page:"), g, "first page" );
    horiz->addSpacing( 19 );
    horiz->addWidget( d->firstPageLabel );

    d->firstPage = new QSpinBox( 1, 9999, 1, g, "first page" );
    d->firstPage->setValue( 1 );
    horiz->addWidget( d->firstPage, 1 );
    connect( d->firstPage, SIGNAL(valueChanged(int)),
	     this, SLOT(setFirstPage(int)) );

    horiz = new QBoxLayout( QBoxLayout::LeftToRight );
    tll->addLayout( horiz );

    d->lastPageLabel = new QLabel( tr("To page:"), g, "last page" );
    horiz->addSpacing( 19 );
    horiz->addWidget( d->lastPageLabel );

    d->lastPage = new QSpinBox( 1, 9999, 1, g, "last page" );
    d->lastPage->setValue( 9999 );
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
    tll->addWidget( rb );
    d->pageOrder->insert( rb, QPrinter::FirstPageFirst );
    rb->setChecked( TRUE );

    rb = new QRadioButton( tr("Print last page first"),
			   g, "last page first" );
    tll->addWidget( rb );
    d->pageOrder->insert( rb, QPrinter::LastPageFirst );

    divider = new QFrame( g, "divider", 0, TRUE );
    divider->setFrameStyle( QFrame::HLine + QFrame::Sunken );
    divider->setMinimumHeight( 6 );
    tll->addWidget( divider, 1 );

    // color mode
    rb = new QRadioButton( tr("Print in color if available"),
			   g, "color" );
    tll->addWidget( rb );
    d->colorMode->insert( rb, QPrinter::Color );
    rb->setChecked( TRUE );

    rb = new QRadioButton( tr("Print in grayscale"),
			   g, "graysacle" );
    tll->addWidget( rb );
    d->colorMode->insert( rb, QPrinter::GrayScale );

    divider = new QFrame( g, "divider", 0, TRUE );
    divider->setFrameStyle( QFrame::HLine + QFrame::Sunken );
    divider->setMinimumHeight( 6 );
    tll->addWidget( divider, 1 );

    // copies

    horiz = new QBoxLayout( QBoxLayout::LeftToRight );
    tll->addLayout( horiz );

    QLabel * l = new QLabel( tr("Number of copies:"), g, "Number of copies" );
    horiz->addWidget( l );

    d->copies = new QSpinBox( 1, 99, 1, g, "copies" );
    d->copies->setValue( 1 );
    horiz->addWidget( d->copies, 1 );
    connect( d->copies, SIGNAL(valueChanged(int)),
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

    d->orient = new QButtonGroup( this );
    d->orient->hide();
    connect( d->orient, SIGNAL(clicked(int)),
	     this, SLOT(orientSelected(int)) );

    d->paperSize = new QButtonGroup( this );
    d->paperSize->hide();
    connect( d->paperSize, SIGNAL(clicked(int)),
	     this, SLOT(paperSizeSelected(int)) );

    // page orientation
    QRadioButton * rb = new QRadioButton( tr("Portrait"),
					  g, "portrait format" );
    d->orient->insert( rb, (int)QPrinter::Portrait );
    tll->addWidget( rb );

    rb->setChecked( TRUE );
    d->orientation = QPrinter::Portrait;

    rb = new QRadioButton( tr("Landscape"), g, "landscape format" );
    d->orient->insert( rb, (int)QPrinter::Landscape );
    tll->addWidget( rb );

    QFrame * divider = new QFrame( g, "divider", 0, TRUE );
    divider->setFrameStyle( QFrame::HLine + QFrame::Sunken );
    divider->setMinimumHeight( 6 );
    tll->addWidget( divider, 1 );

    // paper size
    rb = new QRadioButton( QString::fromLatin1("A4 (210 x 297 mm)"), g, "A4" );
    d->paperSize->insert( rb, 0 );
    rb->setChecked( TRUE );
    d->pageSize = QPrinter::A4;
    tll->addWidget( rb );

    rb = new QRadioButton( QString::fromLatin1("B5"), g, "B5" );
    d->paperSize->insert( rb, 1 );
    tll->addWidget( rb );

    rb = new QRadioButton( g, "Letter" );

    // This is really just exercising QFontMetrics::inFont()...
    QString letter_name = "Letter (8";
    if ( rb->fontMetrics().inFont(QChar('½')) )
	letter_name += "½";
    else
	letter_name += ".5";
    letter_name += " x 11 in.)";
    rb->setText(letter_name);

    d->paperSize->insert( rb, 2 );
    tll->addWidget( rb );

    rb = new QRadioButton( QString::fromLatin1("Legal"), g, "Letter" );
    d->paperSize->insert( rb, 3 );
    tll->addWidget( rb );

    rb = new QRadioButton( QString::fromLatin1("Executive"), g, "Letter" );
    d->paperSize->insert( rb, 4 );
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
	globalPrintDialog->setCaption( QPrintDialog::tr( "Setup Printer" ) );
	qAddPostRoutine( deleteGlobalPrintDialog );
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
    d->pageSize = QPrinter::PageSize(id);
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
    QString fn = QFileDialog::getSaveFileName( QString::null, tr( "Postscript files (*.ps);;All files (*)" ), this );
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
    d->printer->setNumCopies( d->numCopies );
    if ( d->printAllButton->isChecked() )
	d->printer->setFromTo( d->printer->minPage(), d->printer->maxPage() );
    else
	d->printer->setFromTo( d->firstPage->value(), d->lastPage->value() );

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
	d->lastPage->setRange( fp, QMAX(fp, QPrintDialog::d->printer->maxPage()) );
}


void QPrintDialog::setLastPage( int lp )
{
    if ( d->printer )
	d->firstPage->setRange( QMIN(lp, QPrintDialog::d->printer->minPage()), lp );
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
	printerOrFileSelected( p->outputToFile() );

	// printer name
	if ( !!p->printerName() ) {
	    QListViewItem * i = d->printers->firstChild();
	    while( i && i->text( 0 ) != p->printerName() )
		i = i->nextSibling();
	    if ( i )
		d->printers->setSelected( i, TRUE );
	}

	// print command does not exist any more

	// file name
	d->fileName->setText( p->outputFileName() );

	// orientation
	d->orient->setButton( (int)p->orientation() );
	orientSelected( p->orientation() );

	// page size
	d->paperSize->setButton( p->pageSize() );
	paperSizeSelected( p->pageSize() );

	// New stuff (Options)

	// page order
	d->pageOrder->setButton( (int)p->pageOrder() );
	pageOrderSelected( p->pageOrder() );

	// color mode
	d->colorMode->setButton( (int)p->colorMode() );
	colorModeSelected( p->colorMode() );

	// number of copies
	d->copies->setValue( p->numCopies() );
	setNumCopies( p->numCopies() );
    }

    if ( p && p->maxPage() ) {
	d->printRangeButton->setEnabled( TRUE );
	d->firstPage->setRange( p->minPage(), p->maxPage() );
	d->lastPage->setRange( p->minPage(), p->maxPage() );
	// page range
	int some = p->maxPage()
		&& p->fromPage() && p->toPage()
		&& (p->fromPage() != p->minPage()
		    || p->toPage() != p->maxPage());
	if ( p->fromPage() ) {
	    setFirstPage( p->fromPage() );
	    setLastPage( p->toPage() );
	    d->firstPage->setValue(p->fromPage());
	    d->lastPage->setValue(p->toPage());
	}
	d->printRange->setButton( some );
	printRangeSelected( some );
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


/*! \base64 printerdialog.png

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

*/
