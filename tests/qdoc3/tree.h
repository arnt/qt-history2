/*
  tree.h
*/

#ifndef TREE_H
#define TREE_H

#include "node.h"

class QStringList;
class TreePrivate;

class Tree
{
public:
    Tree();
    ~Tree();

    Node *findNode( const QStringList& path );
    Node *findNode( const QStringList& path, Node::Type type );
    FunctionNode *findFunctionNode( const QStringList& path );
    FunctionNode *findFunctionNode( const QStringList& parentPath,
				    const FunctionNode *clone );
    void addBaseClass( ClassNode *subclass, Node::Access access,
		       const QStringList& basePath,
		       const QString& baseTemplateArgs );
    void addPropertyFunction(PropertyNode *property, const QString &funcName,
			     PropertyNode::FunctionRole funcRole);
    void resolveInheritance();
    void resolveProperties();

    const NamespaceNode *root() const { return &roo; }

    NamespaceNode *root() { return &roo; }

private:
    void resolveInheritance( int pass, ClassNode *classe );
    FunctionNode *findFunctionInBaseClasses( ClassNode *classe,
					     FunctionNode *clone );

    NamespaceNode roo;
    TreePrivate *priv;
};

#endif
