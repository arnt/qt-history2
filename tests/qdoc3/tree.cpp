/*
  tree.cpp
*/

#include "node.h"
#include "tree.h"

struct InheritanceBound
{
    Node::Access access;
    ClassNode *derivedClass;
    QString baseClassTemplateArgs;

    InheritanceBound()
	: access( Node::Public ) { }
    InheritanceBound( Node::Access access0, ClassNode *derivedClass0,
		      const QString& baseClassTemplateArgs0 )
	: access( access0 ), derivedClass( derivedClass0 ),
	  baseClassTemplateArgs( baseClassTemplateArgs0 ) { }
};

class TreePrivate
{
public:
    // QMultiMap<baseClassPath, InheritanceBound>
    QMap<QStringList, QValueList<InheritanceBound> > unresolvedInheritanceMap;
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
#if 0
    bas[subclass].append( BaseClass(access, basePath, baseTemplateArgs) );
#endif
}
