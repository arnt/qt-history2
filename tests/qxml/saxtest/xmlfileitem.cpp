#include "xmlfileitem.h"

XMLFileItem::XMLFileItem( QListView *lv,
	const QString& n1, const QString& n2, const QString& n3,
	QTextView *s, QListView *p, QLabel *err, QHBox *t
	) : QListViewItem( lv, n1, n2, n3 )
{
    source = s;
    parseProtocol = p;
    errorProtocol = err;
    tree = t;
}

XMLFileItem::~XMLFileItem()
{
}
