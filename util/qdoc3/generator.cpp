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
#include "quoter.h"
#include "separator.h"
#include "tokenizer.h"

QList<Generator *> Generator::generators;
QMap<QString, QMap<QString, QString> > Generator::fmtLeftMaps;
QMap<QString, QMap<QString, QString> > Generator::fmtRightMaps;
QMap<QString, QStringList> Generator::imgFileExts;
QStringList Generator::imageFiles;
QStringList Generator::imageDirs;
QString Generator::outDir;

static Text stockLink(const QString &target)
{
    return Text() << Atom(Atom::Link, target) << Atom(Atom::FormattingLeft, ATOM_FORMATTING_LINK)
		  << target << Atom(Atom::FormattingRight, ATOM_FORMATTING_LINK);
}

Generator::Generator()
    : amp("&amp;"), lt("&lt;"), gt("&gt;"), quot("&quot;"), tag("</?@[^>]*>")
{
    generators.prepend(this);
}

Generator::~Generator()
{
    generators.removeAll(this);
}

void Generator::initializeGenerator(const Config & /* config */)
{
}

void Generator::terminateGenerator()
{
}

void Generator::initialize(const Config &config)
{
    Set<QString> outputFormats = config.getStringSet(CONFIG_OUTPUTFORMATS);
    if ( !outputFormats.isEmpty() ) {
	outDir = config.getString(CONFIG_OUTPUTDIR);
	if ( outDir.isEmpty() )
	    config.lastLocation().fatal(tr("No output directory specified in configuration file"));
	QDir dirInfo;
	if ( dirInfo.exists(outDir) ) {
	    if ( !Config::removeDirContents(outDir) )
		config.lastLocation().error(tr("Cannot empty output directory '%1'").arg(outDir));
	} else {
	    if ( !dirInfo.mkdir(outDir) )
		config.lastLocation().fatal(tr("Cannot create output directory '%1'").arg(outDir));
	}
    }

    imageFiles = config.getStringList(CONFIG_IMAGES);
    imageDirs = config.getStringList(CONFIG_IMAGEDIRS);

    QString imagesDotFileExtensions = CONFIG_IMAGES + Config::dot + CONFIG_FILEEXTENSIONS;
    Set<QString> formats = config.subVars( imagesDotFileExtensions );
    Set<QString>::ConstIterator f = formats.begin();
    while ( f != formats.end() ) {
	imgFileExts[*f] = config.getStringList(imagesDotFileExtensions + Config::dot + *f);
	++f;
    }

    QList<Generator *>::ConstIterator g = generators.begin();
    while (g != generators.end()) {
	(*g)->initializeGenerator(config);
        QStringList extraImages = config.getStringList(CONFIG_EXTRAIMAGES + Config::dot
						       + (*g)->format());
	QStringList::ConstIterator e = extraImages.begin();
        while (e != extraImages.end()) {
	    QString userFriendlyFilePath;
	    QString filePath = Config::findFile(config.lastLocation(), imageFiles, imageDirs, *e,
						imgFileExts[(*g)->format()], userFriendlyFilePath);
	    Config::copyFile(config.lastLocation(), filePath, userFriendlyFilePath,
			     (*g)->outputDir());
	    ++e;
	}
	++g;
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
		int numOccs = def.count("\1");

		if ( numParams != 1 ) {
		    config.lastLocation().warning(tr("Formatting '%1' must have exactly one"
						     " parameter (found %2)")
						 .arg(*n).arg(numParams));
		} else if ( numOccs > 1 ) {
		    config.lastLocation().fatal(tr("Formatting '%1' must contain exactly one"
						    " occurrence of '\\1' (found %2)")
						.arg(*n).arg(numOccs));
		} else {
		    int paramPos = def.indexOf( "\1" );
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
    QList<Generator *>::ConstIterator g = generators.begin();
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
    QList<Generator *>::ConstIterator g = generators.begin();
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

void Generator::generateClassLikeNode(const InnerNode * /* classe */, CodeMarker * /* marker */)
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
#if 1
// ###
if (node->name().contains("foo"))
node->doc().body().dump();
#endif

    if ( node->type() == Node::Function ) {
	const FunctionNode *func = (const FunctionNode *) node;
	if ( func->isOverload() && func->metaness() != FunctionNode::Ctor )
	    generateOverload( node, marker );
    }

    if (node->doc().isEmpty()) {
	node->location().warning(tr("No documentation for '%1'").arg(marker->plainFullName(node)));
    } else {
        generateText(node->doc().body(), node, marker);

        if ( node->type() == Node::Enum ) {
	    const EnumNode *enume = (const EnumNode *) node;

	    Set<QString> definedItems;
	    QList<EnumItem>::ConstIterator it = enume->items().begin();
	    while ( it != enume->items().end() ) {
	        definedItems.insert( (*it).name() );
	        ++it;
	    }

	    Set<QString> documentedItems = enume->doc().enumItemNames();
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

		        node->doc().location().warning(tr("No such enum item '%1'").arg(*a),
						       details);
		    } else if ( !documentedItems.contains(*a) ) {
		        node->doc().location().warning(tr("Undocumented enum item '%1'").arg(*a));
		    }
		    ++a;
	        }
	    }
        } else if ( node->type() == Node::Function ) {
	    const FunctionNode *func = static_cast<const FunctionNode *>(node);

	    Set<QString> definedParams;
	    QList<Parameter>::ConstIterator p = func->parameters().begin();
	    while (p != func->parameters().end()) {
	        if ((*p).name().isEmpty() && (*p).leftType() != QLatin1String("...")
			&& func->name() != QLatin1String("operator++")
			&& func->name() != QLatin1String("operator--")) {
		    node->doc().location().warning(tr("Missing parameter name"));
	        } else {
		    definedParams.insert( (*p).name() );
	        }
	        ++p;
	    }

	    Set<QString> documentedParams = func->doc().parameterNames();
	    Set<QString> allParams = reunion( definedParams, documentedParams );
	    if (allParams.count() > definedParams.count()
		    || allParams.count() > documentedParams.count()) {
	        Set<QString>::ConstIterator a = allParams.begin();
	        while (a != allParams.end()) {
		    if (!definedParams.contains(*a)) {
		        QString details;
		        QString best = nearestName(*a, definedParams);
		        if ( !best.isEmpty() )
			    details = tr("Maybe you meant '%1'?").arg(best);

		        node->doc().location().warning(tr("No such parameter '%1'").arg(*a),
						       details);
		    } else if ( !(*a).isEmpty() && !documentedParams.contains(*a) ) {
			bool needWarning = (func->status() > Node::Obsolete);
			if (func->overloadNumber() > 1) {
			    FunctionNode *primaryFunc =
				    func->parent()->findFunctionNode(func->name());
			    if (primaryFunc) {
				foreach (Parameter param, primaryFunc->parameters()) {
				    if (param.name() == *a) {
                                        needWarning = false;
					break;
				    }
				}
			    }
                        }
                        if (needWarning)
			    node->doc().location().warning(tr("Undocumented parameter '%1'")
							   .arg(*a));
		    }
		    ++a;
	        }
	    }

	    if ( func->reimplementedFrom() != 0 )
	        generateReimplementedFrom( func, marker );
        } else if (node->type() == Node::Fake) {
            const FakeNode *fake = static_cast<const FakeNode *>(node);
            if (fake->subType() == FakeNode::File) {
                Text text;
                Quoter quoter;
                Doc::quoteFromFile(fake->location(), quoter, fake->name());
                text << Atom(Atom::Code, quoter.quoteTo(fake->location(), "", ""));
                generateText(text, fake, marker);
            }
        }
    }
}

void Generator::generateAlsoList( const Node *node, CodeMarker *marker )
{
    QList<Text>::ConstIterator a;
    int index;

    if ( !node->doc().alsoList().isEmpty() ) {
	Text text;
	text << Atom::ParaLeft << "See also ";

	a = node->doc().alsoList().begin();
	index = 0;
        while ( a != node->doc().alsoList().end() ) {
	    text << *a << separator( index++, node->doc().alsoList().count() );
            ++a;
        }
        text << Atom::ParaRight;
	generateText( text, node, marker );
    }
}

void Generator::generateInherits(const ClassNode *classe, CodeMarker *marker)
{
    QList<RelatedClass>::ConstIterator r;
    int index;

    if ( !classe->baseClasses().isEmpty() ) {
	Text text;
	text << Atom::ParaLeft << "Inherits ";

	r = classe->baseClasses().begin();
	index = 0;
	while ( r != classe->baseClasses().end() ) {
            text << Atom(Atom::LinkNode, CodeMarker::stringForNode((*r).node))
	         << Atom(Atom::FormattingLeft, ATOM_FORMATTING_LINK)
	         << Atom(Atom::String, (*r).dataTypeWithTemplateArgs)
	         << Atom(Atom::FormattingRight, ATOM_FORMATTING_LINK);

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
    QList<RelatedClass>::ConstIterator r;
    int index;

    if ( !classe->derivedClasses().isEmpty() ) {
	Text text;
	text << Atom::ParaLeft << "Inherited by ";

	r = classe->derivedClasses().begin();
	index = 0;
	while ( r != classe->derivedClasses().end() ) {
	    appendFullName( text, (*r).node, classe, marker );
	    text << separator( index++, classe->derivedClasses().count() );
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
    QString filePath = Config::findFile(location, imageFiles, imageDirs, fileBase,
					imgFileExts[format()], userFriendlyFilePath);
    if (filePath.isEmpty())
	return "";

    return Config::copyFile(location, filePath, userFriendlyFilePath, outputDir());
}

void Generator::setImageFileExtensions( const QStringList& extensions )
{
    imgFileExts[format()] = extensions;
}

void Generator::unknownAtom( const Atom *atom )
{
    Location::internalError( tr("unknown atom type '%1' in %2 generator")
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

QString Generator::trimmedTrailing(const QString &string)
{
    QString trimmed = string;
    while (trimmed.length() > 0 && trimmed[trimmed.length() - 1].isSpace())
	trimmed.truncate(trimmed.length() - 1);
    return trimmed;
}

void Generator::generateStatus( const Node *node, CodeMarker *marker )
{
    Text text;

    switch ( node->status() ) {
    case Node::Commendable:
    case Node::Main:
	break;
    case Node::Preliminary:
	text << Atom::ParaLeft << Atom( Atom::FormattingLeft, ATOM_FORMATTING_BOLD ) << "This "
	     << typeString( node ) << " is under development and is subject to change."
	     << Atom( Atom::FormattingRight, ATOM_FORMATTING_BOLD ) << Atom::ParaRight;
	break;
    case Node::Deprecated:
	text << Atom::ParaLeft << Atom( Atom::FormattingLeft, ATOM_FORMATTING_BOLD ) << "This "
	     << typeString( node ) << " is deprecated."
             << Atom( Atom::FormattingRight, ATOM_FORMATTING_BOLD ) << Atom::ParaRight;
	break;
    case Node::Obsolete:
        if (node->isInnerNode()) {
	    text << Atom::ParaLeft << Atom( Atom::FormattingLeft, ATOM_FORMATTING_BOLD ) << "This "
	         << typeString( node ) << " is obsolete."
	         << Atom( Atom::FormattingRight, ATOM_FORMATTING_BOLD )
	         << " It is provided to keep old source code working. We strongly advise against "
                 << "using it in new code." << Atom::ParaRight;
        }
        break;
    case Node::Compat:
        if (node->isInnerNode()) {
	    text << Atom::ParaLeft << Atom( Atom::FormattingLeft, ATOM_FORMATTING_BOLD ) << "This "
	         << typeString( node ) << " is part of the Qt 3 compatibility library."
	         << Atom( Atom::FormattingRight, ATOM_FORMATTING_BOLD )
	         << " It is provided to keep old source code working. We strongly advise against "
                 << "using it in new code." << Atom::ParaRight;
        }
    }
    generateText(text, node, marker);
}

void Generator::generateThreadSafeness(const Node *node, CodeMarker *marker)
{
    Text text;
    Text theStockLink;
    Node::ThreadSafeness parent = node->parent()->inheritedThreadSafeness();

    switch (node->threadSafeness()) {
    case Node::UnspecifiedSafeness:
	break;
    case Node::NonReentrant:
	text << Atom::ParaLeft << Atom(Atom::FormattingLeft, ATOM_FORMATTING_BOLD) << "Warning:"
             << Atom(Atom::FormattingRight, ATOM_FORMATTING_BOLD) << " This "
             << typeString(node) << " is not " << stockLink("reentrant") << "." << Atom::ParaRight;
	break;
    case Node::Reentrant:
    case Node::ThreadSafe:
	text << Atom::ParaLeft << Atom(Atom::FormattingLeft, ATOM_FORMATTING_BOLD);
        if (parent == Node::ThreadSafe) {
	    text << "Warning:";
	} else {
	    text << "Note:";
        }
        text << Atom(Atom::FormattingRight, ATOM_FORMATTING_BOLD) << " ";

	if (node->threadSafeness() == Node::ThreadSafe)
	    theStockLink = stockLink("thread-safe");
	else
	    theStockLink = stockLink("reentrant");

        if (node->isInnerNode()) {
	    const InnerNode *innerNode = static_cast<const InnerNode *>(node);
	    text << "All the functions in this " << typeString(node) << " are "
		 << theStockLink;

	    NodeList except;
            NodeList::ConstIterator c = innerNode->childNodes().begin();
            while (c != innerNode->childNodes().end()) {
		if ((*c)->threadSafeness() != Node::UnspecifiedSafeness)
		    except.append(*c);
		++c;
            }
	    if (except.isEmpty()) {
		text << ".";
	    } else {
		text << ", except ";

                NodeList::ConstIterator e = except.begin();
                int index = 0;
                while (e != except.end()) {
		    appendFullName(text, *e, innerNode, marker);
                    text << separator(index++, except.count());
		    ++e;
                }
            }
	} else {
            text << typeString(node) << " is " << theStockLink << ".";
        }
        text << Atom::ParaRight;
    }
    generateText(text, node, marker);
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
	text << Atom::ParaLeft << "Reimplemented from ";
	appendFullName( text, from->parent(), func, marker, from );
	text << "." << Atom::ParaRight;
	generateText( text, func, marker );
    }
}

const Atom *Generator::generateAtomList(const Atom *atom, const Node *relative, CodeMarker *marker,
					bool generate, int &numAtoms)
{
    while (atom) {
	if (atom->type() == Atom::FormatIf) {
	    int numAtoms0 = numAtoms;
	    bool rightFormat = ( atom->string() == format() );
	    atom = generateAtomList(atom->next(), relative, marker, generate && rightFormat,
				    numAtoms);
	    if (!atom)
		return 0;

	    if (atom->type() == Atom::FormatElse) {
		++numAtoms;
		atom = generateAtomList( atom->next(), relative, marker,
					 generate && !rightFormat, numAtoms );
		if (!atom)
		    return 0;
	    }

	    if (atom->type() == Atom::FormatEndif) {
		if (generate && numAtoms0 == numAtoms) {
		    relative->location().warning(tr("Output format %1 not handled").arg(format()));
		    Atom unhandledFormatAtom(Atom::UnhandledFormat, format());
		    generateAtomList(&unhandledFormatAtom, relative, marker, generate, numAtoms);
		}
		atom = atom->next();
	    }
	} else if (atom->type() == Atom::FormatElse || atom->type() == Atom::FormatEndif) {
	    return atom;
	} else {
	    int n = 1;
	    if (generate) {
		n += generateAtom(atom, relative, marker);
		numAtoms += n;
	    }
	    while (n-- > 0)
		atom = atom->next();
	}
    }
    return 0;
}

void Generator::appendFullName( Text& text, const Node *apparentNode,
				const Node *relative, CodeMarker *marker,
				const Node *actualNode )
{
    if ( actualNode == 0 )
	actualNode = apparentNode;
    text << Atom(Atom::LinkNode, CodeMarker::stringForNode(actualNode))
	 << Atom(Atom::FormattingLeft, ATOM_FORMATTING_LINK)
	 << Atom(Atom::String, marker->plainFullName(apparentNode, relative))
	 << Atom(Atom::FormattingRight, ATOM_FORMATTING_LINK);
}
