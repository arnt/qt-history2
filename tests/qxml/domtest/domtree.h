#include <qhbox.h>
#include <qlistview.h>
#include <qtextview.h>
#include <qdom.h>

#ifndef DOMTREE_H
#define DOMTREE_H

class DomTree : public QHBox
{
    Q_OBJECT

public:
    DomTree( const QString fileName, QWidget *parent = 0, const char *name = 0 );
    ~DomTree();

private slots:
    void selectionChanged( QListViewItem *it );

private:
    QDomDocument domTree;
    QListView *tree;
    QTextView *text;

    void buildTree( QListViewItem *parentItem, const QDomNode &actNode, const QDomNamedNodeMap &attribs );
};


class DomTreeItem : public QListViewItem
{
public:
    DomTreeItem( const QDomNode &node, QListView *parent, QListViewItem *after ) ;
    DomTreeItem( const QDomNode &node, QListViewItem *parent, QListViewItem *after ) ;
    ~DomTreeItem();

    QString contentString();

private:
    QDomNode _node;

    void init();
};

#endif // DOMTREE_H
