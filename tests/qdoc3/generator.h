/*
  generator.h
*/

#ifndef GENERATOR_H
#define GENERATOR_H

#include <qregexp.h>
#include <qstring.h>

#include "molecule.h"

class ClassNode;
class CodeMarker;
class Doc;
class FakeNode;
class FunctionNode;
class Molecule;
class NamespaceNode;
class Node;
class Tree;

class Generator
{
public:
    Generator();
    virtual ~Generator();

    virtual QString formatString() const = 0;
    virtual void generateTree( const Tree *tree, const CodeMarker *marker ) = 0;

protected:
    virtual void startMolecule( const Node *relative,
				const CodeMarker *marker );
    virtual void endMolecule( const Node *relative, const CodeMarker *marker );
    virtual void generateAtom( const Atom *atom, const Node *relative,
			       const CodeMarker *marker );
    virtual void generateNamespaceNode( const NamespaceNode *namespasse,
					const CodeMarker *marker );
    virtual void generateClassNode( const ClassNode *classe,
				    const CodeMarker *marker );
    virtual void generateFakeNode( const FakeNode *fake,
				   const CodeMarker *marker );

    virtual void generateMolecule( const Molecule& molecule,
				   const Node *relative,
				   const CodeMarker *marker );
    virtual void generateBody( const Node *node, const CodeMarker *marker );
    virtual void generateAlsoList( const Node *node, const CodeMarker *marker );
    virtual void generateInherits( const ClassNode *classe,
				   const CodeMarker *marker );
    virtual void generateInheritedBy( const ClassNode *classe,
				      const CodeMarker *marker );
    QString indent( int level, const QString& markedCode );
    QString plainCode( const QString& markedCode );
    QString typeString( const Node *node );
    Molecule sectionHeading( const Atom *sectionBegin );

private:
    void generateStatus( const Node *node, const CodeMarker *marker );
    void generateOverload( const Node *node, const CodeMarker *marker );
    void generateReimplementedFrom( const FunctionNode *func,
				    const CodeMarker *marker );
    void generateReimplementedBy( const FunctionNode *func,
				  const CodeMarker *marker );

    QRegExp amp;
    QRegExp lt;
    QRegExp gt;
    QRegExp quot;
    QRegExp tag;
};

#endif
