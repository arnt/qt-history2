#include <qapplication.h>
#include <qfiledialog.h>
#include <qhbox.h>
#include <qdatetime.h>
#include <qtextview.h>
#include <qtextstream.h>
#include <qlabel.h>
#include <qdom.h>

#include "controlcentral.h"
#include "xmlfileitem.h"
#include "domtree.h"

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
    QPushButton *toString = new QPushButton( "To String", hbox );
    QPushButton *save = new QPushButton( "Save", hbox );
#if 0
    QPushButton *errorProt = new QPushButton( "Error Protocol", hbox );
#endif
    QPushButton *tree = new QPushButton( "Tree", hbox );

    quit = new QPushButton( "Quit", this );

    connect( source, SIGNAL(clicked()), this, SLOT(showSource()) );
    connect( toString, SIGNAL(clicked()), this, SLOT(showToString()) );
    connect( save, SIGNAL(clicked()), this, SLOT(saveToFile()) );
#if 0
    connect( errorProt, SIGNAL(clicked()), this, SLOT(showErrorProtocol()) );
#endif
    connect( tree, SIGNAL(clicked()), this, SLOT(showTree()) );
    connect( quit, SIGNAL(clicked()), qApp, SLOT(quit()) );
}

ControlCentral::~ControlCentral()
{
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

    QDomDocument doc;
    QTime t;
    t.start();
    for ( int i=0; i<10; i++ ) {
	doc.setContent( &file, TRUE );
	file.reset();
    }
    double ms = ((double)t.elapsed()) / 10;
    QString time;
    time.setNum( ms );
    time += " ms";

    file.reset();
    QTextStream ts( &file );
    QTextView* src = new QTextView();
    src->setTextFormat( PlainText );
    src->setText( ts.read() );
    src->setCaption( "Source for " + filename );

#if 0
    QLabel *err = new QLabel( "", 0 );
    err->setCaption( "Error protocol for " + filename );
#endif

    DomTree* tree = new DomTree( filename );
    tree->setCaption( "Tree for " + filename );

    file.reset();
    QString errorStatus;
    if ( doc.setContent( &file, TRUE ) ) {
	errorStatus = "Ok";
    } else {
	errorStatus = "Error";
    }

    new XMLFileItem( lview, filename, errorStatus, time,
	    src, 0, tree );

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

void ControlCentral::showToString()
{
    XMLFileItem *fi = (XMLFileItem*)( lview->selectedItem() );
    if ( fi != 0 ) {
	fi->showToString();
    }
}

void ControlCentral::saveToFile()
{
    XMLFileItem *fi = (XMLFileItem*)( lview->selectedItem() );
    if ( fi != 0 ) {
	fi->save();
    }
}

void ControlCentral::showTree()
{
    XMLFileItem *fi = (XMLFileItem*)( lview->selectedItem() );
    if ( fi != 0 ) {
	fi->tree->show();
    }
}
