#include <qtextview.h>
#include <qlistview.h>
#include <qlabel.h>
#include <qhbox.h>

class XMLFileItem : public QListViewItem
{
public:
    XMLFileItem( QListView*, const QString&, const QString&, const QString&,
	    QTextView*, QListView*, QLabel*, QHBox* );
    virtual ~XMLFileItem();

    QTextView *source;
    QListView *parseProtocol;
    QLabel *errorProtocol;
    QHBox *tree;
};
