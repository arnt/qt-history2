/*
  node.cpp
*/

#include "node.h"

Node::~Node()
{
    if (par)
	par->removeChild(this);
    if (rel)
	rel->removeRelated(this);
}

void Node::setDoc( const Doc& doc, bool replace )
{
    if (!d.isEmpty() && !replace) {
	doc.location().warning(tr("Overrides a previous doc"));
	d.location().warning(tr("(The previous doc is here)"));
    }
    d = doc;
}

Node::Node( Type type, InnerNode *parent, const QString& name )
    : typ(type), acc(Public), sta(Commendable), saf(UnspecifiedSafeness), par(parent), rel(0),
      nam(name)
{
    if (par)
	par->addChild(this);
}

void Node::setRelates(InnerNode *pseudoParent)
{
    if (rel)
	rel->removeRelated(this);
    rel = pseudoParent;
    pseudoParent->related.append(this);
}

Node::Status Node::inheritedStatus() const
{
    Status parentStatus = Commendable;
    if (par)
	parentStatus = inheritedStatus();
    return QMIN( sta, parentStatus );
}

Node::ThreadSafeness Node::threadSafeness() const
{
    if (par && saf == par->inheritedThreadSafeness())
	return UnspecifiedSafeness;
    return saf;
}

Node::ThreadSafeness Node::inheritedThreadSafeness() const
{
    if (par && saf == UnspecifiedSafeness)
	return par->inheritedThreadSafeness();
    return saf;
}

InnerNode::~InnerNode()
{
    deleteChildren();
}

Node *InnerNode::findNode( const QString& name )
{
    QMap<QString, Node *>::ConstIterator c = childMap.find( name );
    if ( c == childMap.end() ) {
	return 0;
    } else {
	return *c;
    }
}

Node *InnerNode::findNode( const QString& name, Type type )
{
    Node *node = findNode( name );
    if ( node->type() == type ) {
	return node;
    } else {
	return 0;
    }
}

FunctionNode *InnerNode::findFunctionNode( const QString& name )
{
    QMap<QString, Node *>::ConstIterator c = primaryFunctionMap.find( name );
    if ( c == primaryFunctionMap.end() ) {
	return 0;
    } else {
	return (FunctionNode *) *c;
    }
}

FunctionNode *InnerNode::findFunctionNode( const FunctionNode *clone )
{
    QMap<QString, Node *>::ConstIterator c =
	    primaryFunctionMap.find( clone->name() );
    if ( c != primaryFunctionMap.end() ) {
	if ( isSameSignature(clone, (FunctionNode *) *c) ) {
	    return (FunctionNode *) *c;
	} else if ( secondaryFunctionMap.contains(clone->name()) ) {
	    const NodeList& secs = secondaryFunctionMap[clone->name()];
	    NodeList::ConstIterator s = secs.begin();
	    while ( s != secs.end() ) {
		if ( isSameSignature(clone, (FunctionNode *) *s) )
		    return (FunctionNode *) *s;
		++s;
	    }
	}
    }
    return 0;
}

void InnerNode::setOverload( const FunctionNode *func, bool overlode )
{
    Node *node = (Node *) func;
    Node *&primary = primaryFunctionMap[func->name()];

    if ( secondaryFunctionMap.contains(func->name()) ) {
	NodeList& secs = secondaryFunctionMap[func->name()];
	if ( overlode ) {
	    if ( primary == node ) {
		primary = secs.first();
		secs.remove( secs.begin() );
		secs.prepend( node );
	    }
	} else {
	    if ( primary != node ) {
		secs.remove( node );
		secs.prepend( primary );
		primary = node;
	    }
	}
    }
}

void InnerNode::normalizeOverloads()
{
    QMap<QString, Node *>::ConstIterator p = primaryFunctionMap.begin();
    while ( p != primaryFunctionMap.end() ) {
	FunctionNode *primaryFunc = (FunctionNode *) *p;
	if ( primaryFunc->isOverload() )
	    primaryFunc->ove = FALSE;
	if ( secondaryFunctionMap.contains(primaryFunc->name()) ) {
	    NodeList& secs = secondaryFunctionMap[primaryFunc->name()];
	    NodeList::ConstIterator s = secs.begin();
	    while ( s != secs.end() ) {
		FunctionNode *secondaryFunc = (FunctionNode *) *s;
		if ( !secondaryFunc->isOverload() )
		    secondaryFunc->ove = TRUE;
		++s;
	    }
	}
	++p;
    }

    NodeList::ConstIterator c = childNodes().begin();
    while ( c != childNodes().end() ) {
	if ( (*c)->isInnerNode() )
	    ((InnerNode *) *c)->normalizeOverloads();
	++c;
    }
}

void InnerNode::deleteChildren()
{
    while (!children.isEmpty())
	delete children.first();
}

bool InnerNode::isInnerNode() const
{
    return TRUE;
}

const Node *InnerNode::findNode( const QString& name ) const
{
    InnerNode *that = (InnerNode *) this;
    return that->findNode( name );
}

const Node *InnerNode::findNode( const QString& name, Type type ) const
{
    InnerNode *that = (InnerNode *) this;
    return that->findNode( name, type );
}

const FunctionNode *InnerNode::findFunctionNode( const QString& name ) const
{
    InnerNode *that = (InnerNode *) this;
    return that->findFunctionNode( name );
}

const FunctionNode *InnerNode::findFunctionNode(
	const FunctionNode *clone ) const
{
    InnerNode *that = (InnerNode *) this;
    return that->findFunctionNode( clone );
}

int InnerNode::overloadNumber( const FunctionNode *func ) const
{
    Node *node = (Node *) func;
    if ( primaryFunctionMap[func->name()] == node ) {
	return 1;
    } else {
	return secondaryFunctionMap[func->name()].findIndex( node ) + 2;
    }
}

int InnerNode::numOverloads( const QString& funcName ) const
{
    if ( primaryFunctionMap.contains(funcName) ) {
	return secondaryFunctionMap[funcName].count() + 1;
    } else {
	return 0;
    }
}

InnerNode::InnerNode( Type type, InnerNode *parent, const QString& name )
    : Node( type, parent, name )
{
}

void InnerNode::addInclude( const QString& include )
{
    inc.append( include );
}

bool InnerNode::isSameSignature( const FunctionNode *f1,
				 const FunctionNode *f2 )
{
    if ( f1->parameters().count() != f2->parameters().count() )
	return FALSE;
    if ( f1->isConst() != f2->isConst() )
	return FALSE;

    QList<Parameter>::ConstIterator p1 = f1->parameters().begin();
    QList<Parameter>::ConstIterator p2 = f2->parameters().begin();
    while ( p2 != f2->parameters().end() ) {
	if ( (*p1).hasType() && (*p2).hasType() ) {
	    if ( (*p1).leftType() != (*p2).leftType() ||
		 (*p1).rightType() != (*p2).rightType() ||
		 (*p1).defaultValue() != (*p2).defaultValue() )
		return FALSE;
	}
	++p1;
	++p2;
    }
    return TRUE;
}

void InnerNode::addChild( Node *child )
{
    children.append( child );
    if ( child->type() == Function ) {
	FunctionNode *func = (FunctionNode *) child;
	if ( *primaryFunctionMap.insert(func->name(), func, FALSE) != func ) {
	    NodeList& secs = secondaryFunctionMap[func->name()];
	    secs.append( func );
	}
    } else {
	childMap.insert( child->name(), child );
    }
}

void InnerNode::removeChild( Node *child )
{
    children.remove( child );
    if ( child->type() == Function ) {
	QMap<QString, Node *>::Iterator prim =
		primaryFunctionMap.find( child->name() );
	NodeList& secs = secondaryFunctionMap[child->name()];
	if ( *prim == child ) {
	    if ( secs.isEmpty() ) {
		primaryFunctionMap.remove( child->name() );
	    } else {
		primaryFunctionMap.replace( child->name(), secs.first() );
		secs.remove( secs.begin() );
	    }
	} else {
	    secs.remove( child );
	}
    } else {
	QMap<QString, Node *>::Iterator ent = childMap.find( child->name() );
	if ( *ent == child )
	    childMap.remove( ent );
    }
}

void InnerNode::removeRelated(Node *pseudoChild)
{
    related.remove(pseudoChild);
}

bool LeafNode::isInnerNode() const
{
    return FALSE;
}

LeafNode::LeafNode( Type type, InnerNode *parent, const QString& name )
    : Node( type, parent, name )
{
}

NamespaceNode::NamespaceNode( InnerNode *parent, const QString& name )
    : InnerNode( Namespace, parent, name )
{
}

ClassNode::ClassNode( InnerNode *parent, const QString& name )
    : InnerNode( Class, parent, name )
{
}

void ClassNode::addBaseClass( Access access, ClassNode *node,
			      const QString& templateArgs )
{
    bas.append( RelatedClass(access, node, templateArgs) );
    node->der.append( RelatedClass(access, this) );
}

FakeNode::FakeNode( InnerNode *parent, const QString& name, SubType subType )
    : InnerNode( Fake, parent, name ), sub( subType )
{
}

EnumNode::EnumNode( InnerNode *parent, const QString& name )
    : LeafNode( Enum, parent, name )
{
}

void EnumNode::addItem( const EnumItem& item )
{
    itms.append( item );
}

Node::Access EnumNode::itemAccess(const QString &name) const
{
    if (doc().omitEnumItemNames().contains(name)) {
	return Private;
    } else {
	return Public;
    }
}

TypedefNode::TypedefNode( InnerNode *parent, const QString& name )
    : LeafNode( Typedef, parent, name )
{
}

Parameter::Parameter( const QString& leftType, const QString& rightType,
		      const QString& name, const QString& defaultValue )
    : lef( leftType ), rig( rightType ), nam( name ), def( defaultValue )
{
}

Parameter::Parameter( const Parameter& p )
    : lef( p.lef ), rig( p.rig ), nam( p.nam ), def( p.def )
{
}

Parameter& Parameter::operator=( const Parameter& p )
{
    lef = p.lef;
    rig = p.rig;
    nam = p.nam;
    def = p.def;
    return *this;
}

FunctionNode::FunctionNode( InnerNode *parent, const QString& name )
    : LeafNode( Function, parent, name ), met( Plain ), vir( NonVirtual ),
      con( FALSE ), sta( FALSE ), ove( FALSE ), rf( 0 ), ap( 0 )
{
}

void FunctionNode::setOverload( bool overlode )
{
    parent()->setOverload( this, overlode );
    ove = overlode;
}

void FunctionNode::addParameter( const Parameter& parameter )
{
    params.append( parameter );
}

void FunctionNode::borrowParameterNames( const FunctionNode *source )
{
    QList<Parameter>::Iterator t = params.begin();
    QList<Parameter>::ConstIterator s = source->params.begin();
    while ( s != source->params.end() && t != params.end() ) {
	if ( !(*s).name().isEmpty() )
	    (*t).setName( (*s).name() );
	++s;
	++t;
    }
}

void FunctionNode::setReimplementedFrom( FunctionNode *from )
{
    rf = from;
    from->rb.append( this );
}

void FunctionNode::setAssociatedProperty( PropertyNode *property )
{
    ap = property;
}

int FunctionNode::overloadNumber() const
{
    return parent()->overloadNumber( this );
}

int FunctionNode::numOverloads() const
{
    return parent()->numOverloads( name() );
}

QStringList FunctionNode::parameterNames() const
{
    QStringList names;
    QList<Parameter>::ConstIterator p = parameters().begin();
    while ( p != parameters().end() ) {
	names << (*p).name();
	++p;
    }
    return names;
}

PropertyNode::PropertyNode( InnerNode *parent, const QString& name )
    : LeafNode(Property, parent, name), sto(Trool_Default), des(Trool_Default)
{
    for (int i = 0; i < (int)NumFunctionRoles; ++i)
	funcs[i] = 0;
}

PropertyNode::Trool PropertyNode::toTrool( bool boolean )
{
    return boolean ? Trool_True : Trool_False;
}

bool PropertyNode::fromTrool( Trool troolean, bool defaultValue )
{
    switch ( troolean ) {
    case Trool_True:
	return TRUE;
    case Trool_False:
	return FALSE;
    default:
	return defaultValue;
    }
}
