#include "listviewitemdrag.h"
#include <qdatastream.h>

QDataStream & operator<< ( QDataStream & stream, const QListViewItem & item )
{
    int columns = item.listView()->columns();    
    stream << columns;

    Q_UINT8 b = 0;

    for ( int i = 0; i < columns; i++ ) {
	b = (Q_UINT8) ( item.text( i ) != QString::null ); // column i has string ?
	stream << b;
	if ( b ) {
	    stream << item.text( i );
	}
    }
    
    for ( int i = 0; i < columns; i++ ) {
	b = (Q_UINT8) ( !!item.pixmap( i ) ); // column i has pixmap ?
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

    for ( int i = 0; i < columns; i++ ) {
	stream << (Q_UINT8) item.renameEnabled( i );
    }

    //FIXME: RTTI

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
    stream >> columns;

    Q_UINT8 b = 0;

    QString text;
    for ( int i = 0; i < columns; i++ ) {
	stream >> b;
	if ( b ) { // column i has string ?
	    stream >> text;
	    item.setText( i, text );
	}
    }

    QPixmap pixmap;
    for ( int i = 0; i < columns; i++ ) {
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

    for ( int i = 0; i < columns; i++ ) {
	stream >> b;
	item.setRenameEnabled( i, b );
    }

    //FIXME: RTTI

    stream >> b;
    item.setMultiLinesEnabled( b );

    int childCount;
    stream >> childCount;

    QListViewItem * child;
    for ( int i = 0; i < childCount; i++ ) {
	child = new QListViewItem( &item );
	stream >> ( *child );
	item.insertItem( child );
    }

    return stream;
}

ListViewItemDrag::ListViewItemDrag( QListViewItem * item, QWidget * parent, const char * name )
    : QStoredDrag( "qt/listviewitem", parent, name ), QListViewItem( item )
{
    QByteArray data( sizeof( QListViewItem ) );
    QDataStream stream( data, IO_WriteOnly );
    stream << (*item);
    setEncodedData( data );
}

bool ListViewItemDrag::canDecode( QDragMoveEvent * event )
{
    return event->provides( "qt/listviewitem" );
}

bool ListViewItemDrag::decode( QDropEvent * event, QListViewItem * item )
{
    QByteArray data = event->encodedData( "qt/listviewitem" );

    if ( data.size() ) {
	event->accept();
	QDataStream stream( data, IO_ReadOnly );
	stream >> (*item);
	return TRUE;
    }
    return FALSE;
}
