#include "xmlfileitem.h"

XMLFileItem::XMLFileItem( QListView *lv,
	const QString& n1, const QString& n2, const QString& n3,
	QTextView *s, QTextView *ts, DomTree *t
	) : QListViewItem( lv, n1, n2, n3 )
{
    source = s;
    toString = ts;
    tree = t;
}

XMLFileItem::~XMLFileItem()
{
    delete toString;
}

void XMLFileItem::showToString()
{
    delete toString;
    QTextView* toString = new QTextView();
    toString->setTextFormat( PlainText );
    toString->setText( tree->domDocument()->toString() );
    toString->setCaption( "To String for " + text(1) );
    toString->show();
}
