/*
  atom.cpp
*/

#include "atom.h"
#include "messages.h"

/*! \class Atom
    \brief The Atom class is the fundamental unit for representing
    documents internally.

  Atoms have a \i type and are completed by a \i string whose
  meaning depends on the \i type. For example, the string
  \quotation
      \e italic text looks nicer than \b bold text"
  \endquotation
  is represented by the following atoms:
  \quotation
      (FormatBegin, "italic")
      (RawString, 
      (FormatEnd, "italic")
  \endquotation

  \also Molecule
*/

/*! \enum Atom::Type

  \value AbstractBegin
  \value AbstractEnd
  \value BaseName
  \value BriefBegin
  \value BriefEnd
  \value C
  \value CaptionBegin
  \value CaptionEnd
  \value Code
  \value FootnoteBegin
  \value FootnoteEnd
  \value FormatBegin
  \value FormatEnd
  \value GeneratedList
  \value Image
  \value Link
  \value LinkNode
  \value ListBegin
  \value ListItemNumber
  \value ListItemBegin
  \value ListItemEnd
  \value ListEnd
  \value Nop
  \value ParagraphBegin
  \value ParagraphEnd
  \value QuotationBegin
  \value QuotationEnd
  \value RawFormat
  \value RawString
  \value SectionBegin
  \value SectionEnd
  \value SectionHeadingBegin
  \value SectionHeadingEnd
  \value SidebarBegin
  \value SidebarEnd
  \value String
  \value TableBegin
  \value TableEnd
  \value TableOfContents
  \value Target
  \value UnknownCommand
*/

static const struct {
    const char *english;
    int no;
} atms[] = {
    { "AbstractBegin", Atom::AbstractBegin },
    { "AbstractEnd", Atom::AbstractEnd },
    { "BaseName", Atom::BaseName },
    { "BriefBegin", Atom::BriefBegin },
    { "BriefEnd", Atom::BriefEnd },
    { "C", Atom::C },
    { "CaptionBegin", Atom::CaptionBegin },
    { "CaptionEnd", Atom::CaptionEnd },
    { "Code", Atom::Code },
    { "FootnoteBegin", Atom::FootnoteBegin },
    { "FootnoteEnd", Atom::FootnoteEnd },
    { "FormatBegin", Atom::FormatBegin },
    { "FormatEnd", Atom::FormatEnd },
    { "GeneratedList", Atom::GeneratedList },
    { "Image", Atom::Image },
    { "Link", Atom::Link },
    { "LinkNode", Atom::LinkNode },
    { "ListBegin", Atom::ListBegin },
    { "ListItemNumber", Atom::ListItemNumber },
    { "ListItemBegin", Atom::ListItemBegin },
    { "ListItemEnd", Atom::ListItemEnd },
    { "ListEnd", Atom::ListEnd },
    { "Nop", Atom::Nop },
    { "ParagraphBegin", Atom::ParagraphBegin },
    { "ParagraphEnd", Atom::ParagraphEnd },
    { "QuotationBegin", Atom::QuotationBegin },
    { "QuotationEnd", Atom::QuotationEnd },
    { "RawFormat", Atom::RawFormat },
    { "RawString", Atom::RawString },
    { "SectionBegin", Atom::SectionBegin },
    { "SectionEnd", Atom::SectionEnd },
    { "SectionHeadingBegin", Atom::SectionHeadingBegin },
    { "SectionHeadingEnd", Atom::SectionHeadingEnd },
    { "SidebarBegin", Atom::SidebarBegin },
    { "SidebarEnd", Atom::SidebarEnd },
    { "String", Atom::String },
    { "TableBegin", Atom::TableBegin },
    { "TableEnd", Atom::TableEnd },
    { "TableOfContents", Atom::TableOfContents },
    { "Target", Atom::Target },
    { "UnknownCommand", Atom::UnknownCommand },
    { 0, 0 }
};

/*! \fn Atom::Atom( Type type, const QString& string )

  Constructs an atom (\a type, \a string) outside of any atom list.
*/

/*! \fn Atom( Atom *prev, Type type, const QString& string )

  Constructs an atom (\a type, \a string) that follows \a prev in \a
  prev's atom list.
*/

/*! \fn void Atom::appendChar( QChar ch )

  Appends \a ch to the string parameter of this atom.

  \also string()
*/

/*! \fn void Atom::appendString( const QString& string )

  Appends \a string to the string parameter of this atom.

  \also string()
*/

/*! \fn void Atom::chopString()

  \also string()
*/

/*! \fn Atom *Atom::next()

  Returns the next atom in the atom list.

  \also type(), string()
*/

/*! \fn const Atom *Atom::next() const
    \overload
*/

/*! \fn Type Atom::type() const

  Returns the type of this atom.

  \also string(), next()
*/

/*!
  Returns the type of this atom as a string. Returns "Invalid" if
  type() returns an impossible value.

  This is only useful for debugging.

  \also type()
*/
QString Atom::typeString() const
{
    static bool deja = FALSE;

    if ( !deja ) {
	int i = 0;
	while ( atms[i].english != 0 ) {
	    if ( atms[i].no != i )
		Messages::internalError( qdoc::tr("atom %1 missing").arg(i) );
	    i++;
	}
	deja = TRUE;
    }

    int i = (int) type();
    if ( i < 0 || i > (int) Last ) {
	return "Invalid";
    } else {
	return atms[i].english;
    }
}

/*! \fn const QString& Atom::string() const

  Returns the string parameter that together with the type
  characterizes this atom.

  \also type(), next()
*/
