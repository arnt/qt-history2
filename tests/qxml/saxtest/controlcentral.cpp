#include <qapplication.h>
#include <qfiledialog.h>
#include <qhbox.h>
#include <qdatetime.h>
#include <qxml.h>

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

    quit = new QPushButton( "Quit", this );

    connect( source, SIGNAL(clicked()), this, SLOT(showSource()) );
    connect( parseProt, SIGNAL(clicked()), this, SLOT(showParseProtocol()) );
    connect( errorProt, SIGNAL(clicked()), this, SLOT(showErrorProtocol()) );
    connect( tree, SIGNAL(clicked()), this, SLOT(showTree()) );
    connect( quit, SIGNAL(clicked()), qApp, SLOT(quit()) );

    parseProtocolFile = new QFile( "parseProtocol" );
    if ( !parseProtocolFile->open( IO_WriteOnly | IO_Truncate ) ) {
	parseProtocolFile = 0;
	parseProtocolTS = 0;
    } else {
	parseProtocolTS = new QTextStream( parseProtocolFile );
    }
}

ControlCentral::~ControlCentral()
{
    if ( parseProtocolFile != 0 ) {
	parseProtocolFile->close();
	delete parseProtocolFile;
	delete parseProtocolTS;
    }
}

QSize ControlCentral::sizeHint() const
{
    return QSize( 500, 250 );
}

void ControlCentral::parse( const QString& filename )
{
    QFile file( filename );
    if ( !file.open(IO_ReadOnly) ) {
	return;
    }

    *parseProtocolTS << endl << "******** "
	<< filename << " ********" << endl;

    QXmlSimpleReader parser;
    QXmlInputSource source( &file );

    parser.setFeature( "http://xml.org/sax/features/namespaces", TRUE );
    parser.setFeature( "http://xml.org/sax/features/namespace-prefixes", TRUE );
    parser.setFeature( "http://trolltech.com/xml/features/report-whitespace-only-CharData", FALSE );
		 
    QTime t;
    t.start();
    for ( int i=0; i<10; i++ ) {
	parser.parse( source );
    }
    double ms = ((double)t.elapsed()) / 10;
    QString time;
    time.setNum( ms );
    time += " ms";

    file.reset();
    QTextView* src = new QTextView();
    src->setTextFormat( PlainText );
    src->setText( source.data() );
    src->setCaption( "Source for " + filename );

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

    XMLParser hnd( protocol, err, listTree, parseProtocolTS );
    parser.setEntityResolver( &hnd );
    parser.setDTDHandler( &hnd );
    parser.setContentHandler( &hnd );
    parser.setErrorHandler( &hnd );
    parser.setLexicalHandler( &hnd );
    parser.setDeclHandler( &hnd );

    QString errorStatus;
    if ( parser.parse( source ) ) {
	errorStatus = "Ok";
    } else {
	errorStatus = "Error";
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
