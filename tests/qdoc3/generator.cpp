/*
  generator.cpp
*/

#include <qdir.h>

#include "codemarker.h"
#include "config.h"
#include "doc.h"
#include "editdistance.h"
#include "generator.h"
#include "node.h"
#include "separator.h"

QValueList<Generator *> Generator::generators;
QMap<QString, QMap<QString, QString> > Generator::fmtLeftMaps;
QMap<QString, QMap<QString, QString> > Generator::fmtRightMaps;
QMap<QString, QStringList> Generator::imgFileExts;
QStringList Generator::imageFiles;
QStringList Generator::imageDirs;
QString Generator::outDir;

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

void Generator::initializeGenerator( const Config& /* config */ )
{
}

void Generator::terminateGenerator()
{
}

void Generator::initialize( const Config& config )
{
    outDir = config.getString( CONFIG_OUTPUTDIR );
    if ( outDir.isEmpty() )
	config.lastLocation().fatal( tr("No output directory specified in"
					" configuration file") );

    QDir dirInfo;
    if ( dirInfo.exists(outDir) ) {
	if ( !Config::removeDirContents(outDir) )
	    config.lastLocation().error( tr("Cannot empty output directory"
					    " '%1'")
					 .arg(outDir) );
    } else {
	if ( !dirInfo.mkdir(outDir) )
	    config.lastLocation().fatal( tr("Cannot create output directory"
					    " '%1'")
					 .arg(outDir) );
    }

    QValueList<Generator *>::ConstIterator g = generators.begin();
    while ( g != generators.end() ) {
	(*g)->initializeGenerator( config );
	++g;
    }

    imageFiles = config.getStringList( CONFIG_IMAGES );
    imageDirs = config.getStringList( CONFIG_IMAGEDIRS );

    QString imagesDotFileExtensions = CONFIG_IMAGES + Config::dot +
				      CONFIG_FILEEXTENSIONS;
    Set<QString> formats = config.subVars( imagesDotFileExtensions );
    Set<QString>::ConstIterator f = formats.begin();
    while ( f != formats.end() ) {
	imgFileExts[*f] = config.getStringList( imagesDotFileExtensions +
						Config::dot + *f );
	++f;
    }

    QRegExp secondParamAndAbove( "[\2-\7]" );
    Set<QString> formattingNames = config.subVars( CONFIG_FORMATTING );
    Set<QString>::ConstIterator n = formattingNames.begin();
    while ( n != formattingNames.end() ) {
	QString formattingDotName = CONFIG_FORMATTING + Config::dot + *n;

	Set<QString> formats = config.subVars( formattingDotName );
	Set<QString>::ConstIterator f = formats.begin();
	while ( f != formats.end() ) {
	    QString def = config.getString( formattingDotName + Config::dot +
					    *f );
	    if ( !def.isEmpty() ) {
		int numParams = Config::numParams( def );
		int numOccs = def.contains( "\1" );

		if ( numParams != 1 ) {
		    config.lastLocation().warning( tr("Formatting '%1' must"
						    " have exactly one"
						    " parameter (found %2)")
						 .arg(*n).arg(numParams) );
		} else if ( numOccs > 1 ) {
		    config.lastLocation().fatal( tr("Formatting '%1' must"
						    " contain exactly one"
						    " occurrence of '\\1'"
						    " (found %2)")
						 .arg(*n).arg(numOccs) );
		} else {
		    int paramPos = def.find( "\1" );
		    fmtLeftMaps[*f].insert( *n, def.left(paramPos) );
		    fmtRightMaps[*f].insert( *n, def.mid(paramPos + 1) );
		}
	    }
	    ++f;
	}
	++n;
    }
}

void Generator::terminate()
{
    QValueList<Generator *>::ConstIterator g = generators.begin();
    while ( g != generators.end() ) {
	(*g)->terminateGenerator();
	++g;
    }

    fmtLeftMaps.clear();
    fmtRightMaps.clear();
    imgFileExts.clear();
    imageFiles.clear();
    imageDirs.clear();
    outDir = "";
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

int Generator::generateAtom( const Atom * /* atom */,
			     const Node * /* relative */,
			     CodeMarker * /* marker */ )
{
    return 0;
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
    if ( text.firstAtom() != 0 ) {
	int numAtoms = 0;
	startText( relative, marker );
	generateAtomList( text.firstAtom(), relative, marker, TRUE, numAtoms );
	endText( relative, marker );
    }
}

void Generator::generateBody( const Node *node, CodeMarker *marker )
{
    if ( node->status() != Node::Commendable )
	generateStatus( node, marker );
    if ( node->type() == Node::Function ) {
	const FunctionNode *func = (const FunctionNode *) node;
	if ( func->isOverload() && func->metaness() != FunctionNode::Ctor )
	    generateOverload( node, marker );
    }

    if ( node->doc().isEmpty() ) {
	QString name = plainCode( marker->markedUpFullName(node, 0) );
	node->location().warning( tr("No documentation for '%1'").arg(name) );
    }
    generateText( node->doc().body(), node, marker );

    if ( node->type() == Node::Enum ) {
	const EnumNode *enume = (const EnumNode *) node;

	Set<QString> definedItems;
	QValueList<EnumItem>::ConstIterator it = enume->items().begin();
	while ( it != enume->items().end() ) {
	    definedItems.insert( (*it).name() );
	    ++it;
	}

	Set<QString> documentedItems;
	if ( enume->doc().enumItemNames() != 0 )
	    documentedItems = *enume->doc().enumItemNames();

	Set<QString> allItems = reunion( definedItems, documentedItems );
	if ( allItems.count() > definedItems.count() ||
	     allItems.count() > documentedItems.count() ) {
	    Set<QString>::ConstIterator a = allItems.begin();
	    while ( a != allItems.end() ) {
		if ( !definedItems.contains(*a) ) {
		    QString details;
		    QString best = nearestName( *a, definedItems );
		    if ( !best.isEmpty() && !documentedItems.contains(best) )
			details = tr( "Maybe you meant '%1'?" ).arg( best );

		    node->doc().location().warning( tr("No such enum item '%1'")
						    .arg(*a),
						    details );
		} else if ( !documentedItems.contains(*a) ) {
		    node->doc().location().warning( tr("Undocumented enum item"
						       " '%1'").arg(*a) );
		}
		++a;
	    }
	}
    } else if ( node->type() == Node::Function ) {
	const FunctionNode *func = (const FunctionNode *) node;

	Set<QString> definedParams;
	QValueList<Parameter>::ConstIterator p = func->parameters().begin();
	while ( p != func->parameters().end() ) {
	    if ( (*p).name().isEmpty() ) {
		node->location().warning( tr("Missing parameter name") );
	    } else {
		definedParams.insert( (*p).name() );
	    }
	    ++p;
	}

	Set<QString> documentedParams;
	if ( func->doc().parameterNames() != 0 )
	    documentedParams = *func->doc().parameterNames();

	Set<QString> allParams = reunion( definedParams, documentedParams );
	if ( allParams.count() > definedParams.count() ||
	     allParams.count() > documentedParams.count() ) {
	    Set<QString>::ConstIterator a = allParams.begin();
	    while ( a != allParams.end() ) {
		if ( !definedParams.contains(*a) ) {
		    QString details;
		    QString best = nearestName( *a, definedParams );
		    if ( !best.isEmpty() && !documentedParams.contains(best) )
			details = tr( "Maybe you meant '%1'?" ).arg( best );

		    node->doc().location().warning( tr("No such parameter '%1'")
						    .arg(*a),
						    details );
		} else if ( !documentedParams.contains(*a) ) {
		    node->doc().location().warning( tr("Undocumented parameter"
						       " '%1'").arg(*a) );
		}
		++a;
	    }
	}

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
	text << Atom::ParaLeft << "See also ";

	a = node->doc().alsoList()->begin();
	index = 0;
        while ( a != node->doc().alsoList()->end() ) {
	    text << *a << separator( index++, node->doc().alsoList()->count() );
            ++a;
        }
        text << Atom::ParaRight;
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
	text << Atom::ParaLeft << "Inherits ";

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
	text << Atom::ParaRight;
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
	text << Atom::ParaLeft << "Inherited by ";

	r = classe->derivedClasses().begin();
	index = 0;
	while ( r != classe->derivedClasses().end() ) {
	    text << Atom( Atom::C,
			  marker->markedUpFullName((*r).node, classe) )
		     << separator( index++, classe->derivedClasses().count() );
	    ++r;
	}
	text << Atom::ParaRight;
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

QString Generator::imageFileName( const Location& location,
				  const QString& fileBase )
{
    QString userFriendlyFilePath;
    QString filePath = Config::findFile( location, imageFiles, imageDirs,
					 fileBase, imgFileExts[format()],
					 userFriendlyFilePath );
    if ( filePath.isEmpty() )
	return "";

    return Config::copyFile( location, filePath, userFriendlyFilePath,
			     outputDir() );
}

void Generator::setImageFileExtensions( const QStringList& extensions )
{
    imgFileExts[format()] = extensions;
}

void Generator::unknownAtom( const Atom *atom )
{
    Location::internalError( tr("Unknown atom type '%1' in %2 generator")
			     .arg(atom->typeString()).arg(format()) );
}

bool Generator::matchAhead( const Atom *atom, Atom::Type expectedAtomType )
{
    return atom->next() != 0 && atom->next()->type() == expectedAtomType;
}

QMap<QString, QString>& Generator::formattingLeftMap()
{
    return fmtLeftMaps[format()];
}

QMap<QString, QString>& Generator::formattingRightMap()
{
    return fmtRightMaps[format()];
}

void Generator::generateStatus( const Node *node, CodeMarker *marker )
{
    Text text;
    switch ( node->status() ) {
    case Node::Commendable:
	break;
    case Node::Preliminary:
	text << Atom::ParaLeft
	     << Atom( Atom::FormattingLeft, ATOM_FORMATTING_BOLD ) << "This "
	     << typeString( node )
	     << " is under development and is subject to change."
	     << Atom( Atom::FormattingRight, ATOM_FORMATTING_BOLD )
	     << Atom::ParaRight;
	break;
    case Node::Deprecated:
	text << Atom::ParaLeft
	     << Atom( Atom::FormattingLeft, ATOM_FORMATTING_BOLD ) << "This "
	     << typeString( node ) << " is deprecated."
	     << Atom( Atom::FormattingRight, ATOM_FORMATTING_BOLD )
	     << Atom::ParaRight;
	break;
    case Node::Obsolete:
	text << Atom::ParaLeft
	     << Atom( Atom::FormattingLeft, ATOM_FORMATTING_BOLD ) << "This "
	     << typeString( node ) << " is obsolete."
	     << Atom( Atom::FormattingRight, ATOM_FORMATTING_BOLD )
	     << " It is provided to keep old source code working.  We strongly"
	     << " advise against using it in new code." << Atom::ParaRight;
    }
    generateText( text, node, marker );
}

void Generator::generateOverload( const Node *node, CodeMarker *marker )
{
    Text text;
    text << Atom::ParaLeft
	 << "This is an overloaded member function, provided for convenience."
	 << " It behaves essentially like the above function."
	 << Atom::ParaRight;
    generateText( text, node, marker );
}

void Generator::generateReimplementedFrom( const FunctionNode *func,
					   CodeMarker *marker )
{
    if ( func->reimplementedFrom() != 0 ) {
	const FunctionNode *from = func->reimplementedFrom();
	Text text;
	text << Atom::ParaLeft << "Reimplemented from "
	     << Atom( Atom::LinkNode, CodeMarker::stringForNode(from) )
	     << Atom( Atom::FormattingLeft, ATOM_FORMATTING_LINK )
	     << Atom( Atom::C, marker->markedUpFullName(from->parent(), func) )
	     << Atom( Atom::FormattingRight, ATOM_FORMATTING_LINK ) << "."
	     << Atom::ParaRight;
	generateText( text, func, marker );
    }
}

const Atom *Generator::generateAtomList( const Atom *atom, const Node *relative,
					 CodeMarker *marker, bool generate,
					 int& numAtoms )
{
    while ( atom != 0 ) {
	if ( atom->type() == Atom::FormatIf ) {
	    int numAtoms0 = numAtoms;
	    bool rightFormat = ( atom->string() == format() );
	    atom = generateAtomList( atom->next(), relative, marker,
				     generate && rightFormat, numAtoms );
	    if ( atom == 0 )
		return 0;

	    if ( atom->type() == Atom::FormatElse ) {
		atom = generateAtomList( atom->next(), relative, marker,
					 generate && !rightFormat, numAtoms );
		if ( atom == 0 )
		    return 0;
	    }

	    if ( atom->type() == Atom::FormatEndif ) {
		if ( generate && numAtoms0 == numAtoms ) {
		    relative->location().warning(
			    tr("Output format %1 not handled").arg(format()) );
		    Atom unhandledFormatAtom( Atom::UnhandledFormat, format() );
		    generateAtomList( &unhandledFormatAtom, relative, marker,
				      generate, numAtoms );
		}
		atom = atom->next();
	    }
	} else if ( atom->type() == Atom::FormatElse ||
		    atom->type() == Atom::FormatEndif ) {
	    return atom;
	} else {
	    int n = 1;
	    if ( generate ) {
		n += generateAtom( atom, relative, marker );
		numAtoms += n;
	    }
	    while ( n-- > 0 )
		atom = atom->next();
	}
    }
    return 0;
}

void Generator::generateReimplementedBy( const FunctionNode *func,
					 CodeMarker *marker )
{
    QValueList<FunctionNode *>::ConstIterator r;
    int index;

    Text text;
    text << Atom::ParaLeft << "Reimplemented by";

    r = func->reimplementedBy().begin();
    index = 0;
    while ( r != func->reimplementedBy().end() ) {
	text << Atom( Atom::C, marker->markedUpFullName(*r, func) )
	     << separator( index++, func->reimplementedBy().count() );
	++r;
    }
    text << Atom::ParaRight;
    generateText( text, func, marker );
}
