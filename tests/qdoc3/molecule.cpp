/*
  molecule.cpp
*/

#include "molecule.h"

Molecule::Molecule()
    : first( 0 ), last( 0 )
{
}

Molecule::Molecule( const Molecule& molecule )
{
    first = 0;
    last = 0;
    operator=( molecule );
}

Molecule::~Molecule()
{
    clear();
}

Molecule& Molecule::operator=( const Molecule& molecule )
{
    if ( this != &molecule ) {
	clear();
	operator<<( molecule );
    }
    return *this;
}

Molecule& Molecule::operator<<( Atom::Type atomType )
{
    return operator<<( Atom(atomType) );
}

Molecule& Molecule::operator<<( const QString& string )
{
    return operator<<( Atom(Atom::String, string) );
}

Molecule& Molecule::operator<<( const Atom& atom )
{
    if ( first == 0 ) {
	first = new Atom( atom.type(), atom.string() );
	last = first;
    } else {
	last = new Atom( last, atom.type(), atom.string() );
    }
    return *this;
}

Molecule& Molecule::operator<<( const Molecule& molecule )
{
    const Atom *atom = molecule.firstAtom();
    while ( atom != 0 ) {
	operator<<( *atom );
	atom = atom->next();
    }
    return *this;
}

Molecule Molecule::subMolecule( const Atom *begin, const Atom *end ) const
{
    Molecule molecule;
    if ( begin != 0 ) {
	while ( begin != end ) {
	    molecule << *begin;
	    begin = begin->next();
	}
    }
    return molecule;
}

Molecule Molecule::subMolecule( Atom::Type before, Atom::Type after ) const
{
    const Atom *begin = firstAtom();
    const Atom *end;

    while ( begin != 0 && begin->type() != before )
	begin = begin->next();
    if ( begin != 0 )
	begin = begin->next();

    end = begin;
    while ( end != 0 && end->type() != after )
	end = end->next();
    if ( end == 0 )
	begin = 0;
    return subMolecule( begin, end );
}

void Molecule::clear()
{
    while ( first != 0 ) {
	Atom *atom = first;
	first = first->next();
	delete atom;
    }
    first = 0;
    last = 0;
}
