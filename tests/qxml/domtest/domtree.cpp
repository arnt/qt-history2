#include "domtree.h"
#include <qfile.h>
#include <qmessagebox.h>

DomTree::DomTree( const QString fileName, QWidget *parent, const char *name )
    : QListView( parent, name )
{
    // div. configuration of the list view
    addColumn( "DOM Nodes" );
    setRootIsDecorated( TRUE );
    setSorting( -1 );

    // read the XML file and create DOM tree
    QFile file( fileName );
    if ( !file.open( IO_ReadOnly ) ) {
#if 0
	QMessageBox::critical( 0,
		tr( "Critical Error" ),
		tr( "Cannot open file %1" ).arg( fileName ) );
#endif
	return;
    }
    if ( !domTree.setContent( &file ) ) {
#if 0
	QMessageBox::critical( 0,
		tr( "Critical Error" ),
		tr( "Parsing error for file %1" ).arg( fileName ) );
#endif
	file.close();
	return;
    }
    file.close();
    buildTree( 0, domTree.documentElement() );
}

DomTree::~DomTree()
{
}

void DomTree::buildTree( QListViewItem *parentItem, const QDomNode &actNode )
{
    QListViewItem *thisItem = 0;
    QDomNode node = actNode;
    while ( !node.isNull() ) {
	if ( parentItem == 0 )
	    thisItem = new QListViewItem( this, thisItem );
	else
	    thisItem = new QListViewItem( parentItem, thisItem );
	thisItem->setText( 0, node.nodeName() );

	buildTree( thisItem, node.firstChild() );
	node = node.nextSibling();
    }
}
