/*
  node.cpp
*/

#include "messages.h"
#include "node.h"

Node::~Node()
{
    if ( par != 0 )
	par->removeChild( this );
}

void Node::setDoc( const Doc& doc )
{
    if ( !d.isEmpty() ) {
	warning( 1, doc.location(), "Overrides a previous doc" );
	warning( 1, d.location(), "(the previous doc is here)" );
    }
    d = doc;
}

Node::Node( Type type, InnerNode *parent, const QString& name )
    : typ( type ), par( parent ), nam( name ), acc( Public ), sta( Approved )
{
    if ( par != 0 )
	par->addChild( this );
}

Node::Status Node::inheritedStatus() const
{
    Status parentStatus = Approved;
    if ( par != 0 )
	parentStatus = inheritedStatus();
    return (Status) QMAX( (int) sta, (int) parentStatus );
}

InnerNode::~InnerNode()
{
    while ( !children.isEmpty() )
	delete children.first();
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

FunctionNode *InnerNode::findFunctionNode( const FunctionNode *clone )
{
    QMap<QString, Node *>::ConstIterator c =
	    primaryFunctionMap.find( clone->name() );
    if ( c == primaryFunctionMap.end() ) {
	return 0;
    } else {
	if ( isSameSignature(clone, (FunctionNode *) *c) ) {
	    return (FunctionNode *) *c;
	} else {
	    const QValueList<Node *>& secondaries =
		    secondaryFunctionMap[clone->name()];
	    QValueList<Node *>::ConstIterator s = secondaries.begin();
	    while ( s != secondaries.end() ) {
		if ( isSameSignature(clone, (FunctionNode *) *s) )
		    return (FunctionNode *) *s;
	    }
	    return 0;
	}
    }
}

bool InnerNode::isInnerNode() const
{
    return TRUE;
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

    QValueList<Parameter>::ConstIterator p1 = f1->parameters().begin(),
					 p2 = f2->parameters().begin();
    while ( p2 != f2->parameters().end() ) {
	if ( (*p1).leftType() != (*p2).leftType() ||
	     (*p1).rightType() != (*p2).rightType() )
	    return FALSE;
	++p1;
	++p2;
    }
    return TRUE;
}

void InnerNode::addChild( Node *child )
{
    children.append( child );
    if ( child->type() == Function ) {
	if ( *primaryFunctionMap.insert(child->name(), child, FALSE) != child )
	    secondaryFunctionMap[child->name()].append( child );
    } else {
	childMap.insert( child->name(), child );
    }
}

void InnerNode::removeChild( Node *child )
{
    children.remove( child );
    if ( child->type() == Function ) {
	QMap<QString, Node *>::Iterator primary =
		primaryFunctionMap.find( child->name() );
	if ( *primary == child ) {
	    QValueList<Node *>& secondaries =
		    secondaryFunctionMap[child->name()];
	    if ( secondaries.isEmpty() ) {
		primaryFunctionMap.remove( child->name() );
	    } else {
		primaryFunctionMap.replace( child->name(),
					    secondaries.first() );
		secondaries.remove( secondaries.begin() );
	    }
	} else {
	    secondaryFunctionMap[child->name()].remove( child );
	}
    } else {
	QMap<QString, Node *>::Iterator ent = childMap.find( child->name() );
	if ( *ent == child )
	    childMap.remove( ent );
    }
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
    if ( access != Private ) { // ###
	bas.append( RelatedClass(access, node, templateArgs) );
	node->der.append( RelatedClass(access, this) );
    }
}

QValueList<ClassSection> ClassNode::overviewSections() const
{
    ClassSection publicMembers( "Public Members" );
    ClassSection publicSlots( "Public Slots" );
    ClassSection publicSignals( "Signals" );
    ClassSection staticPublicMembers( "Static Public Members" );
    ClassSection importantInheritedMembers( "Important Inherited Members" );
    ClassSection properties( "Properties" );
    ClassSection protectedMembers( "Protected Members" );
    ClassSection protectedSlots( "Protected Slots" );
    ClassSection staticProtectedMembers( "Static Protected Members" );
    ClassSection privateMembers( "Private Members" );
    ClassSection privateSlots( "Private Slots" );
    ClassSection staticPrivateMembers( "Static Private Members" );
    ClassSection relatedNonMemberFunctions( "Related Non-Member Functions" );

    NodeList::ConstIterator c = childNodes().begin();
    while ( c != childNodes().end() ) {
	FunctionNode::Metaness metaness = FunctionNode::Plain;
	bool isStatic = FALSE;
	if ( (*c)->type() == Function ) {
	    FunctionNode *func = (FunctionNode *) *c;
	    metaness = func->metaness();
	    isStatic = func->isStatic();
	}
	bool isSlot = ( metaness == FunctionNode::Slot );

	switch ( (*c)->access() ) {
	case Public:
	    if ( isSlot ) {
		publicSlots.members.append( *c );
	    } else if ( metaness == FunctionNode::Signal ) {
		publicSignals.members.append( *c );
	    } else if ( isStatic ) {
		staticPublicMembers.members.append( *c );
	    } else if ( (*c)->type() == Property ) {
		properties.members.append( *c );
	    } else {
		publicMembers.members.append( *c );
	    }
	    break;
	case Protected:
	    if ( isSlot ) {
		protectedSlots.members.append( *c );
	    } else if ( isStatic ) {
		staticProtectedMembers.members.append( *c );
	    } else {
		protectedMembers.members.append( *c );
	    }
	    break;
	case Private:
	    if ( isSlot ) {
		privateSlots.members.append( *c );
	    } else if ( isStatic ) {
		staticPrivateMembers.members.append( *c );
	    } else {
		privateMembers.members.append( *c );
	    }
	}
	++c;
    }

    QValueList<ClassSection> sections;
    append( &sections, publicMembers );
    append( &sections, publicSlots );
    append( &sections, publicSignals );
    append( &sections, staticPublicMembers );
    append( &sections, importantInheritedMembers );
    append( &sections, properties );
    append( &sections, protectedMembers );
    append( &sections, protectedSlots );
    append( &sections, staticProtectedMembers );
    if ( FALSE ) { // ###
	append( &sections, privateMembers );
	append( &sections, privateSlots );
	append( &sections, staticPrivateMembers );
    }
    append( &sections, relatedNonMemberFunctions );
    return sections;
}

QValueList<ClassSection> ClassNode::detailedSections() const
{
    ClassSection memberTypes( "Member Type Documentation" );
    ClassSection memberFunctions( "Member Function Documentation" );
    ClassSection properties( "Property Documentation" );
    ClassSection relatedNonMemberFunctions(
	    "Related Non-Member Function Documentation" );

    QMap<QString, Node *> typeMap;
    QMap<QString, Node *> funcMap;
    QMap<QString, Node *> propertyMap;

    NodeList::ConstIterator c = childNodes().begin();
    while ( c != childNodes().end() ) {
	if ( (*c)->type() == Enum || (*c)->type() == Typedef ) {
	    typeMap.insert( (*c)->name(), *c );
	} else if ( (*c)->type() == Function ) {
	    funcMap.insert( (*c)->name(), *c ); // ### overloads
	} else if ( (*c)->type() == Property ) {
	    propertyMap.insert( (*c)->name(), *c );
	}
	++c;
    }

    memberTypes.members = nodeList( typeMap );
    memberFunctions.members = nodeList( funcMap );
    properties.members = nodeList( propertyMap );

    QValueList<ClassSection> sections;
    append( &sections, memberTypes );
    append( &sections, memberFunctions );
    append( &sections, properties );
    append( &sections, relatedNonMemberFunctions );
    return sections;
}

void ClassNode::append( QValueList<ClassSection> *sectionList,
			const ClassSection& section )
{
    if ( !section.members.isEmpty() )
	sectionList->append( section );
}

NodeList ClassNode::nodeList( const QMap<QString, Node *>& map )
{
    NodeList members;

    QMap<QString, Node *>::ConstIterator m = map.begin();
    while ( m != map.end() ) {
	members.append( *m );
	++m;
    }
    return members;
}

EnumNode::EnumNode( InnerNode *parent, const QString& name )
    : LeafNode( Enum, parent, name )
{
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
      con( FALSE ), sta( FALSE ), ove( 1 )
{
}

void FunctionNode::addParameter( const Parameter& parameter )
{
    params.append( parameter );
}

void FunctionNode::borrowParameterNames( const FunctionNode *source )
{
    QValueList<Parameter>::Iterator t = params.begin();
    QValueList<Parameter>::ConstIterator s = source->params.begin();
    while ( s != source->params.end() && t != params.end() ) {
	if ( !(*s).name().isEmpty() )
	    (*t).setName( (*s).name() );
	++s;
	++t;
    }
}

PropertyNode::PropertyNode( InnerNode *parent, const QString& name )
    : LeafNode( Property, parent, name ), sto( Trool_Default ),
      des( Trool_Default )
{
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
