/*
  text.h
*/

#ifndef TEXT_H
#define TEXT_H

#include "atom.h"

class Text
{
public:
    Text();
    Text( const Text& text );
    ~Text();

    Text& operator=( const Text& text );

    Atom *firstAtom() { return first; }
    Atom *lastAtom() { return last; }
    Text& operator<<( Atom::Type atomType );
    Text& operator<<( const QString& string );
    Text& operator<<( const Atom& atom );
    Text& operator<<( const Text& text );

    bool isEmpty() const { return first == 0; }
    QString toString() const;
    const Atom *firstAtom() const { return first; }
    const Atom *lastAtom() const { return last; }
    Text subText( Atom::Type before, Atom::Type after ) const;
    void dump() const;

    static Text subText( const Atom *begin, const Atom *end = 0 );

private:
    void clear();

    Atom *first;
    Atom *last;
};

#endif
