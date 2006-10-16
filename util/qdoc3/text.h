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
    void stripFirstAtom();
    void stripLastAtom();

    bool isEmpty() const { return first == 0; }
    QString toString() const;
    const Atom *firstAtom() const { return first; }
    const Atom *lastAtom() const { return last; }
    Text subText( Atom::Type left, Atom::Type right, const Atom *from = 0) const;
    void dump() const;

    static Text subText( const Atom *begin, const Atom *end = 0 );
    static Text sectionHeading(const Atom *sectionBegin);
    static int compare(const Text &text1, const Text &text2);

private:
    void clear();

    Atom *first;
    Atom *last;
};

inline bool operator==(const Text &text1, const Text &text2)
{ return Text::compare(text1, text2) == 0; }
inline bool operator!=(const Text &text1, const Text &text2)
{ return Text::compare(text1, text2) != 0; }
inline bool operator<(const Text &text1, const Text &text2)
{ return Text::compare(text1, text2) < 0; }
inline bool operator<=(const Text &text1, const Text &text2)
{ return Text::compare(text1, text2) <= 0; }
inline bool operator>(const Text &text1, const Text &text2)
{ return Text::compare(text1, text2) > 0; }
inline bool operator>=(const Text &text1, const Text &text2)
{ return Text::compare(text1, text2) >= 0; }

#endif
