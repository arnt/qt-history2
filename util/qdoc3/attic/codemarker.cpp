/*
  codemarker.cpp
*/

#include <stdio.h>

#include "codemarker.h"
#include "node.h"

QString CodeMarker::dl = "C++";
QValueList<const CodeMarker *> CodeMarker::markers;

CodeMarker::CodeMarker()
    : amp( "&" ), lt( "<" ), gt( ">" ), quot( "\"" )
{
    markers.prepend( this );
}

CodeMarker::~CodeMarker()
{
    markers.remove( this );
}

const CodeMarker *CodeMarker::markerForCode( const QString& code )
{
    const CodeMarker *defaultMarker = markerForLanguage( defaultLanguage() );
    if ( defaultMarker != 0 && defaultMarker->recognizeCode(code) )
	return defaultMarker;

    QValueList<const CodeMarker *>::ConstIterator m = markers.begin();
    while ( m != markers.end() ) {
	if ( (*m)->recognizeCode(code) )
	    return *m;
	++m;
    }
    return defaultMarker;
}

const CodeMarker *CodeMarker::markerForFileName( const QString& fileName )
{
    QString ext;
    int k = fileName.findRev( '.' );
    if ( k != -1 )
	ext = fileName.mid( k + 1 ).lower();

    const CodeMarker *defaultMarker = markerForLanguage( defaultLanguage() );
    if ( defaultMarker != 0 && defaultMarker->recognizeExtension(ext) )
	return defaultMarker;

    QValueList<const CodeMarker *>::ConstIterator m = markers.begin();
    while ( m != markers.end() ) {
	if ( (*m)->recognizeExtension(ext) )
	    return *m;
	++m;
    }
    return defaultMarker;
}

const CodeMarker *CodeMarker::markerForLanguage( const QString& lang )
{
    QValueList<const CodeMarker *>::ConstIterator m = markers.begin();
    while ( m != markers.end() ) {
	if ( (*m)->recognizeLanguage(lang.lower()) )
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

QString CodeMarker::protect( const QString& string ) const
{
    QString marked = string;
    marked.replace( amp, "&amp;" );
    marked.replace( lt, "&lt;" );
    marked.replace( gt, "&gt;" );
    marked.replace( quot, "&quot;" );
    return marked;
}

QString CodeMarker::taggedNode( const Node *node ) const
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

QString CodeMarker::linkTag( const Node *node, const QString& body ) const
{
    return "<@link node=\"" + stringForNode( node ) + "\">" + body + "</@link>";
}
