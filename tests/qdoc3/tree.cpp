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
    for ( int pass = 0; pass < 2; pass++ ) {
	NodeList::ConstIterator c = root()->childNodes().begin();
	while ( c != root()->childNodes().end() ) {
	    if ( (*c)->type() == Node::Class )
		resolveInheritance( pass, (ClassNode *) *c );
	    ++c;
	}
	priv->unresolvedInheritanceMap.clear();
    }
}

void Tree::resolveInheritance( int pass, ClassNode *classe )
{
    if ( pass == 0 ) {
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
    } else {
	NodeList::ConstIterator c = classe->childNodes().begin();
	while ( c != classe->childNodes().end() ) {
	    if ( (*c)->type() == Node::Function ) {
		FunctionNode *func = (FunctionNode *) *c;
		FunctionNode *from = findFunctionInBaseClasses( classe, func );
		if ( from != 0 ) {
		    if ( func->virtualness() == FunctionNode::NonVirtual )
			func->setVirtualness( FunctionNode::ImpureVirtual );
		    func->setReimplementedFrom( from );
		}
	    }
	    ++c;
	}
    }
}

FunctionNode *Tree::findFunctionInBaseClasses( ClassNode *classe,
					       FunctionNode *clone )
{
    QValueList<RelatedClass>::ConstIterator r = classe->baseClasses().begin();
    while ( r != classe->baseClasses().end() ) {
	FunctionNode *func;
	if ( ((func = findFunctionInBaseClasses((*r).node, clone)) != 0 ||
	      (func = (*r).node->findFunctionNode(clone)) != 0) &&
	     func->virtualness() != FunctionNode::NonVirtual )
	    return func;
 	++r;
    }
    return 0;
}
