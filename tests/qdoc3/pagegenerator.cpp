/*
  pagegenerator.cpp
*/

#include <qfile.h>

#include "pagegenerator.h"
#include "tree.h"

PageGenerator::PageGenerator()
{
}

PageGenerator::~PageGenerator()
{
    while ( !outStreamStack.isEmpty() )
	endSubPage();
}

void PageGenerator::generateTree( const Tree *tree, CodeMarker *marker )
{
    generateInnerNode( tree->root(), marker );
}

QString PageGenerator::fileName( const Node *node )
{
    return fileBase( node ) + "." + fileExtension( node );
}

void PageGenerator::beginSubPage( const Location& location,
				  const QString& fileName )
{
    QFile *outFile = new QFile( outputDir() + "/" + fileName );
    if ( !outFile->open(IO_WriteOnly) )
	location.fatal( tr("Cannot open output file '%1'")
			.arg(outFile->name()) );
    outStreamStack.push( new QTextStream(outFile) );
}

void PageGenerator::endSubPage()
{
    delete outStreamStack.top()->device();
    delete outStreamStack.pop();
}

QTextStream &PageGenerator::out()
{
    return *outStreamStack.top();
}

void PageGenerator::generateInnerNode( const InnerNode *node,
				       CodeMarker *marker )
{
    if ( node->parent() != 0 ) {
	beginSubPage( node->location(), fileName(node) );
	if ( node->type() == Node::Namespace ) {
	    generateNamespaceNode( (const NamespaceNode *) node, marker );
	} else if ( node->type() == Node::Class ) {
	    generateClassNode( (const ClassNode *) node, marker );
	} else if ( node->type() == Node::Fake ) {
	    generateFakeNode( (const FakeNode *) node, marker );
	}
	endSubPage();
    }

    NodeList::ConstIterator c = node->childNodes().begin();
    while ( c != node->childNodes().end() ) {
	if ( (*c)->isInnerNode() )
	    generateInnerNode( (const InnerNode *) *c, marker );
	++c;
    }
}
