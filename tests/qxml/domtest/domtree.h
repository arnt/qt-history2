#ifndef DOMTREE_H
#define DOMTREE_H

#include <qlistview.h>
#include <qdom.h>

class DomTree : public QListView
{
    Q_OBJECT

public:
    DomTree( const QString fileName, QWidget *parent = 0, const char *name = 0 );
    ~DomTree();

private:
    QDomDocument domTree;
    void buildTree( QListViewItem *parentItem, const QDomNode &actNode );
};

#endif // DOMTREE_H
