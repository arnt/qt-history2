/*
  pagegenerator.h
*/

#ifndef PAGEGENERATOR_H
#define PAGEGENERATOR_H

#include <qptrstack.h>
#include <qtextstream.h>

#include "generator.h"
#include "location.h"

class ClassNode;
class InnerNode;
class NamespaceNode;

class PageGenerator : public Generator
{
public:
    PageGenerator();
    ~PageGenerator();

    virtual void generateTree( const Tree *tree, CodeMarker *marker );

protected:
    virtual QString fileBase( const Node *node ) = 0;
    virtual QString fileExtension( const Node *node ) = 0;
    QString fileName( const Node *node );
    void beginSubPage( const Location& location, const QString& fileName );
    void endSubPage();
    QTextStream& out();

private:
    void generateInnerNode( const InnerNode *node, CodeMarker *marker );

    QPtrStack<QTextStream> outStreamStack;
};

#endif
