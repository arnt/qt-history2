#include "changeitem.h"

ChangeItem::ChangeItem( QListView *parent, int changeNr ) :
    QListViewItem( parent, QString::number(changeNr) ),
    visited(FALSE)
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

void ChangeItem::paintCell( QPainter *p, const QColorGroup &cg, int column, int width, int align )
{
    QColorGroup c = cg;
    if ( isVisited() ) {
	c.setColor( QColorGroup::Text, cg.linkVisited() );
    } else {
	c.setColor( QColorGroup::Text, cg.link() );
    }
    QListViewItem::paintCell( p, c, column, width, align );
}

void ChangeItem::setVisitedEnable( bool v )
{
    visited = v;
}

bool ChangeItem::isVisited() const
{
    return visited;
}
