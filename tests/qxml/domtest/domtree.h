#include <qvbox.h>
#include <qlistview.h>
#include <qtextview.h>
#include <qdom.h>

#ifndef DOMTREE_H
#define DOMTREE_H

class DomTree : public QVBox
{
    Q_OBJECT

public:
    DomTree( const QString &fileName, QWidget *parent = 0, const char *name = 0 );
    ~DomTree();

    QDomDocument *domDocument() { return domTree; }

private slots:
    void selectionChanged( QListViewItem *it );
    void withNSProc();
    void withoutNSProc();

    void createElement();
    void createDocumentFragment();
    void createTextNode();
    void createComment();
    void createCDATASection();
    void createProcessingInstruction();
    void createAttribute();
    void createEntityReference();
    void createElementNS();
    void createAttributeNS();

private:
    QString filename;
    QDomDocument *domTree;
    QListView *tree;
    QTextView *text;

    void setContent( const QString &fileName, bool processNS );
    void buildTree( bool namespaces, QListViewItem *parentItem, const QDomNode &actNode, const QDomNamedNodeMap &attribs );
};


class DomTreeItem : public QListViewItem
{
public:
    DomTreeItem( bool useNS, const QDomNode &node, QListView *parent, QListViewItem *after ) ;
    DomTreeItem( bool useNS, const QDomNode &node, QListViewItem *parent, QListViewItem *after ) ;
    ~DomTreeItem();

    QString contentString();

private:
    QDomNode _node;

    void init();
    bool namespaces;
};

#endif // DOMTREE_H
