/*
  webgenerator.cpp
*/

#include "messages.h"
#include "tree.h"
#include "webgenerator.h"

WebGenerator::WebGenerator()
    : outStream( &outFile )
{
}

WebGenerator::~WebGenerator()
{
}

void WebGenerator::generateTree( const Tree *tree, const CodeMarker *marker )
{
    generateInnerNode( tree->root(), marker );
}

QString WebGenerator::fileName( const Node *node )
{
    return fileBase( node ) + "." + fileExtension( node );
}

QTextStream& WebGenerator::out()
{
    return outStream;
}

void WebGenerator::generateInnerNode( const InnerNode *node,
				      const CodeMarker *marker )
{
    outFile.setName( fileName(node) );
    if ( !outFile.open(IO_WriteOnly) ) {
	syswarning( "Cannot open '%s' for writing", outFile.name().latin1() );
    } else {
	if ( node->type() == Node::Namespace ) {
	    generateNamespaceNode( (const NamespaceNode *) node, marker );
	} else if ( node->type() == Node::Class ) {
	    generateClassNode( (const ClassNode *) node, marker );
	}
	outFile.close();
    }

    NodeList::ConstIterator c = node->childNodes().begin();
    while ( c != node->childNodes().end() ) {
	if ( (*c)->isInnerNode() )
	    generateInnerNode( (const InnerNode *) *c, marker );
	++c;
    }
}
