/*
  tree.cpp
*/

#include <qstringlist.h>

#include "messages.h"
#include "stringset.h"
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
    bas[subclass].append( BaseClass(access, basePath, baseTemplateArgs) );
}

void Tree::freeze()
{
    freezeBaseClasses();
    freezeInnerNode( root() );
}

void Tree::freezeBaseClasses()
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

void Tree::freezeInnerNode( InnerNode *node )
{
    NodeList childNodes = node->childNodes();
    NodeList::ConstIterator c;

    c = childNodes.begin();
    while ( c != childNodes.end() ) {
	if ( (*c)->isInnerNode() ) {
	    freezeInnerNode( (InnerNode *) *c );
	} else if ( (*c)->type() == Node::Function &&
		    node->type() == Node::Class ) {
	    FunctionNode *func = (FunctionNode *) *c;
	    ClassNode *classe = (ClassNode *) node;
	    FunctionNode *source = findReimplementedFrom( func, classe );
	    if ( source != 0 ) {
		if ( func->virtualness() == FunctionNode::NonVirtual )
		    func->setVirtualness( FunctionNode::ImpureVirtual );
		func->setReimplementedFrom( source );

		if ( !func->doc().isEmpty() && !func->isReimplementation() )
		    warning( 1, func->doc().location(),
			     "Missing '\\reimp' in documentation" );
	    } else {
		if ( func->isReimplementation() )
		    warning( 1, func->doc().location(),
			     "Function does not appear to be a"
			     " reimplementation" );
	    }
	} else if ( (*c)->type() == Node::Property &&
		     node->type() == Node::Class ) {
	    PropertyNode *property = (PropertyNode *) *c;

	    if ( !property->getter().isEmpty() )
		freezePropertyFunction( "getter", property,
					property->getter() );
	    if ( !property->setter().isEmpty() )
		freezePropertyFunction( "setter", property,
					property->setter() );
	    if ( !property->resetter().isEmpty() )
		freezePropertyFunction( "resetter", property,
					property->resetter() );
	}
	++c;
    }
}

void Tree::freezePropertyFunction( const QString& role, PropertyNode *property,
				   const QString& funcName )
{
    ClassNode *classe = (ClassNode *) property->parent();
    FunctionNode *func = classe->findFunctionNode( funcName ); // ### inherited
    if ( func == 0 ) {
	warning( 1, property->location(), "Cannot find %s '%s' in class '%s'",
		 role.latin1(), property->getter().latin1(),
		 classe->name().latin1() );
    } else {
	Location location;
	if ( property->doc().isEmpty() ) {
	    location = property->location();
	} else {
	    location = property->doc().location();
	}

	QString text;

	Doc doc = Doc::propertyFunctionDoc( property->doc(), role, "" );

	if ( func->doc().isEmpty() ) {
	    func->setDoc( doc );
	    func->setStatus( property->status() );
	} else {
	    warning( 1, func->doc().location(),
		     "Superfluous documentation for property %s '%s'",
		     role.latin1(), funcName.latin1() );
	}
    }
}

FunctionNode *Tree::findReimplementedFrom( FunctionNode *func,
					   ClassNode *classe )
{
    FunctionNode *source = 0;
    QValueList<RelatedClass>::ConstIterator b = classe->baseClasses().begin();
    while ( b != classe->baseClasses().end() ) {
	ClassNode *baseClass = (*b).node;
	source = baseClass->findFunctionNode( func );
	if ( source != 0 ) {
	    if ( source->virtualness() != FunctionNode::NonVirtual ||
		 findReimplementedFrom(source, baseClass) != 0 )
		return source;
	}

	source = findReimplementedFrom( func, baseClass );
	if ( source != 0 )
	    return source;
	++b;
    }
    return 0;
}
