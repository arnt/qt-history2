/*
  tree.cpp
*/

#include <qstringlist.h>

#include "tree.h"

Tree::Tree()
    : roo( 0, "" )
{
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
				      const FunctionNode *synopsys )
{
    Node *parent = findNode( path );
    if ( parent == 0 || !parent->isInnerNode() ) {
	return 0;
    } else {
	return ((InnerNode *) parent)->findFunctionNode( synopsys );
    }
}

void Tree::addBaseClass( ClassNode *subclass, Node::Access access,
			 const QStringList& basePath,
			 const QString& baseTemplateArgs )
{
    bas[subclass].append( BaseClass(access, basePath, baseTemplateArgs) );
}

void Tree::freeze()
{
    QMap<ClassNode *, QValueList<BaseClass> >::ConstIterator b;
    QValueList<BaseClass>::ConstIterator c;

    b = bas.begin();
    while ( b != bas.end() ) {
	c = (*b).begin();
	while ( c != (*b).end() ) {
	    ClassNode *node = (ClassNode *) findNode( (*c).path, Node::Class );
	    if ( node != 0 )
		b.key()->addBaseClass( (*c).access, node, (*c).templateArgs );
	    ++c;
	}
	++b;
    }
}
