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

typedef QMap<PropertyNode::FunctionRole, QString> RoleMap;
typedef QMap<PropertyNode *, RoleMap> PropertyMap;

class TreePrivate
{
public:
    QMap<ClassNode *, QList<InheritanceBound> > unresolvedInheritanceMap;
    PropertyMap unresolvedPropertyMap;
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
    while ( p != path.end() ) {
	if ( node == 0 || !node->isInnerNode() )
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

FunctionNode *Tree::findFunctionNode( const QStringList& path )
{
    QStringList parentPath = path;
    parentPath.remove( parentPath.fromLast() );
    Node *parent = findNode( parentPath );
    if ( parent == 0 || !parent->isInnerNode() ) {
	return 0;
    } else {
	return ((InnerNode *) parent)->findFunctionNode( path.last() );
    }
}

FunctionNode *Tree::findFunctionNode( const QStringList& parentPath,
				      const FunctionNode *clone )
{
    Node *parent = findNode( parentPath );
    if ( parent == 0 || !parent->isInnerNode() ) {
	return 0;
    } else {
	return ((InnerNode *)parent)->findFunctionNode( clone );
    }
}

void Tree::addBaseClass( ClassNode *subclass, Node::Access access,
			 const QStringList& basePath,
			 const QString& baseTemplateArgs )
{
    priv->unresolvedInheritanceMap[subclass].append(
	    InheritanceBound(access, basePath, baseTemplateArgs) );
}


void Tree::addPropertyFunction(PropertyNode *property, const QString &funcName,
			       PropertyNode::FunctionRole funcRole)
{
    priv->unresolvedPropertyMap[property].insert(funcRole, funcName);
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

void Tree::resolveProperties()
{
    PropertyMap::ConstIterator propEntry = priv->unresolvedPropertyMap.begin();
    while (propEntry != priv->unresolvedPropertyMap.end()) {
	PropertyNode *property = propEntry.key();
        InnerNode *parent = property->parent();
	QString getterName = (*propEntry)[PropertyNode::Getter];
	QString setterName = (*propEntry)[PropertyNode::Setter];
	QString resetterName = (*propEntry)[PropertyNode::Resetter];

	FunctionNode *getterFunc = parent->findFunctionNode(getterName);
        FunctionNode *setterFunc = parent->findFunctionNode(setterName); // ### not good enough
        FunctionNode *resetterFunc = parent->findFunctionNode(resetterName);

	if (getterFunc) {
	    property->setFunction(getterFunc, PropertyNode::Getter);
            getterFunc->setAssociatedProperty(property);
	}
	if (setterFunc) {
	    property->setFunction(setterFunc, PropertyNode::Setter);
            setterFunc->setAssociatedProperty(property);
	}
	if (resetterFunc) {
	    property->setFunction(resetterFunc, PropertyNode::Resetter);
            resetterFunc->setAssociatedProperty(property);
	}

	++propEntry;
    }
    priv->unresolvedPropertyMap.clear();
}

void Tree::resolveInheritance(int pass, ClassNode *classe)
{
    if ( pass == 0 ) {
	QList<InheritanceBound> bounds = priv->unresolvedInheritanceMap[classe];
	QList<InheritanceBound>::ConstIterator b = bounds.begin();
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
    QList<RelatedClass>::ConstIterator r = classe->baseClasses().begin();
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
