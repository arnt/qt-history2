/*
  atom.cpp
*/

#include "atom.h"
#include "messages.h"

static const char * const typeTable[] = {
    "AbstractBegin", "AbstractEnd", "Alias", "AliasArg",
    "BriefBegin", "BriefEnd", "C", "CaptionBegin", "CaptionEnd",
    "CitationBegin", "CitationEnd", "Code", "DocBegin", "DocEnd",
    "FootnoteBegin", "FootnoteEnd", "FormatBegin", "FormatEnd",
    "GeneratedList", "Image", "Index", "Link", "ListBegin",
    "ListItemNumber", "ListItemBegin", "ListItemEnd", "ListEnd",
    "ParagraphBegin", "ParagraphEnd", "RawFormat", "RawString",
    "SectionBegin", "SectionEnd", "SectionHeadingBegin",
    "SectionHeadingEnd", "SidebarBegin", "SidebarEnd", "String",
    "TableBegin", "TableEnd", "TableOfContents", "Target",
    "TitleBegin", "TitleEnd", 0
};

QString Atom::typeString() const
{
    int t = (int) type();
    if ( t < 0 || t > (int) Last ) {
	return "Invalid";
    } else if ( sizeof(typeTable) / sizeof(typeTable[0]) != (int) Last + 1 ) {
	message( 0, "Internal error: 'typeTable' not up to date" );
	return "Invalid";
    } else {
	return typeTable[t];
    }
}
