#include "changeitem.h"

ChangeItem::ChangeItem( QListView *parent, int changeNr, const QString& date ) :
    QListViewItem( parent, QString::number(changeNr), date ),
    visited(false)
{
}

ChangeItem::ChangeItem( QListViewItem *parent, int changeNr, const QString& date ) :
    QListViewItem( parent, QString::number(changeNr), date ),
    visited(false)
{
}

ChangeItem::~ChangeItem()
{
}

int ChangeItem::compare( QListViewItem *i, int col, bool ascending ) const
{
    if ( col == 0 ) { 
	int myValue = text(0).toInt();
	int hisValue = i->text(0).toInt();
	if ( myValue == hisValue )
	    return 0;
	else if ( myValue > hisValue )
	    return 1;
	return -1;
    }
    return QListViewItem::compare( i, col, ascending );
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
