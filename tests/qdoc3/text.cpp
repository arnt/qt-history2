/*
  text.cpp
*/

#include <qregexp.h>

#include "text.h"

Text::Text()
    : first( 0 ), last( 0 )
{
}

Text::Text( const Text& text )
{
    first = 0;
    last = 0;
    operator=( text );
}

Text::~Text()
{
    clear();
}

Text& Text::operator=( const Text& text )
{
    if ( this != &text ) {
	clear();
	operator<<( text );
    }
    return *this;
}

Text& Text::operator<<( Atom::Type atomType )
{
    return operator<<( Atom(atomType) );
}

Text& Text::operator<<( const QString& string )
{
    return operator<<( Atom(Atom::String, string) );
}

Text& Text::operator<<( const Atom& atom )
{
    if ( first == 0 ) {
	first = new Atom( atom.type(), atom.string() );
	last = first;
    } else {
	last = new Atom( last, atom.type(), atom.string() );
    }
    return *this;
}

Text& Text::operator<<( const Text& text )
{
    const Atom *atom = text.firstAtom();
    while ( atom != 0 ) {
	operator<<( *atom );
	atom = atom->next();
    }
    return *this;
}

QString Text::toString() const
{
    QString str;
    const Atom *atom = firstAtom();
    while ( atom != 0 ) {
	if ( atom->type() == Atom::String )
	    str += atom->string();
	atom = atom->next();
    }
    return str;
}

Text Text::subText( Atom::Type left, Atom::Type right ) const
{
    const Atom *begin = firstAtom();
    const Atom *end;

    while ( begin != 0 && begin->type() != left )
	begin = begin->next();
    if ( begin != 0 )
	begin = begin->next();

    end = begin;
    while ( end != 0 && end->type() != right )
	end = end->next();
    if ( end == 0 )
	begin = 0;
    return subText( begin, end );
}

void Text::dump() const
{
    const Atom *atom = firstAtom();
    while ( atom != 0 ) {
	QString str = atom->string();
	str.replace( QRegExp("\\\\"), "\\\\" );
	str.replace( QRegExp("\""), "\\\"" );
	str.replace( QRegExp("\n"), "\\n" );
	str.replace( QRegExp("[\\x00-\\x1f?\\x7f-\\xfffff]"), "?" );
	qDebug( "    (%s, \"%s\")", atom->typeString().latin1(), str.latin1() );
	atom = atom->next();
    }
}

Text Text::subText( const Atom *begin, const Atom *end )
{
    Text text;
    if ( begin != 0 ) {
	while ( begin != end ) {
	    text << *begin;
	    begin = begin->next();
	}
    }
    return text;
}

void Text::clear()
{
    while ( first != 0 ) {
	Atom *atom = first;
	first = first->next();
	delete atom;
    }
    first = 0;
    last = 0;
}
