/*
  webgenerator.h
*/

#ifndef WEBGENERATOR_H
#define WEBGENERATOR_H

#include <qfile.h>
#include <qtextstream.h>

#include "generator.h"

class ClassNode;
class InnerNode;
class NamespaceNode;

class WebGenerator : public Generator
{
public:
    WebGenerator();
    ~WebGenerator();

    virtual void generateTree( const Tree *tree, const CodeMarker *marker );

protected:
    virtual QString fileBase( const Node *node ) = 0;
    virtual QString fileExtension( const Node *node ) = 0;
    virtual void generateNamespaceNode( const NamespaceNode *namespasse,
					const CodeMarker *marker ) = 0;
    virtual void generateClassNode( const ClassNode *classe,
				    const CodeMarker *marker ) = 0;
    QString fileName( const Node *node );
    QTextStream& out();

private:
    void generateInnerNode( const InnerNode *node, const CodeMarker *marker );

    QFile outFile;
    QTextStream outStream;
};

#endif
