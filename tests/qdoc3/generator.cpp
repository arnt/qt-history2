/*
  generator.cpp
*/

#include "codemarker.h"
#include "doc.h"
#include "generator.h"
#include "messages.h"
#include "node.h"
#include "separator.h"

QValueList<Generator *> Generator::generators;

Generator::Generator()
    : amp( "&amp;" ), lt( "&lt;" ), gt( "&gt;" ), quot( "&quot;" ),
      tag( "</?@[^>]*>" )
{
    generators.prepend( this );
}

Generator::~Generator()
{
    generators.remove( this );
}

Generator *Generator::generatorForFormat( const QString& format )
{
    QValueList<Generator *>::ConstIterator g = generators.begin();
    while ( g != generators.end() ) {
	if ( (*g)->format() == format )
	    return *g;
	++g;
    }
    return 0;
}

void Generator::startText( const Node * /* relative */,
			   CodeMarker * /* marker */ )
{
}

void Generator::endText( const Node * /* relative */,
			 CodeMarker * /* marker */ )
{
}

void Generator::generateAtom( const Atom * /* atom */,
			      const Node * /* relative */,
			      CodeMarker * /* marker */ )
{
}

void Generator::generateNamespaceNode( const NamespaceNode * /* namespasse */,
				       CodeMarker * /* marker */ )
{
}

void Generator::generateClassNode( const ClassNode * /* classe */,
				   CodeMarker * /* marker */ )
{
}

void Generator::generateFakeNode( const FakeNode * /* fake */,
				  CodeMarker * /* marker */ )
{
}

void Generator::generateText( const Text& text, const Node *relative,
			      CodeMarker *marker )
{
    const Atom *atom = text.firstAtom();
    if ( atom != 0 ) {
	startText( relative, marker );
	while ( atom != 0 ) {
	    generateAtom( atom, relative, marker );
	    atom = atom->next();
	}
	endText( relative, marker );
    }
}

void Generator::generateBody( const Node *node, CodeMarker *marker )
{
    if ( node->status() != Node::Approved )
	generateStatus( node, marker );
    if ( node->type() == Node::Function ) {
	const FunctionNode *func = (const FunctionNode *) node;
	if ( func->isOverload() )
	    generateOverload( node, marker );
    }

    generateText( node->doc().body(), node, marker );

    if ( node->type() == Node::Function ) {
	const FunctionNode *func = (const FunctionNode *) node;
	if ( func->reimplementedFrom() != 0 )
	    generateReimplementedFrom( func, marker );
	if ( !func->reimplementedBy().isEmpty() )
	    generateReimplementedBy( func, marker );
    }
}

void Generator::generateAlsoList( const Node *node, CodeMarker *marker )
{
    QValueList<Text>::ConstIterator a;
    int index;

    if ( node->doc().alsoList() != 0 && !node->doc().alsoList()->isEmpty() ) {
	Text text;
	text << Atom::ParagraphLeft << "See also ";

	a = node->doc().alsoList()->begin();
	index = 0;
        while ( a != node->doc().alsoList()->end() ) {
	    text << *a << separator( index++, node->doc().alsoList()->count() );
            ++a;
        }
        text << Atom::ParagraphRight;
	generateText( text, node, marker );
    }
}

void Generator::generateInherits( const ClassNode *classe,
				  CodeMarker *marker )
{
    QValueList<RelatedClass>::ConstIterator r;
    int index;

    if ( !classe->baseClasses().isEmpty() ) {
	Text text;
	text << Atom::ParagraphLeft << "Inherits ";

	r = classe->baseClasses().begin();
	index = 0;
	while ( r != classe->baseClasses().end() ) {
	    text << Atom( Atom::C,
			  marker->markedUpFullName((*r).node, classe) );
	    if ( (*r).access == Node::Protected ) {
		text << " (protected)";
	    } else if ( (*r).access == Node::Private ) {
		text << " (private)";
	    }
	    text << separator( index++, classe->baseClasses().count() );
	    ++r;
	}
	text << Atom::ParagraphRight;
	generateText( text, classe, marker );
    }
}

void Generator::generateInheritedBy( const ClassNode *classe,
				     CodeMarker *marker )
{
    QValueList<RelatedClass>::ConstIterator r;
    int index;

    if ( !classe->derivedClasses().isEmpty() ) {
	Text text;
	text << Atom::ParagraphLeft << "Inherited by ";

	r = classe->derivedClasses().begin();
	index = 0;
	while ( r != classe->derivedClasses().end() ) {
	    text << Atom( Atom::C,
			  marker->markedUpFullName((*r).node, classe) )
		     << separator( index++, classe->derivedClasses().count() );
	    ++r;
	}
	text << Atom::ParagraphRight;
	generateText( text, classe, marker );
    }
}

QString Generator::indent( int level, const QString& markedCode )
{
    if ( level == 0 )
	return markedCode;

    QString t;
    int column = 0;

    int i = 0;
    while ( i < (int) markedCode.length() ) {
	if ( markedCode[i] == '<' ) {
	    while ( i < (int) markedCode.length() ) {
		t += markedCode[i++];
		if ( markedCode[i - 1] == '>' )
		    break;
	    }
	} else {
	    if ( markedCode[i] == '\n' ) {
		column = 0;
	    } else {
		if ( column == 0 ) {
		    for ( int j = 0; j < level; j++ )
			t += ' ';
		}
		column++;
	    } 
	    t += markedCode[i++];
	}
    }
    return t;
}

QString Generator::plainCode( const QString& markedCode )
{
    QString t = markedCode;
    t.replace( tag, "" );
    t.replace( quot, "\"" );
    t.replace( gt, ">" );
    t.replace( lt, "<" );
    t.replace( amp, "&" );
    return t;
}

QString Generator::typeString( const Node *node )
{
    switch ( node->type() ) {
    case Node::Namespace:
	return "namespace";
    case Node::Class:
	return "class";
    case Node::Fake:
    default:
	return "documentation";
    case Node::Enum:
	return "enum";
    case Node::Typedef:
	return "typedef";
    case Node::Function:
	return "function";
    case Node::Property:
	return "property";
    }
}

Text Generator::sectionHeading( const Atom *sectionLeft )
{
    if ( sectionLeft != 0 ) {
	const Atom *begin = sectionLeft;
	while ( begin != 0 && begin->type() != Atom::SectionHeadingLeft )
	    begin = begin->next();
	if ( begin != 0 )
	    begin = begin->next();

	const Atom *end = begin;
	while ( end != 0 && end->type() != Atom::SectionHeadingRight )
	    end = end->next();

	if ( end != 0 )
	    return Text::subText( begin, end );
    }
    return Text();
}

void Generator::unknownAtom( const Atom *atom )
{
    Messages::internalError( Qdoc::tr("Unknown atom type '%1' in %2 generator")
			     .arg(atom->typeString()).arg(format()) );
}

void Generator::generateStatus( const Node *node, CodeMarker *marker )
{
    Text text;
    switch ( node->status() ) {
    case Node::Approved:
	break;
    case Node::Preliminary:
	text << Atom::ParagraphLeft << Atom( Atom::FormatLeft, "bold" )
	     << "This " << typeString( node )
	     << " is under development and is subject to change."
	     << Atom( Atom::FormatRight, "bold" ) << Atom::ParagraphRight;
	break;
    case Node::Deprecated:
	text << Atom::ParagraphLeft << Atom( Atom::FormatLeft, "bold" )
	     << "This " << typeString( node ) << " is deprecated."
	     << Atom( Atom::FormatRight, "bold" ) << Atom::ParagraphRight;
	break;
    case Node::Obsolete:
	text << Atom::ParagraphLeft << Atom( Atom::FormatLeft, "bold" )
	     << "This " << typeString( node ) << " is obsolete."
	     << Atom( Atom::FormatRight, "bold" )
	     << " It is provided to keep old source code working.  We strongly"
	     << " advise against using it in new code." << Atom::ParagraphRight;
    }
    generateText( text, node, marker );
}

void Generator::generateOverload( const Node *node, CodeMarker *marker )
{
    Text text;
    text << Atom::ParagraphLeft
	 << "This is an overloaded member function, provided for convenience."
	 << " It behaves essentially like the above function."
	 << Atom::ParagraphRight;
    generateText( text, node, marker );
}

void Generator::generateReimplementedFrom( const FunctionNode *func,
					   CodeMarker *marker )
{
    if ( func->reimplementedFrom() != 0 ) {
	const FunctionNode *from = func->reimplementedFrom();
	Text text;
	text << Atom::ParagraphLeft << "Reimplemented from "
	     << Atom( Atom::LinkNode, CodeMarker::stringForNode(from) )
	     << Atom( Atom::FormatLeft, "link" )
	     << Atom( Atom::C, marker->markedUpFullName(from->parent(), func) )
	     << Atom( Atom::FormatRight, "link" ) << "."
	     << Atom::ParagraphRight;
	generateText( text, func, marker );
    }
}

void Generator::generateReimplementedBy( const FunctionNode *func,
					 CodeMarker *marker )
{
    QValueList<FunctionNode *>::ConstIterator r;
    int index;

    Text text;
    text << Atom::ParagraphLeft << "Reimplemented by";

    r = func->reimplementedBy().begin();
    index = 0;
    while ( r != func->reimplementedBy().end() ) {
	text << Atom( Atom::C, marker->markedUpFullName(*r, func) )
	     << separator( index++, func->reimplementedBy().count() );
	++r;
    }
    text << Atom::ParagraphRight;
    generateText( text, func, marker );
}
