/*
  molecule.h
*/

#ifndef MOLECULE_H
#define MOLECULE_H

#include "atom.h"

class Molecule
{
public:
    Molecule();
    Molecule( const Molecule& molecule );
    ~Molecule();

    Molecule& operator=( const Molecule& molecule );

    Atom *firstAtom() { return first; }
    Atom *lastAtom() { return last; }
    Molecule& operator<<( Atom::Type atomType );
    Molecule& operator<<( const QString& string );
    Molecule& operator<<( const Atom& atom );
    Molecule& operator<<( const Molecule& molecule );

    bool isEmpty() const { return first == 0; }
    const Atom *firstAtom() const { return first; }
    const Atom *lastAtom() const { return last; }
    Molecule subMolecule( const Atom *begin, const Atom *end = 0 ) const;
    Molecule subMolecule( Atom::Type before, Atom::Type after ) const;

private:
    void clear();

    Atom *first;
    Atom *last;
};

#endif
