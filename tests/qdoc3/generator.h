/*
  generator.h
*/

#ifndef GENERATOR_H
#define GENERATOR_H

#include <qregexp.h>
#include <qstring.h>

#include "text.h"

class ClassNode;
class Config;
class CodeMarker;
class Doc;
class FakeNode;
class FunctionNode;
class NamespaceNode;
class Node;
class Tree;

class Generator
{
public:
    Generator();
    virtual ~Generator();

    virtual void initializeGenerator( const Config& config );
    virtual void terminateGenerator();
    virtual QString format() = 0;
    virtual void generateTree( const Tree *tree, CodeMarker *marker ) = 0;

    static void initialize( const Config& config );
    static void terminate();
    static Generator *generatorForFormat( const QString& format );

protected:
    virtual void startText( const Node *relative, CodeMarker *marker );
    virtual void endText( const Node *relative, CodeMarker *marker );
    virtual void generateAtom( const Atom *atom, const Node *relative,
			       CodeMarker *marker );
    virtual void generateNamespaceNode( const NamespaceNode *namespasse,
					CodeMarker *marker );
    virtual void generateClassNode( const ClassNode *classe,
				    CodeMarker *marker );
    virtual void generateFakeNode( const FakeNode *fake, CodeMarker *marker );

    virtual void generateText( const Text& text, const Node *relative,
			       CodeMarker *marker );
    virtual void generateBody( const Node *node, CodeMarker *marker );
    virtual void generateAlsoList( const Node *node, CodeMarker *marker );
    virtual void generateInherits( const ClassNode *classe,
				   CodeMarker *marker );
    virtual void generateInheritedBy( const ClassNode *classe,
				      CodeMarker *marker );
    const QString& outputDir() { return outDir; }
    QString indent( int level, const QString& markedCode );
    QString plainCode( const QString& markedCode );
    QString typeString( const Node *node );
    Text sectionHeading( const Atom *sectionBegin );
    void unknownAtom( const Atom *atom );

private:
    Atom *generateAtomList( const Atom *atom, const Node *relative,
			    CodeMarker *marker, bool generate );
    void generateStatus( const Node *node, CodeMarker *marker );
    void generateOverload( const Node *node, CodeMarker *marker );
    void generateReimplementedFrom( const FunctionNode *func,
				    CodeMarker *marker );
    void generateReimplementedBy( const FunctionNode *func,
				  CodeMarker *marker );

    QRegExp amp;
    QRegExp lt;
    QRegExp gt;
    QRegExp quot;
    QRegExp tag;

    static QValueList<Generator *> generators;
    static QString outDir;
};

#endif
