/*
  codemarker.cpp
*/

#include <stdio.h>

#include "codemarker.h"
#include "config.h"
#include "node.h"

QString CodeMarker::defaultLang;
QValueList<CodeMarker *> CodeMarker::markers;

CodeMarker::CodeMarker()
    : amp( "&" ), lt( "<" ), gt( ">" ), quot( "\"" )
{
    markers.prepend( this );
}

CodeMarker::~CodeMarker()
{
    markers.remove( this );
}

void CodeMarker::initializeMarker( const Config& /* config */ )
{
}

void CodeMarker::terminateMarker()
{
}

void CodeMarker::initialize( const Config& config )
{
    defaultLang = config.getString( CONFIG_LANGUAGE );

    QValueList<CodeMarker *>::ConstIterator m = markers.begin();
    while ( m != markers.end() ) {
	(*m)->initializeMarker( config );
	++m;
    }
}

void CodeMarker::terminate()
{
    QValueList<CodeMarker *>::ConstIterator m = markers.begin();
    while ( m != markers.end() ) {
	(*m)->terminateMarker();
	++m;
    }
}

CodeMarker *CodeMarker::markerForCode( const QString& code )
{
    CodeMarker *defaultMarker = markerForLanguage( defaultLang );
    if ( defaultMarker != 0 && defaultMarker->recognizeCode(code) )
	return defaultMarker;

    QValueList<CodeMarker *>::ConstIterator m = markers.begin();
    while ( m != markers.end() ) {
	if ( (*m)->recognizeCode(code) )
	    return *m;
	++m;
    }
    return defaultMarker;
}

CodeMarker *CodeMarker::markerForFileName( const QString& fileName )
{
    CodeMarker *defaultMarker = markerForLanguage( defaultLang );
    int dot = -1;
    while ( (dot = fileName.find(".", dot + 1)) != -1 ) {
	QString ext = fileName.mid( dot + 1 );
	if ( defaultMarker != 0 && defaultMarker->recognizeExtension(ext) )
	    return defaultMarker;
	QValueList<CodeMarker *>::ConstIterator m = markers.begin();
	while ( m != markers.end() ) {
	    if ( (*m)->recognizeExtension(ext) )
		return *m;
	    ++m;
	}
    }
    return defaultMarker;
}

CodeMarker *CodeMarker::markerForLanguage( const QString& lang )
{
    QValueList<CodeMarker *>::ConstIterator m = markers.begin();
    while ( m != markers.end() ) {
	if ( (*m)->recognizeLanguage(lang) )
	    return *m;
	++m;
    }
    return 0;
}

const Node *CodeMarker::nodeForString( const QString& string )
{
    void *ptr = 0;
    sscanf( string.latin1(), "%p", &ptr );
    return (const Node *) ptr;
}

QString CodeMarker::stringForNode( const Node *node )
{
    QString str;
    str.sprintf( "%p", (void *) node );
    return str;
}

QString CodeMarker::protect( const QString& string )
{
    QString marked = string;
    marked.replace( amp, "&amp;" );
    marked.replace( lt, "&lt;" );
    marked.replace( gt, "&gt;" );
    marked.replace( quot, "&quot;" );
    return marked;
}

QString CodeMarker::taggedNode( const Node *node )
{
    QString tag;

    switch ( node->type() ) {
    case Node::Namespace:
	tag = "@namespace";
	break;
    case Node::Class:
	tag = "@class";
	break;
    case Node::Enum:
	tag = "@enum";
	break;
    case Node::Typedef:
	tag = "@typedef";
	break;
    case Node::Function:
	tag = "@function";
	break;
    case Node::Property:
	tag = "@property";
	break;
    default:
	tag = "@unknown";
    }
    return "<" + tag + ">" + protect( node->name() ) + "</" + tag + ">";
}

QString CodeMarker::linkTag( const Node *node, const QString& body )
{
    return "<@link node=\"" + stringForNode( node ) + "\">" + body + "</@link>";
}

QString CodeMarker::sortName( const Node *node )
{
    if ( node->type() == Node::Function ) {
	FunctionNode *func = (FunctionNode *) node;
	int sortNo;
	if ( func->metaness() == FunctionNode::Ctor ) {
	    sortNo = 1;
	} else if ( func->metaness() == FunctionNode::Dtor ) {
	    sortNo = 2;
	} else {
	    sortNo = 3;
	}
	return QString::number( sortNo ) + func->name() + " " +
	       QString::number( func->overloadNumber(), 36 );
    } else {
	return node->name();
    }
}

void CodeMarker::insert( FastClassSection& fastSection, Node *node )
{
    if ( node->parent() == (const InnerNode *) fastSection.classe ) {
	fastSection.memberMap.insert( sortName(node), node );
    } else if ( node->parent()->type() == Node::Class ) {
	if ( fastSection.inherited.isEmpty() ||
	     fastSection.inherited.last().first != node->parent() )
	    fastSection.inherited.append(
		    QPair<ClassNode *, int>((ClassNode *) node->parent(), 0) );
	fastSection.inherited.last().second++;
    }
}

void CodeMarker::append( QValueList<ClassSection>& sectionList,
			 const FastClassSection& fastSection )
{
    if ( !fastSection.memberMap.isEmpty() ||
	 !fastSection.inherited.isEmpty() ) {
	ClassSection section( fastSection.name, fastSection.singularMember,
			      fastSection.pluralMember );
	section.members = fastSection.memberMap.values();
	section.inherited = fastSection.inherited;
	sectionList.append( section );
    }
}
