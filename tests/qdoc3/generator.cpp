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

void Generator::generateDoc( const Doc& doc, const Node *node,
			     const CodeMarker *marker )
{
    if ( node->status() != Node::Approved )
	generateStatus( node, marker );
    if ( node->type() == Node::Function ) {
	const FunctionNode *func = (const FunctionNode *) node;
	if ( func->isOverload() )
	    generateOverload( node, marker );
    }

    generateMolecule( doc.molecule(), node, marker );

    if ( node->type() == Node::Function ) {
	const FunctionNode *func = (const FunctionNode *) node;
	if ( func->reimplementedFrom() != 0 )
	    generateReimplementedFrom( func, marker );
	if ( !func->reimplementedBy().isEmpty() )
	    generateReimplementedBy( func, marker );
    }
}

void Generator::generateAlso( const Doc& /* doc */, const Node * /* node */,
			      const CodeMarker * /* marker */ )
{
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

void Generator::generateStatus( const Node *node, const CodeMarker *marker )
{
    Molecule molecule;
    molecule << Atom::ParagraphBegin << "This member is obsolete."
	     << Atom::ParagraphEnd;
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
    Molecule molecule;
    molecule << Atom::ParagraphBegin << "Reimplemented from "
	     << Atom::ParagraphEnd;
    generateMolecule( molecule, func, marker );
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
