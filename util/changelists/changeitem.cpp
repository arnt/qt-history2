#include "changeitem.h"

ChangeItem::ChangeItem( QListView *parent, int changeNr ) :
    QListViewItem( parent, QString::number(changeNr) )
{
}

ChangeItem::~ChangeItem()
{
}

QString ChangeItem::key( int column, bool ascending ) const
{
    if ( column == 0 ) { 
	QString tmpString;
	tmpString.sprintf( "%08d", text(0).toInt() ); 
	return tmpString; 
    }
    return QListViewItem::key( column, ascending );
}
