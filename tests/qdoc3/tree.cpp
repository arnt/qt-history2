/*
  tree.cpp
*/

#include "node.h"
#include "tree.h"

struct InheritanceBound
{
    Node::Access access;
    QStringList basePath;
    QString baseTemplateArgs;

    InheritanceBound()
	: access( Node::Public ) { }
    InheritanceBound( Node::Access access0, const QStringList& basePath0,
		      const QString& baseTemplateArgs0 )
	: access( access0 ), basePath( basePath0 ),
	  baseTemplateArgs( baseTemplateArgs0 ) { }
};

class TreePrivate
{
public:
    QMap<ClassNode *, QValueList<InheritanceBound> > unresolvedInheritanceMap;
};

Tree::Tree()
    : roo( 0, "" )
{
    priv = new TreePrivate;
}

Tree::~Tree()
{
    delete priv;
}

Node *Tree::findNode( const QStringList& path )
{
    Node *node = root();

    QStringList::ConstIterator p = path.begin();
    while ( p != path.end() && node != 0 ) {
	if ( !node->isInnerNode() )
	    return 0;
	node = ((InnerNode *) node)->findNode( *p );
	++p;
    }
    return node;
}

Node *Tree::findNode( const QStringList& path, Node::Type type )
{
    Node *node = findNode( path );
    if ( node != 0 && node->type() == type ) {
	return node;
    } else {
	return 0;
    }
}

FunctionNode *Tree::findFunctionNode( const QStringList& path,
				      const FunctionNode *synopsis )
{
    Node *parent = findNode( path );
    if ( parent == 0 || !parent->isInnerNode() ) {
	return 0;
    } else {
	return ((InnerNode *) parent)->findFunctionNode( synopsis );
    }
}

void Tree::addBaseClass( ClassNode *subclass, Node::Access access,
			 const QStringList& basePath,
			 const QString& baseTemplateArgs )
{
    priv->unresolvedInheritanceMap[subclass].append(
	    InheritanceBound(access, basePath, baseTemplateArgs) );
}

void Tree::resolveInheritance()
{
    NodeList::ConstIterator c = root()->childNodes().begin();
    while ( c != root()->childNodes().end() ) {
	if ( (*c)->type() == Node::Class )
	    resolveInheritance( (ClassNode *) *c );
	++c;
    }
    priv->unresolvedInheritanceMap.clear();
}

void Tree::resolveInheritance( ClassNode *classe )
{
    QValueList<InheritanceBound> bounds =
	    priv->unresolvedInheritanceMap[classe];
    QValueList<InheritanceBound>::ConstIterator b = bounds.begin();
    while ( b != bounds.end() ) {
	ClassNode *baseClass =
		(ClassNode *) findNode( (*b).basePath, Node::Class );
	if ( baseClass != 0 )
	    classe->addBaseClass( (*b).access, baseClass,
				  (*b).baseTemplateArgs );
	++b;
    }
}
