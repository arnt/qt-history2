/*
  generator.h
*/

#ifndef GENERATOR_H
#define GENERATOR_H

#include <qregexp.h>
#include <qstring.h>

#include "atom.h"

class CodeMarker;
class Doc;
class Node;
class Tree;

class Generator
{
public:
    Generator();
    virtual ~Generator();

    virtual void generateTree( const Tree *tree, const CodeMarker *marker ) = 0;

protected:
    virtual void generateAtom( const Atom *atom, const Node *relative,
			       const CodeMarker *marker );
    void generateAtomSubList( const Atom *begin, const Atom *end,
			      const Node *relative, const CodeMarker *marker );
    void generateDoc( const Doc& doc, const Node *relative,
		      const CodeMarker *marker );
    bool findAtomSubList( const Atom *atomList, Atom::Type leftType,
			  Atom::Type rightType, const Atom **beginPtr,
			  const Atom **endPtr );
    QString plainCode( const QString& markedCode );

    QRegExp amp;
    QRegExp lt;
    QRegExp gt;
    QRegExp quot;
    QRegExp tag;
};

#endif
