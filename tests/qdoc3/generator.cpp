/*
  generator.cpp
*/

#include "doc.h"
#include "generator.h"

Generator::Generator()
    : amp( "&amp;" ), lt( "&lt;" ), gt( "&gt;" ), quot( "&quot;" ),
      tag( "</?@[^>]*>" )
{
}

Generator::~Generator()
{
}

void Generator::generateAtom( const Atom * /* atom */,
			      const Node * /* relative */,
			      const CodeMarker * /* marker */ )
{
}

void Generator::generateAtomSubList( const Atom *begin, const Atom *end,
				     const Node *relative,
				     const CodeMarker *marker )
{
    const Atom *atom = begin;
    while ( atom != end ) {
	generateAtom( atom, relative, marker );
	atom = atom->next();
    }
}

void Generator::generateDoc( const Doc& doc, const Node *relative,
			     const CodeMarker *marker )
{
    generateAtomSubList( doc.atomList(), 0, relative, marker );
    Atom *atomList = doc.createAlsoAtomList();
    generateAtomSubList( atomList, 0, relative, marker );
    delete atomList;
}

bool Generator::findAtomSubList( const Atom *atomList, Atom::Type leftType,
				 Atom::Type rightType, const Atom **beginPtr,
				 const Atom **endPtr )
{
    const Atom *begin = atomList;
    const Atom *end;

    while ( begin != 0 && begin->type() != leftType )
	begin = begin->next();
    if ( begin != 0 )
	begin = begin->next();
    end = begin;
    while ( end != 0 && end->type() != rightType )
	end = end->next();
    if ( end == 0 )
	begin = 0;

    if ( beginPtr != 0 )
	*beginPtr = begin;
    if ( endPtr != 0 )
	*endPtr = end;
    return begin != 0;
}

QString Generator::plainCode( const QString& markedCode )
{
    QString t = markedCode;
    t.replace( tag, "" );
    t.replace( quot, "\"" );
    t.replace( gt, ">" );
    t.replace( lt, "<" );
    t.replace( amp, "&" );
    return t;
}
