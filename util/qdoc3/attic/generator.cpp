/*
  generator.cpp
*/

#include "codemarker.h"
#include "doc.h"
#include "generator.h"
#include "molecule.h"
#include "node.h"
#include "separator.h"

Generator::Generator()
    : amp( "&amp;" ), lt( "&lt;" ), gt( "&gt;" ), quot( "&quot;" ),
      tag( "</?@[^>]*>" )
{
}

Generator::~Generator()
{
}

void Generator::startMolecule( const Node * /* relative */,
			       const CodeMarker * /* marker */ )
{
}

void Generator::endMolecule( const Node * /* relative */,
			     const CodeMarker * /* marker */ )
{
}

void Generator::generateAtom( const Atom * /* atom */,
			      const Node * /* relative */,
			      const CodeMarker * /* marker */ )
{
}

void Generator::generateNamespaceNode( const NamespaceNode * /* namespasse */,
				       const CodeMarker * /* marker */ )
{
}

void Generator::generateClassNode( const ClassNode * /* classe */,
				   const CodeMarker * /* marker */ )
{
}

void Generator::generateFakeNode( const FakeNode * /* fake */,
				  const CodeMarker * /* marker */ )
{
}

void Generator::generateMolecule( const Molecule& molecule,
				  const Node *relative,
				  const CodeMarker *marker )
{
    const Atom *atom = molecule.firstAtom();
    if ( atom != 0 ) {
	startMolecule( relative, marker );
	while ( atom != 0 ) {
	    generateAtom( atom, relative, marker );
	    atom = atom->next();
	}
	endMolecule( relative, marker );
    }
}

void Generator::generateBody( const Node *node, const CodeMarker *marker )
{
    if ( node->status() != Node::Approved )
	generateStatus( node, marker );
    if ( node->type() == Node::Function ) {
	const FunctionNode *func = (const FunctionNode *) node;
	if ( func->isOverload() )
	    generateOverload( node, marker );
    }

    generateMolecule( node->doc().body(), node, marker );

    if ( node->type() == Node::Function ) {
	const FunctionNode *func = (const FunctionNode *) node;
	if ( func->reimplementedFrom() != 0 )
	    generateReimplementedFrom( func, marker );
	if ( !func->reimplementedBy().isEmpty() )
	    generateReimplementedBy( func, marker );
    }
}

void Generator::generateAlsoList( const Node *node, const CodeMarker *marker )
{
    QValueList<Molecule>::ConstIterator a;
    int index;

    if ( node->doc().alsoList() != 0 && !node->doc().alsoList()->isEmpty() ) {
	Molecule molecule;
	molecule << Atom::ParagraphBegin << "See also ";

	a = node->doc().alsoList()->begin();
	index = 0;
        while ( a != node->doc().alsoList()->end() ) {
	    molecule << *a << separator( index++,
					 node->doc().alsoList()->count() );
            ++a;
        }
        molecule << Atom::ParagraphEnd;
	generateMolecule( molecule, node, marker );
    }
}

void Generator::generateInherits( const ClassNode *classe,
				  const CodeMarker *marker )
{
    QValueList<RelatedClass>::ConstIterator r;
    int index;

    if ( !classe->baseClasses().isEmpty() ) {
	Molecule molecule;
	molecule << Atom::ParagraphBegin << "Inherits ";

	r = classe->baseClasses().begin();
	index = 0;
	while ( r != classe->baseClasses().end() ) {
	    molecule << Atom( Atom::C,
			      marker->markedUpFullName((*r).node, classe) );
	    if ( (*r).access == Node::Protected ) {
		molecule << " (protected)";
	    } else if ( (*r).access == Node::Private ) {
		molecule << " (private)";
	    }
	    molecule << separator( index++, classe->baseClasses().count() );
	    ++r;
	}
	molecule << Atom::ParagraphEnd;
	generateMolecule( molecule, classe, marker );
    }
}

void Generator::generateInheritedBy( const ClassNode *classe,
				     const CodeMarker *marker )
{
    QValueList<RelatedClass>::ConstIterator r;
    int index;

    if ( !classe->derivedClasses().isEmpty() ) {
	Molecule molecule;
	molecule << Atom::ParagraphBegin << "Inherited by ";

	r = classe->derivedClasses().begin();
	index = 0;
	while ( r != classe->derivedClasses().end() ) {
	    molecule << Atom( Atom::C,
			      marker->markedUpFullName((*r).node, classe) )
		     << separator( index++, classe->derivedClasses().count() );
	    ++r;
	}
	molecule << Atom::ParagraphEnd;
	generateMolecule( molecule, classe, marker );
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

Molecule Generator::sectionHeading( const Atom *sectionBegin )
{
    if ( sectionBegin != 0 ) {
	const Atom *begin = sectionBegin;
	while ( begin != 0 && begin->type() != Atom::SectionHeadingBegin )
	    begin = begin->next();
	if ( begin != 0 )
	    begin = begin->next();

	const Atom *end = begin;
	while ( end != 0 && end->type() != Atom::SectionHeadingEnd )
	    end = end->next();

	if ( end != 0 )
	    return Molecule::subMolecule( begin, end );
    }
    return Molecule();
}

void Generator::generateStatus( const Node *node, const CodeMarker *marker )
{
    Molecule molecule;
    switch ( node->status() ) {
    case Node::Approved:
	break;
    case Node::Preliminary:
	molecule << Atom::ParagraphBegin << Atom( Atom::FormatBegin, "bold" )
		 << "This " << typeString( node )
		 << " is under development and is subject to change."
		 << Atom( Atom::FormatEnd, "bold" ) << Atom::ParagraphEnd;
	break;
    case Node::Deprecated:
	molecule << Atom::ParagraphBegin << Atom( Atom::FormatBegin, "bold" )
		 << "This " << typeString( node ) << " is deprecated."
		 << Atom( Atom::FormatEnd, "bold" ) << Atom::ParagraphEnd;
	break;
    case Node::Obsolete:
	molecule << Atom::ParagraphBegin << Atom( Atom::FormatBegin, "bold" )
		 << "This " << typeString( node ) << " is obsolete."
		 << Atom( Atom::FormatEnd, "bold" )
		 << " It is provided to keep old source code working.  We"
		    " strongly advise against using it in new code."
		 << Atom::ParagraphEnd;
    }
    generateMolecule( molecule, node, marker );
}

void Generator::generateOverload( const Node *node, const CodeMarker *marker )
{
    Molecule molecule;
    molecule << Atom::ParagraphBegin
	     << "This is an overloaded member function, provided for"
		" convenience. It behaves essentially like the above function."
	     << Atom::ParagraphEnd;
    generateMolecule( molecule, node, marker );
}

void Generator::generateReimplementedFrom( const FunctionNode *func,
					   const CodeMarker *marker )
{
    if ( func->reimplementedFrom() != 0 ) {
	const FunctionNode *from = func->reimplementedFrom();
	Molecule molecule;
	molecule << Atom::ParagraphBegin << "Reimplemented from "
		 << Atom( Atom::LinkNode, CodeMarker::stringForNode(from) )
		 << Atom( Atom::FormatBegin, "link" )
		 << Atom( Atom::C, marker->markedUpFullName(from->parent(),
							    func) )
		 << Atom( Atom::FormatEnd, "link" )
		 << "."
		 << Atom::ParagraphEnd;
	generateMolecule( molecule, func, marker );
    }
}

void Generator::generateReimplementedBy( const FunctionNode *func,
					 const CodeMarker *marker )
{
    QValueList<FunctionNode *>::ConstIterator r;
    int index;

    Molecule molecule;
    molecule << Atom::ParagraphBegin << "Reimplemented by";

    r = func->reimplementedBy().begin();
    index = 0;
    while ( r != func->reimplementedBy().end() ) {
	molecule << Atom( Atom::C, marker->markedUpFullName(*r, func) )
		 << separator( index++, func->reimplementedBy().count() );
	++r;
    }
    molecule << Atom::ParagraphEnd;
    generateMolecule( molecule, func, marker );
}
