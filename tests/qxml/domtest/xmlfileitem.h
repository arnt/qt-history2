#include <qtextview.h>
#include <qlistview.h>
#include <qlabel.h>
#include <qhbox.h>

#include "domtree.h"

class XMLFileItem : public QListViewItem
{
public:
    XMLFileItem( QListView*, const QString&, const QString&, const QString&,
	    QTextView*, QTextView*, DomTree* );
    virtual ~XMLFileItem();

    void showToString();

public:
    QTextView *source;
    QTextView *toString;
    DomTree *tree;
};
