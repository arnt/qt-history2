/*
  atom.h
*/

#ifndef ATOM_H
#define ATOM_H

#include <qstring.h>

class Atom
{
public:
    enum Type { AbstractLeft, AbstractRight, BaseName, BriefLeft, BriefRight, C, CaptionLeft,
                CaptionRight, Code, FootnoteLeft, FootnoteRight, FormatElse, FormatEndif, FormatIf,
                FormattingLeft, FormattingRight, GeneratedList, Image, ImageText, InlineImage,
                LegaleseLeft, LegaleseRight, Link, LinkNode, ListLeft, ListItemNumber, ListTagLeft,
                ListTagRight, ListItemLeft, ListItemRight, ListRight, Nop, ParaLeft, ParaRight,
                QuotationLeft, QuotationRight, RawString, SectionLeft, SectionRight,
                SectionHeadingLeft, SectionHeadingRight, SidebarLeft, SidebarRight, String,
                TableLeft, TableRight, TableHeaderLeft, TableHeaderRight, TableRowLeft,
                TableRowRight, TableItemLeft, TableItemRight, TableOfContents, Target,
                UnhandledFormat, UnknownCommand, Last = UnknownCommand };

    Atom(Type type, const QString &string = "")
	: nex(0), typ(type), str(string) { }
    Atom(Atom *prev, Type type, const QString &string = "")
	: nex(prev->nex), typ(type), str(string) { prev->nex = this; }

    void appendChar( QChar ch ) { str += ch; }
    void appendString( const QString& string ) { str += string; }
    void chopString() { str.truncate( str.length() - 1 ); }
    void setString(const QString &string) { str = string; }
    Atom *next() { return nex; }
    void setNext( Atom *newNext ) { nex = newNext; }

    const Atom *next() const { return nex; }
    Type type() const { return typ; }
    QString typeString() const;
    const QString& string() const { return str; }

private:
    Atom *nex;
    Type typ;
    QString str;
};

#define ATOM_FORMATTING_BOLD            "bold"
#define ATOM_FORMATTING_INDEX           "index"
#define ATOM_FORMATTING_ITALIC          "italic"
#define ATOM_FORMATTING_LINK            "link"
#define ATOM_FORMATTING_PARAMETER       "parameter"
#define ATOM_FORMATTING_SUBSCRIPT       "subscript"
#define ATOM_FORMATTING_SUPERSCRIPT     "superscript"
#define ATOM_FORMATTING_TELETYPE        "teletype"
#define ATOM_FORMATTING_UNDERLINE       "underline"

#define ATOM_LIST_BULLET                "bullet"
#define ATOM_LIST_TAG                   "tag"
#define ATOM_LIST_VALUE                 "value"
#define ATOM_LIST_LOWERALPHA            "loweralpha"
#define ATOM_LIST_LOWERROMAN            "lowerroman"
#define ATOM_LIST_NUMERIC               "numeric"
#define ATOM_LIST_UPPERALPHA            "upperalpha"
#define ATOM_LIST_UPPERROMAN            "upperroman"

#endif
