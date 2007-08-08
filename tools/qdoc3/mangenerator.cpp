/*
  mangenerator.cpp
*/

#include <qdatetime.h>
#include <qregexp.h>

#include "mangenerator.h"
#include "node.h"
#include "tree.h"

ManGenerator::ManGenerator()
{
    date = QDate::currentDate().toString( "d MMMM yyyy" );
}

ManGenerator::~ManGenerator()
{
}

QString ManGenerator::format()
{
    return "man";
}

int ManGenerator::generateAtom( const Atom *atom, const Node * /* relative */,
				CodeMarker * /* marker */ )
{
#if 0
    switch ( atom->type() ) {
    case Atom::AbstractBegin:
	break;
    case Atom::AbstractEnd:
	break;
    case Atom::Alias:
	break;
    case Atom::AliasArg:
	break;
    case Atom::BaseName:
	break;
    case Atom::BriefBegin:
	break;
    case Atom::BriefEnd:
	break;
    case Atom::C:
	break;
    case Atom::CaptionBegin:
	break;
    case Atom::CaptionEnd:
	break;
    case Atom::CitationBegin:
	break;
    case Atom::CitationEnd:
	break;
    case Atom::Code:
	break;
    case Atom::FootnoteBegin:
	break;
    case Atom::FootnoteEnd:
	break;
    case Atom::FormatBegin:
	break;
    case Atom::FormatEnd:
	break;
    case Atom::GeneratedList:
	break;
    case Atom::Image:
	break;
    case Atom::ImageText:
	break;
    case Atom::Link:
	break;
    case Atom::LinkNode:
	break;
    case Atom::ListBegin:
	break;
    case Atom::ListItemNumber:
	break;
    case Atom::ListItemBegin:
	out() << ".IP " << atom->string() << ".\n";
	break;
    case Atom::ListItemEnd:
	break;
    case Atom::ListEnd:
	break;
    case Atom::Nop:
	break;
    case Atom::ParaBegin:
	out() << ".PP\n";
	break;
    case Atom::ParaEnd:
	out() << "\n";
	break;
    case Atom::RawFormat:
	break;
    case Atom::RawString:
	break;
    case Atom::SectionBegin:
	break;
    case Atom::SectionEnd:
	break;
    case Atom::SectionHeadingBegin:
	break;
    case Atom::SectionHeadingEnd:
	break;
    case Atom::SidebarBegin:
	break;
    case Atom::SidebarEnd:
	break;
    case Atom::String:
	out() << protectTextLine( atom->string() );
	break;
    case Atom::TableBegin:
	break;
    case Atom::TableEnd:
	break;
    case Atom::TableOfContents:
	break;
    case Atom::Target:
	break;
    case Atom::UnknownCommand:
	;
    }
#endif
    unknownAtom( atom );
    return 0;
}

void ManGenerator::generateClassLikeNode( const InnerNode *classe,
				      CodeMarker *marker )
{
    generateHeader( classe->name() );
    out() << ".SH NAME\n"
	  << classe->name() << "\n"
          << ".SH SYNOPSYS\n";
    generateBody( classe, marker );
    generateFooter();
}

void ManGenerator::generateFakeNode( const FakeNode *fake, CodeMarker *marker )
{
    generateHeader( "foo" );
    generateBody( fake, marker );
    generateFooter();
}

QString ManGenerator::fileExtension(const Node * /* node */)
{
    return "3qt";
}

void ManGenerator::generateHeader( const QString& name )
{
    out() << ".TH " << protectArg( name )
	  << " " << protectArg( "3qt" )
	  << " " << protectArg( date )
	  << " " << protectArg( "Trolltech ASA" )
	  << " " << protectArg( "Qt Toolkit" ) << "\n";
}

void ManGenerator::generateFooter()
{
}

QString ManGenerator::protectArg( const QString& str )
{
    for ( int i = 0; i < (int) str.length(); i++ ) {
	if ( str[i] == ' ' || str[i].isSpace() ) {
	    QString quoted = str;
	    quoted.replace( "\"", "\"\"" );
	    return "\"" + quoted + "\"";
	}
    }
    return str;
}

QString ManGenerator::protectTextLine( const QString& str )
{
    QString t = str;
    if ( t.startsWith(".") || t.startsWith("'") )
	t.prepend( "\\&" );
    return t;
}
