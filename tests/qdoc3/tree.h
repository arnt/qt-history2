/*
  tree.h
*/

#ifndef TREE_H
#define TREE_H

#include <qmap.h>
#include <qstringlist.h>
#include <qvaluelist.h>

#include "node.h"

struct BaseClass
{
    Node::Access access;
    QStringList path;
    QString templateArgs;

    BaseClass()
	: access( Node::Public ) { }
    BaseClass( Node::Access access0, const QStringList& path0,
	      const QString& templateArgs0 )
	: access( access0 ), path( path0 ), templateArgs( templateArgs0 ) { }
};

class Tree
{
public:
    Tree();

    NamespaceNode *root() { return &roo; }
    Node *findNode( const QStringList& path );
    Node *findNode( const QStringList& path, Node::Type type );
    FunctionNode *findFunctionNode( const QStringList& path,
				    const FunctionNode *clone );
    void addBaseClass( ClassNode *subclass, Node::Access access,
		       const QStringList& basePath,
		       const QString& baseTemplateArgs );
    void freeze();

    const NamespaceNode *root() const { return &roo; }

private:
    NamespaceNode roo;
    QMap<ClassNode *, QValueList<BaseClass> > bas;
};

#endif
