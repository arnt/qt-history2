/*
  atom.h
*/

#ifndef ATOM_H
#define ATOM_H

#include <qstring.h>

class Atom
{
public:
    enum Type { AbstractBegin, AbstractEnd, Alias, BriefBegin, BriefEnd, C,
		CaptionBegin,
		CaptionEnd, CitationBegin, CitationEnd, Code, DocBegin,
		DocEnd, FootnoteBegin,
		FootnoteEnd, FormatBegin, FormatEnd, GeneratedList,
		Img, Index, Link, ListBegin, ListItemNumber,
		ListItemBegin, ListItemEnd, ListEnd, ParagraphBegin,
		ParagraphEnd, RawFormat, RawString, SectionBegin, SectionEnd,
		SectionHeadingBegin, SectionHeadingEnd,
		SidebarBegin, SidebarEnd,
		String, TableBegin, TableEnd, TableOfContents,
		Target, TitleBegin, TitleEnd };

    Atom( Type type )
	: nex( 0 ), typ( type ) { }
    Atom( Atom *prev, Type type, const QString& string = "" )
	: nex( 0 ), typ( type ), str( string ) { prev->nex = this; }
    ~Atom() { delete nex; }

    void appendChar( QChar ch ) { str += ch; }
    void appendString( const QString& string ) { str += string; }
    void chopString() { str.truncate( str.length() - 1 ); }
    Atom *next() { return nex; }

    const Atom *next() const { return nex; }
    int type() const { return typ; }
    const QString& string() const { return str; }

private:
    Atom *nex;
    int typ;
    QString str;
};

#endif
