#include "xmlfileitem.h"

XMLFileItem::XMLFileItem( QListView *lv,
	const QString& n1, const QString& n2, const QString& n3,
	QTextView *s, DomTree *t
	) : QListViewItem( lv, n1, n2, n3 )
{
    source = s;
    tree = t;
}

XMLFileItem::~XMLFileItem()
{
}
