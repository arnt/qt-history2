#include <qapplication.h>
#include <qhbox.h>
#include <qxml.h>

#include "playdisplay.h"
#include "playparser.h"
#include "itemtextview.h"


PlayDisplay::PlayDisplay() : QVBox()
{
    QHBox* tree = new QHBox( this );

    lview = new QListView( tree );
    lview->addColumn(  "Table of Content" );
    lview->setSorting( -1 );

    ItemTextView *content = new ItemTextView( tree );
    content->setIndex( 1 );
    content->setTextFormat( RichText );

    tree->setStretchFactor( content, 3 );

    quit = new QPushButton( "Quit", this );

    connect( quit, SIGNAL(clicked()), qApp, SLOT(quit()) );
    connect( lview, SIGNAL(selectionChanged(QListViewItem*)),
	    content, SLOT(change(QListViewItem*)) );
}

PlayDisplay::~PlayDisplay()
{
}

QSize PlayDisplay::sizeHint() const
{
    return QSize( 600, 600 );
}

void PlayDisplay::show( const QString& filename )
{
    // create handler
    QString err;
    PlayParser hnd( lview, &err );

    // create input source
    QFile file( filename );
    if ( !file.open(IO_ReadOnly) ) {
	return;
    }
    QTextStream ts( &file );
    QXmlInputSource source( ts );

    // create reader
    QXmlSimpleReader parser;
    parser.setContentHandler( &hnd );

    // parse string
    bool ok = parser.parse( source );

    // close
    file.close();

    // test if parsing was ok
    if ( !ok ) {
	// TODO: print error message
	return;
    }

    QVBox::show();
}
