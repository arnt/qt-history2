/*
  pagegenerator.cpp
*/

#include <qfile.h>
#include <qfileinfo.h>

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

QString PageGenerator::fileBase(const Node *node)
{
    if (node->relates())
	node = node->relates();
    else if (!node->isInnerNode())
	node = node->parent();

    QString base = node->doc().baseName();
    if (base.isEmpty()) {
	const Node *p = node;

        forever {
            base.prepend(p->name());
            if (!p->parent() || p->parent()->name().isEmpty() || p->parent()->type() == Node::Fake)
                break;
            base.prepend("-");
            p = p->parent();
        }

        if (node->type() == Node::Fake) {
#ifdef QDOC2_COMPAT
            if (base.endsWith(".html"))
	        base.truncate(base.length() - 5);
#endif
        }

	base.replace(QRegExp("[^A-Za-z0-9]+"), " ");
	base = base.trimmed();
	base.replace(" ", "-");
	base = base.toLower();
    }
    return base;
}

QString PageGenerator::fileName( const Node *node )
{
    return fileBase( node ) + "." + fileExtension();
}

QString PageGenerator::outFileName()
{
    return QFileInfo(static_cast<QFile *>(out().device())->fileName()).fileName();
}

void PageGenerator::beginSubPage( const Location& location,
				  const QString& fileName )
{
    QFile *outFile = new QFile( outputDir() + "/" + fileName );
    if ( !outFile->open(QFile::WriteOnly) )
	location.fatal( tr("Cannot open output file '%1'")
			.arg(outFile->fileName()) );
    outStreamStack.push( new QTextStream(outFile) );
}

void PageGenerator::endSubPage()
{
    outStreamStack.top()->flush();
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
    if (!node->url().isEmpty())
        return;

    if ( node->parent() != 0 ) {
	beginSubPage( node->location(), fileName(node) );
	if ( node->type() == Node::Namespace || node->type() == Node::Class) {
	    generateClassLikeNode(node, marker);
	} else if ( node->type() == Node::Fake ) {
	    generateFakeNode(static_cast<const FakeNode *>(node), marker);
	}
	endSubPage();
    }

    NodeList::ConstIterator c = node->childNodes().begin();
    while ( c != node->childNodes().end() ) {
	if ((*c)->isInnerNode() && (*c)->access() != Node::Private)
	    generateInnerNode( (const InnerNode *) *c, marker );
	++c;
    }
}
