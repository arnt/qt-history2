#include "listviewitemdrag.h"
#include <qdatastream.h>

QDataStream & operator<< ( QDataStream & stream, const QListViewItem & item );
QDataStream & operator>> ( QDataStream & stream, QListViewItem & item );

ListViewItemDrag::ListViewItemDrag( ListViewItemList &items, QWidget * parent, const char * name )
    : QStoredDrag( "qt/listviewitem", parent, name )
{
    // ### FIX!
    QByteArray data( sizeof( Q_INT32 ) + sizeof( QListViewItem ) * items.count() );
    QDataStream stream( data, IO_WriteOnly );

    stream << (Q_INT32)items.count();

    QListViewItem *i = items.first();
    while ( i ) {
        stream << *i;
	i = items.next();
    }

    setEncodedData( data );
}

bool ListViewItemDrag::canDecode( QDragMoveEvent * event )
{
    return event->provides( "qt/listviewitem" );
}

bool ListViewItemDrag::decode( QDropEvent * event, QListView *parent, QListViewItem *below )
{
    QByteArray data = event->encodedData( "qt/listviewitem" );

    // Which current??
    // src->setCurrentItem( item );

    if ( data.size() ) {
	event->accept();
	QDataStream stream( data, IO_ReadOnly );
    
	int count = 0;
	stream >> (Q_INT32) count;

	for( int i = 0; i < count; i++ ) {
	    below = new QListViewItem( parent, below );
	    stream >> *below;
	}
	return TRUE;
    }
    return FALSE;
}


QDataStream & operator<< ( QDataStream & stream, const QListViewItem & item )
{
    int columns = item.listView()->columns();
    stream << (Q_INT32) columns;
 
    Q_UINT8 b = 0;

    int i;
    for ( i = 0; i < columns; i++ ) {
	b = (Q_UINT8) ( item.text( i ) != QString::null ); // does column i have a string ?
	stream << b;
	if ( b ) {
	    stream << item.text( i );
	}
    }
    
    for ( i = 0; i < columns; i++ ) {
	b = (Q_UINT8) ( !!item.pixmap( i ) ); // does column i have a pixmap ?
	stream << b;
	if ( b ) {
	    stream << ( *item.pixmap( i ) );
	}
    }

    stream << (Q_UINT8) item.isOpen();
    stream << (Q_UINT8) item.isSelectable();
    stream << (Q_UINT8) item.isExpandable();
    stream << (Q_UINT8) item.dragEnabled();
    stream << (Q_UINT8) item.dropEnabled();
    stream << (Q_UINT8) item.isVisible();

    for ( i = 0; i < columns; i++ ) {
	stream << (Q_UINT8) item.renameEnabled( i );
    }

    stream << (Q_UINT8) item.multiLinesEnabled();
    stream << item.childCount();

    if ( item.childCount() > 0 ) {
	QListViewItem * child = item.firstChild();
	while ( child ) {
	    stream << ( *child ); // recursive call
	    child = child->nextSibling();
	}
    }

    return stream;
}
    
QDataStream & operator>> ( QDataStream & stream, QListViewItem & item )
{
    int columns;
    stream >> (Q_INT32) columns;

    Q_UINT8 b = 0;

    QString text;
    int i;
    for ( i = 0; i < columns; i++ ) {
	stream >> b;
	if ( b ) { // column i has string ?
	    stream >> text;
	    item.setText( i, text );
	}
    }

    QPixmap pixmap;
    for ( i = 0; i < columns; i++ ) {
	stream >> b; // column i has pixmap ?
	if ( b ) {
	    stream >> pixmap;
	    item.setPixmap( i, pixmap );
	}
    }

    stream >> b;
    item.setOpen( b );

    stream >> b;
    item.setSelectable( b );

    stream >> b;
    item.setExpandable( b );

    stream >> b;
    item.setDragEnabled( b );

    stream >> b;
    item.setDropEnabled( b );

    stream >> b;
    item.setVisible( b );

    for ( i = 0; i < columns; i++ ) {
	stream >> b;
	item.setRenameEnabled( i, b );
    }

    stream >> b;
    item.setMultiLinesEnabled( b );

    int childCount;
    stream >> childCount;

    QListViewItem *child = 0;
    QListViewItem *prevchild = 0;
    for ( i = 0; i < childCount; i++ ) {
	child = new QListViewItem( &item, prevchild );
	stream >> ( *child );
	item.insertItem( child );
	prevchild = child;
    }

    return stream;
}
