#include "xmlfileitem.h"
#include <qfiledialog.h>
#include <qtextstream.h>

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
    toString->setCaption( "To String for " + text(0) );
    toString->show();
}

void XMLFileItem::save()
{
    QFile file( QFileDialog::getSaveFileName() );
    if ( file.open( IO_WriteOnly | IO_Truncate ) ) {
	QTextStream ts( &file );
	tree->domDocument()->save( ts, 2 );
	file.close();
    }
}
