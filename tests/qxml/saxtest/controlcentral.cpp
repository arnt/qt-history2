#include <qapplication.h>
#include <qfiledialog.h>
#include <qhbox.h>
#include <qdatetime.h>
#include <qxml.h>
#include <qbuffer.h>

#include "controlcentral.h"
#include "xmlparser.h"
#include "xmlfileitem.h"
#include "itemtextview.h"


//
// ControlCentral
// 
ControlCentral::ControlCentral() : QVBox()
{

    lview = new QListView( this );
    lview->addColumn( "File" );
    lview->addColumn( "Status" );
    lview->addColumn( "Time" );

    QHBox *hbox= new QHBox( this );
    QPushButton *source = new QPushButton( "Source", hbox );
    QPushButton *parseProt = new QPushButton( "Parse Protocol", hbox );
    QPushButton *errorProt = new QPushButton( "Error Protocol", hbox );
    QPushButton *tree = new QPushButton( "Tree", hbox );

    hbox= new QHBox( this );
    incSteps = new QLineEdit( hbox );
    QPushButton *iParse = new QPushButton( "Incremental Parse", hbox );

    quit = new QPushButton( "Quit", this );

    connect( source, SIGNAL(clicked()), this, SLOT(showSource()) );
    connect( parseProt, SIGNAL(clicked()), this, SLOT(showParseProtocol()) );
    connect( errorProt, SIGNAL(clicked()), this, SLOT(showErrorProtocol()) );
    connect( tree, SIGNAL(clicked()), this, SLOT(showTree()) );
    connect( iParse, SIGNAL(clicked()), this, SLOT(incrementalParse()) );
    connect( quit, SIGNAL(clicked()), qApp, SLOT(quit()) );

    parseProtocolFile = new QFile( "parseProtocol" );
    if ( !parseProtocolFile->open( IO_WriteOnly | IO_Truncate ) ) {
	parseProtocolFile = 0;
	parseProtocolTS = 0;
    } else {
	parseProtocolTS = new QTextStream( parseProtocolFile );
    }

    parsePerformanceFile = new QFile( "parsePerformance" );
    if ( !parsePerformanceFile->open( IO_WriteOnly | IO_Truncate ) ) {
	parsePerformanceFile = 0;
	parsePerformanceTS = 0;
    } else {
	parsePerformanceTS = new QTextStream( parsePerformanceFile );
    }
}

ControlCentral::~ControlCentral()
{
    if ( parseProtocolFile != 0 ) {
	parseProtocolFile->close();
	delete parseProtocolFile;
	delete parseProtocolTS;
    }

    if ( parsePerformanceFile != 0 ) {
	parsePerformanceFile->close();
	delete parsePerformanceFile;
	delete parsePerformanceTS;
    }
}

QSize ControlCentral::sizeHint() const
{
    return QSize( 500, 250 );
}

void ControlCentral::parse( const QString& filename, const QString& incrementalSteps )
{
    qDebug( "parse %s", filename.latin1() );
    QString time;

    QFile file( filename );
    if ( !file.open(IO_ReadOnly) ) {
	return;
    }

    QXmlSimpleReader parser;
    QXmlInputSource source( &file );
    QString sourceData = source.data();

    parser.setFeature( "http://xml.org/sax/features/namespaces", TRUE );
    parser.setFeature( "http://xml.org/sax/features/namespace-prefixes", TRUE );
    parser.setFeature( "http://trolltech.com/xml/features/report-whitespace-only-CharData", FALSE );
    parser.setFeature( "http://trolltech.com/xml/features/report-start-end-entity", TRUE );

    QTextView* src = new QTextView();
    src->setCaption( "Source for " + filename );
    src->setTextFormat( PlainText );

    if ( incrementalSteps.isNull() ) {
	*parseProtocolTS << endl
	    << "******** " << filename << " ********" << endl;

	QTime t;
	t.start();
	for ( int i=0; i<10; i++ ) {
	    source.reset();
	    parser.parse( source );
	}
	double ms = ((double)t.elapsed()) / 10;
	time.setNum( ms );
	time += " ms";

	*parsePerformanceTS << endl
	    << "************* " << filename << " *************" << endl
	    << "TIME: " << time << endl
	    << "    namespaces:                      " << (int)parser.feature( "http://xml.org/sax/features/namespaces" ) << endl
	    << "    namespace-prefixes:              " << (int)parser.feature( "http://xml.org/sax/features/namespace-prefixes" ) << endl
	    << "    report-whitespace-only-CharData: " << (int)parser.feature( "http://trolltech.com/xml/features/report-whitespace-only-CharData" ) << endl
	    << "    report-stat-end-entity:          " << (int)parser.feature( "http://trolltech.com/xml/features/report-start-end-entity" ) << endl;

	file.reset();

	src->setText( sourceData );
    }

    QListView* protocol = new QListView;
    protocol->addColumn( "Function" );
    protocol->addColumn( "Arguments" );
    protocol->addColumn( "Line" );
    protocol->addColumn( "Column" );
    protocol->setSorting( -1 );
    protocol->setCaption( "Protocol for " + filename );

    QLabel *err = new QLabel( "", 0 );
    err->setCaption( "Error protocol for " + filename );

    QHBox* tree = new QHBox;
    tree->setCaption( "Tree for " + filename );
    QListView* listTree = new QListView( tree );
    listTree->addColumn(  "XML Tree" );
    listTree->addColumn( "Namespace URI" );
    listTree->addColumn( "Local Name" );
    listTree->addColumn( "" );
    listTree->setSorting( -1 );
    ItemTextView* contentTree = new ItemTextView( tree );
    contentTree->setIndex( 4 );
    connect( listTree, SIGNAL(selectionChanged(QListViewItem*)),
	    contentTree, SLOT(change(QListViewItem*)) );

    QFileInfo finfo( filename );
    XMLParser hnd( protocol, err, listTree, parseProtocolTS, finfo.dir(TRUE) );
    parser.setEntityResolver( &hnd );
    parser.setDTDHandler( &hnd );
    parser.setContentHandler( &hnd );
    parser.setErrorHandler( &hnd );
    parser.setLexicalHandler( &hnd );
    parser.setDeclHandler( &hnd );

    QString errorStatus;
    if ( incrementalSteps.isNull() ) {
	source.reset();
	if ( parser.parse( source ) ) {
	    errorStatus = "OK";
	} else {
	    errorStatus = "Error";
	}
    } else {

	file.reset();
	bool first = TRUE;
	QByteArray rawData;
	QXmlInputSource source;
	QStringList steps = QStringList::split( ",", incrementalSteps );

	QStringList::Iterator it = steps.begin();
	while ( TRUE ) {
	    if ( it != steps.end() ) {
		int size = (*it).toInt();
		rawData.resize( size );
		size = file.readBlock( rawData.data(), size );
		rawData.resize( size );
		source.setData( rawData );
		QString tmp = source.data();
		src->append( "\n---------------------------------------------------------\n" );
		src->append( tmp );
		source.setData( tmp );
	    }
	    // last parse is on empty input, so that we really get the end of
	    // the document
	    if ( first ) {
qDebug( "*** parse" );
		first = FALSE;
		if ( parser.parse( &source, TRUE ) ) {
		    errorStatus = "OK";
		} else {
		    errorStatus = "Error";
		    break;
		}
	    } else {
qDebug( "*** parse continue" );
		if ( parser.parseContinue() ) {
		    errorStatus = "OK";
		} else {
		    errorStatus = "Error";
		    break;
		}
	    }
	    if ( it == steps.end() ) {
		break;
	    }
	    ++it;
	}
	src->append( "\n---------------------------------------------------------\n" );
    }

    new XMLFileItem( lview, filename, errorStatus, time,
	    src, protocol, err, tree );

    file.close();
}

void ControlCentral::show( QStringList* files )
{
    if ( files == 0 ) {
	QStringList xmlFiles( QFileDialog::getOpenFileNames() );
	for ( QStringList::Iterator it = xmlFiles.begin();
		it != xmlFiles.end(); ++it ) {
	    parse( (*it) );
	}
    } else {
	for ( QStringList::Iterator it = files->begin();
		it != files->end(); ++it ) {
	    parse( *it );
	}
    }
    QVBox::show();
}

void ControlCentral::showSource()
{
    XMLFileItem *fi = (XMLFileItem*)( lview->selectedItem() );
    if ( fi != 0 ) {
	fi->source->show();
    }
}

void ControlCentral::showParseProtocol()
{
    XMLFileItem *fi = (XMLFileItem*)( lview->selectedItem() );
    if ( fi != 0 ) {
	fi->parseProtocol->show();
    }
}

void ControlCentral::showErrorProtocol()
{
    XMLFileItem *fi = (XMLFileItem*)( lview->selectedItem() );
    if ( fi != 0 ) {
	fi->errorProtocol->show();
    }
}

void ControlCentral::showTree()
{
    XMLFileItem *fi = (XMLFileItem*)( lview->selectedItem() );
    if ( fi != 0 ) {
	fi->tree->show();
    }
}

void ControlCentral::incrementalParse()
{
    XMLFileItem *fi = (XMLFileItem*)( lview->selectedItem() );
    if ( fi != 0 ) {
	parse( fi->text( 0 ), incSteps->text() );
    }
}
