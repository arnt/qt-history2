/*
  cppcodeparser.cpp
*/

#include <qfile.h>
#include <qregexp.h>

#include <stdio.h>

#include "codechunk.h"
#include "cppcodeparser.h"
#include "messages.h"
#include "tokenizer.h"
#include "tree.h"

/* qmake ignore Q_OBJECT */

CppCodeParser::CppCodeParser()
{
    reinit( 0 );
    nodeTypeMap.insert( "namespace", Node::Namespace );
    nodeTypeMap.insert( "class", Node::Class );
    nodeTypeMap.insert( "enum", Node::Enum );
    nodeTypeMap.insert( "typedef", Node::Typedef );
    nodeTypeMap.insert( "fn", Node::Function );
    nodeTypeMap.insert( "property", Node::Property );
}

void CppCodeParser::parseHeaderFile( const QString& filePath, Tree *tree )
{
    Location loc( filePath );
    FILE *in = fopen( QFile::encodeName(filePath), "r" );
    if ( in == 0 ) {
	syswarning( "Cannot open C++ header file '%s'", filePath.latin1() );
	return;
    }

    reinit( tree );
    FileTokenizer fileTokenizer( loc, in );
    tokenizer = &fileTokenizer;
    readToken();
    matchDeclList( tree->root() );
    fclose( in );
}

void CppCodeParser::parseSourceFile( const QString& filePath, Tree *tree )
{
    Location loc( filePath );
    FILE *in = fopen( QFile::encodeName(filePath), "r" );
    if ( in == 0 ) {
	syswarning( "Cannot open C++ source file '%s'", filePath.latin1() );
	return;
    }

    reinit( tree );
    FileTokenizer fileTokenizer( loc, in );
    tokenizer = &fileTokenizer;
    readToken();
    matchDocsAndStuff();
    fclose( in );
}

void CppCodeParser::convertTree( const Tree * /* tree */ )
{
}

const FunctionNode *CppCodeParser::findFunctionNode( const QString& synopsis,
						     Tree *tree )
{
    QStringList path;
    FunctionNode *clone = 0;
    FunctionNode *func = 0;

    reinit( tree );
    makeFunctionNode( synopsis, &path, &clone );
    if ( clone != 0 ) {
	func = tree->findFunctionNode( path, clone );
	delete clone;
    }
    return func;
}

StringSet CppCodeParser::topicCommands()
{
    return StringSet() << "class" << "enum" << "file" << "fn" << "group"
		       << "module" << "page" << "property" << "typedef";
}

Node *CppCodeParser::processTopicCommand( const QString& command,
					  const QString& arg, const Doc& doc )
{
    if ( command == "fn" ) {
	QStringList path;
	FunctionNode *func = 0;
	FunctionNode *clone = 0;

	makeFunctionNode( arg, &path, &clone );
	if ( clone == 0 )
	    makeFunctionNode( "void " + arg, &path, &clone );

	if ( clone == 0 ) {
	    warning( 1, doc.location(), "Invalid syntax in '\\fn'" );
	} else {
	    func = tree()->findFunctionNode( path, clone );
	    if ( func == 0 ) {
		if ( path.isEmpty() && !lastPath.isEmpty() )
		    func = tree()->findFunctionNode( lastPath, clone );
		if ( func == 0 ) {
		    warning( 1, doc.location(),
			     "Cannot resolve '%s' in '\\%s'",
			     (clone->name() + "()").latin1(), "fn" );
		} else {
		    warning( 1, doc.location(),
			     "Missing '%s::' for '%s' in '\\fn'",
			     lastPath.join("::").latin1(),
			     (clone->name() + "()").latin1() );
		}
	    } else {
		lastPath = path;
	    }

	    if ( func != 0 )
		func->borrowParameterNames( clone );
	    delete clone;
	}
	return func;
    } else if ( nodeTypeMap.contains(command) ) {
	QStringList args = QStringList::split( ' ', arg );
	Node *node = 0;

	if ( args.isEmpty() ) {
	    warning( 1, doc.location(), "Expected name after '\\%s'",
		     command.latin1() );
	} else {
	    QString name = args[0];
	    QStringList path = QStringList::split( "::", name );
	    node = tree()->findNode( path, nodeTypeMap[command] );
	    if ( node == 0 )
		warning( 1, doc.location(),
			 "Cannot resolve '%s' specified with '\\%s'",
			 name.latin1(), command.latin1() );
	    lastPath = path;
	}
	return node;
    } else {
	if ( command == "file" ) {
	    
	} else if ( command == "group" ) {

	} else if ( command == "module" ) {

	} else if ( command == "page" ) {
	    Doc pageDoc = doc;
	    if ( pageDoc.metaCommands() != 0 ) {
		QStringList granularities =
			pageDoc.extractMetaCommand( "granularity" );
		if ( !granularities.isEmpty() )
		    ; /* graunlarities.last() */
	    }
	}
	return 0;
    }
}

StringSet CppCodeParser::otherMetaCommands()
{
    return StringSet() << "important" << "ingroup" << "inheaderfile"
		       << "inmodule" << "internal" << "mainclass" << "obsolete"
		       << "overload" << "preliminary" << "reimp" << "related";
}

void CppCodeParser::processOtherMetaCommand( const QString& command,
					     const QString& arg, Node *node )
{
    if ( command == "important" ) {
	/* ... */
    } else if ( command == "ingroup" ) {
	/* ... */
    } else if ( command == "inheaderfile" ) {
	if ( node->isInnerNode() ) {
	    ((InnerNode *) node)->addInclude( arg );
	} else if ( node->parent()->parent() == 0 ) {
	    /* global function ... */
	} else {
	    warning( 1, node->doc().location(), "Ignored '\\headerfile'" );
	}
    } else {
	if ( command == "deprecated" ) {
	    node->setStatus( Node::Deprecated );
	} else if ( command == "obsolete" ) {
	    node->setStatus( Node::Obsolete );
	} else if ( command == "overload" ) {
	    if ( node->type() == Node::Function ) {
		((FunctionNode *) node)->setOverload( TRUE );
		FunctionNode *primary =
			node->parent()->findFunctionNode( node->name() );
		if ( primary->isOverload() ) {
		    warning( 1, primary->doc().location(),
			     "All versions of '%s' are '\\overload'",
			     (primary->name() + "()").latin1() );
		    primary->setOverload( FALSE );
		}
	    } else {
		warning( 1, node->doc().location(), "Ignored '\\overload'" );
	    }
	} else if ( command == "private" ) {
	    node->setAccess( Node::Private );
	} else if ( command == "reimp" ) {
	    if ( node->type() == Node::Function ) {
		((FunctionNode *) node)->setReimplementation( TRUE );
	    } else {
		warning( 1, node->doc().location(), "Ignored '\\reimp'" );
	    }
	}

	if ( !arg.isEmpty() )
	    warning( 1, node->doc().location(),
		     "Ignored garbage after '\\%1' command", command.latin1() );
    }
}

void CppCodeParser::reinit( Tree *tree )
{
    tre = tree;
    tokenizer = 0;
    tok = 0;
    access = Node::Public;
    metaness = FunctionNode::Plain;
    lastPath.clear();
}

void CppCodeParser::readToken()
{
    tok = tokenizer->getToken();
}

const Location& CppCodeParser::location()
{
    return tokenizer->location();
}

QString CppCodeParser::previousLexeme()
{
    return tokenizer->previousLexeme();
}

QString CppCodeParser::lexeme()
{
    return tokenizer->lexeme();
}

bool CppCodeParser::match( int target )
{
    if ( tok == target ) {
	readToken();
	return TRUE;
    } else {
	return FALSE;
    }
}

bool CppCodeParser::matchTemplateAngles( CodeChunk *dataType )
{
    bool matches = ( tok == Tok_LeftAngle );
    if ( matches ) {
	int depth = 0;
	do {
	    if ( tok == Tok_LeftAngle )
		depth++;
	    else if ( tok == Tok_RightAngle )
		depth--;
	    if ( dataType != 0 )
		dataType->append( lexeme() );
	    readToken();
	} while ( depth > 0 && tok != Tok_Eoi );
    }
    return matches;
}

bool CppCodeParser::matchTemplateHeader()
{
    readToken();
    return matchTemplateAngles();
}

bool CppCodeParser::matchDataType( CodeChunk *dataType, QString *var = 0 )
{
    /*
      This code is really hard to follow... sorry. The loop is there to match
      Alpha::Beta::Gamma::...::Omega.
    */
    for ( ;; ) {
	bool virgin = TRUE;

	if ( tok != Tok_Ident ) {
	    /*
	      There is special processing for 'Foo::operator int()'
	      and such elsewhere. This is the only case where we
	      return something with a trailing gulbrandsen ('Foo::').
	    */
	    if ( tok == Tok_operator )
		return TRUE;

	    /*
	      People may write 'const unsigned short' or
	      'short unsigned const' or any other permutation.
	    */
	    while ( match(Tok_const) || match(Tok_volatile) )
		dataType->append( previousLexeme() );
	    while ( match(Tok_signed) || match(Tok_unsigned) ||
		    match(Tok_short) || match(Tok_long) ) {
		dataType->append( previousLexeme() );
		virgin = FALSE;
	    }
	    while ( match(Tok_const) || match(Tok_volatile) )
		dataType->append( previousLexeme() );

	    if ( match(Tok_Tilde) )
		dataType->append( previousLexeme() );
	}

	if ( virgin ) {
	    if ( match(Tok_Ident) )
		dataType->appendBase( previousLexeme() );
	    else if ( match(Tok_void) || match(Tok_int) || match(Tok_char) ||
		      match(Tok_double) || match(Tok_Ellipsis) )
		dataType->append( previousLexeme() );
	    else
		return FALSE;
	} else if ( match(Tok_int) || match(Tok_char) || match(Tok_double) ) {
	    dataType->append( previousLexeme() );
	}

	matchTemplateAngles( dataType );

	while ( match(Tok_const) || match(Tok_volatile) )
	    dataType->append( previousLexeme() );

	if ( match(Tok_Gulbrandsen) )
	    dataType->appendBase( previousLexeme() );
	else
	    break;
    }

    while ( match(Tok_Ampersand) || match(Tok_Aster) || match(Tok_const) )
	dataType->append( previousLexeme() );

    if ( match(Tok_LeftParenAster) ) {
	/*
	  A function pointer. This would be rather hard to handle without a
	  tokenizer hack, because a type can be followed with a left parenthesis
	  in some cases (e.g., 'operator int()'). The tokenizer recognizes '(*'
	  as a single token.
	*/
	dataType->append( previousLexeme() );
	dataType->appendHotspot();
	if ( var != 0 && match(Tok_Ident) )
	    *var = previousLexeme();
	if ( !match(Tok_RightParen) || tok != Tok_LeftParen )
	    return FALSE;
	dataType->append( previousLexeme() );

	int parenDepth0 = tokenizer->parenDepth();
	while ( tokenizer->parenDepth() >= parenDepth0 && tok != Tok_Eoi ) {
	    dataType->append( lexeme() );
	    readToken();
	}
	if ( match(Tok_RightParen) )
	    dataType->append( previousLexeme() );
    } else {
	/*
	  The common case: Look for an optional identifier, then for
	  some array brackets.
	*/
	dataType->appendHotspot();

	if ( var != 0 ) {
	    if ( match(Tok_Ident) ) {
		*var = previousLexeme();
	    } else if ( match(Tok_Comment) ) {
		QRegExp varComment( "/\\*\\s([a-zA-Z_0-9]+)\\s\\*/" );

		/*
		  A neat hack: Commented-out parameter names are
		  recognized by qdoc. It's impossible to illustrate
		  here inside a C-style comment, because it requires
		  an asterslash. It's also impossible to illustrate
		  inside a C++-style comment, because the explanation
		  does not fit on one line.
		*/
		if ( varComment.exactMatch(previousLexeme()) )
		    *var = varComment.cap( 1 );
	    }
	}

	if ( tok == Tok_LeftBracket ) {
	    int bracketDepth0 = tokenizer->bracketDepth();
	    while ( (tokenizer->bracketDepth() >= bracketDepth0 &&
		     tok != Tok_Eoi) ||
		    tok == Tok_RightBracket ) {
		dataType->append( lexeme() );
		readToken();
	    }
	}
    }
    return TRUE;
}

bool CppCodeParser::matchParameter( FunctionNode *func )
{
    CodeChunk dataType;
    QString name;
    CodeChunk defaultValue;

    if ( !matchDataType(&dataType, &name) )
	return FALSE;
    match( Tok_Comment );
    if ( match(Tok_Equal) ) {
	int parenDepth0 = tokenizer->parenDepth();

	while ( tokenizer->parenDepth() >= parenDepth0 &&
		(tok != Tok_Comma ||
		 tokenizer->parenDepth() > parenDepth0) &&
		tok != Tok_Eoi ) {
	    defaultValue.append( lexeme() );
	    readToken();
	}
    }
    func->addParameter( Parameter(dataType.toString(), "", name,
				  defaultValue.toString()) ); // ###
    return TRUE;
}

bool CppCodeParser::matchFunctionDecl( InnerNode *parent, QStringList *pathPtr,
				       FunctionNode **funcPtr )
{
    QRegExp sep( "(?:<[^>]+>)?::" );
    CodeChunk returnType;
    QStringList path;
    QString name;

    bool sta = match( Tok_static );
    FunctionNode::Virtualness vir = FunctionNode::NonVirtual;
    if ( match(Tok_virtual) )
	vir = FunctionNode::ImpureVirtual;

    if ( !matchDataType(&returnType) )
	return FALSE;

    if ( tok == Tok_operator &&
	 (returnType.toString().isEmpty() ||
	  returnType.toString().endsWith("::")) ) {
	// 'QString::operator const char *()'
	path = QStringList::split( sep, returnType.toString() );
	returnType = CodeChunk();
	readToken();

	CodeChunk restOfName;
	if ( !matchDataType(&restOfName) )
	    return FALSE;
	name = "operator " + restOfName.toString();
    } else if ( tok == Tok_LeftParen ) {
	// constructor or destructor
	path = QStringList::split( sep, returnType.toString() );
	if ( !path.isEmpty() ) {
	    name = path.last();
	    path.remove( path.fromLast() );
	}
	returnType = CodeChunk();
    } else {
	while ( match(Tok_Ident) ) {
	    name = previousLexeme();
	    matchTemplateAngles();

	    if ( match(Tok_Gulbrandsen) ) {
		path.append( name );
	    } else {
		break;
	    }
	}

	if ( tok == Tok_operator ) {
	    name = lexeme();
	    readToken();
	    while ( tok != Tok_Eoi ) {
		name += lexeme();
		readToken();
		if ( tok == Tok_LeftParen )
		    break;
	    }
	}
	if ( tok != Tok_LeftParen )
	    return FALSE;
    }
    readToken();

    FunctionNode *func = new FunctionNode( parent, name );
    func->setAccess( access );
    func->setLocation( location() );
    func->setReturnType( returnType.toString() );
    func->setMetaness( metaness );
    func->setStatic( sta );

    if ( tok != Tok_RightParen ) {
	do {
	    if ( !matchParameter(func) )
		return FALSE;
	} while ( match(Tok_Comma) );
    }
    if ( !match(Tok_RightParen) )
	return FALSE;

    func->setConst( match(Tok_const) );

    if ( match(Tok_Equal) && match(Tok_Number) )
	vir = FunctionNode::PureVirtual;
    func->setVirtualness( vir );

    if ( match(Tok_Colon) ) {
	while ( tok != Tok_LeftBrace && tok != Tok_Eoi )
	    readToken();
    }

    if ( !match(Tok_Semicolon) && tok != Tok_Eoi ) {
	int braceDepth0 = tokenizer->braceDepth();

	if ( !match(Tok_LeftBrace) )
	    return FALSE;
	while ( tokenizer->braceDepth() >= braceDepth0 && tok != Tok_Eoi )
	    readToken();
	match( Tok_RightBrace );
    }
    if ( pathPtr != 0 )
	*pathPtr = path;
    if ( funcPtr != 0 )
	*funcPtr = func;
    return TRUE;
}

bool CppCodeParser::matchBaseSpecifier( ClassNode *classe )
{
    Node::Access access;

    if ( tok == Tok_virtual )
	readToken();

    switch ( tok ) {
    case Tok_public:
	access = Node::Public;
	break;
    case Tok_protected:
	access = Node::Protected;
	break;
    case Tok_private:
	access = Node::Private;
	break;
    default:
	return FALSE;
    }
    readToken();

    CodeChunk baseClass;
    bool matches = matchDataType( &baseClass );
    if ( matches )
	tree()->addBaseClass( classe, access, baseClass.toString(), "" ); // ###
    return matches;
}

bool CppCodeParser::matchBaseList( ClassNode *classe )
{
    for ( ;; ) {
	if ( !matchBaseSpecifier(classe) )
	    return FALSE;
	if ( tok == Tok_LeftBrace )
	    return TRUE;
	if ( !match(Tok_Comma) )
	    return FALSE;
    }
}

bool CppCodeParser::matchClassDecl( InnerNode *parent )
{
    bool isClass = ( tok == Tok_class );
    readToken();
    if ( tok != Tok_Ident )
	return FALSE;
    while ( tok == Tok_Ident )
	readToken();
    if ( tok != Tok_Colon && tok != Tok_LeftBrace )
	return FALSE;

    /*
      So far, so good. We have 'class Foo {' or 'class Foo :'. This is enough
      to recognize a class definition.
    */
    ClassNode *classe = new ClassNode( parent, previousLexeme() );
    classe->setAccess( access );
    classe->setLocation( location() );

    if ( match(Tok_Colon) && !matchBaseList(classe) )
	return FALSE;
    if ( !match(Tok_LeftBrace) )
	return FALSE;

    Node::Access outerAccess = access;
    access = isClass ? Node::Private : Node::Public;
    FunctionNode::Metaness outerMetaness = metaness;
    metaness = FunctionNode::Plain;

    bool matches = ( matchDeclList(classe) && match(Tok_RightBrace) &&
		     match(Tok_Semicolon) );
    access = outerAccess;
    metaness = outerMetaness;
    return matches;
}

bool CppCodeParser::matchEnumItem( EnumNode * /* enume */ )
{
    if ( !match(Tok_Ident) )
	return FALSE;

    QString name = previousLexeme();
    CodeChunk val;

    if ( match(Tok_Equal) ) {
	while ( tok != Tok_Comma && tok != Tok_RightBrace &&
		tok != Tok_Eoi ) {
	    val.append( lexeme() );
	    readToken();
	}
    }
#if notyet
    enume->addItem( new EnumItemNode(name, val) );
#endif
    return TRUE;
}

bool CppCodeParser::matchEnumDecl( InnerNode *parent )
{
    QString name;

    if ( !match(Tok_enum) )
	return FALSE;
    if ( match(Tok_Ident) )
	name = previousLexeme();

    if ( tok != Tok_LeftBrace )
	return FALSE;

    EnumNode *enume = new EnumNode( parent, name );
    enume->setAccess( access );
    enume->setLocation( location() );

    readToken();

    if ( !matchEnumItem(enume) )
	return FALSE;

    while ( match(Tok_Comma) ) {
	if ( !matchEnumItem(enume) )
	    return FALSE;
    }
    return match( Tok_RightBrace ) && match( Tok_Semicolon );
}

bool CppCodeParser::matchTypedefDecl( InnerNode *parent )
{
    CodeChunk dataType;
    QString name;

    if ( !match(Tok_typedef) )
	return FALSE;
    if ( !matchDataType(&dataType, &name) )
	return FALSE;
    if ( !match(Tok_Semicolon) )
	return FALSE;

    TypedefNode *typedeffe = new TypedefNode( parent, name );
    typedeffe->setAccess( access );
    typedeffe->setLocation( location() );
    return TRUE;
}

bool CppCodeParser::matchProperty( InnerNode *parent )
{
    if ( !match(Tok_Q_PROPERTY) && !match(Tok_Q_OVERRIDE) )
	return FALSE;
    if ( !match(Tok_LeftParen) )
	return FALSE;

    QString name;
    CodeChunk dataType( lexeme() );

    readToken();
    matchTemplateAngles( &dataType );

    if ( !match(Tok_Ident) )
	return FALSE;
    name = previousLexeme();

    PropertyNode *property = new PropertyNode( parent, name );
    property->setAccess( Node::Public );
    property->setLocation( location() );
    property->setDataType( dataType.toString() );

    while ( tok != Tok_RightParen && tok != Tok_Eoi ) {
	if ( !match(Tok_Ident) )
	    return FALSE;
	QString key = previousLexeme();

	if ( !match(Tok_Ident) )
	    return FALSE;
	QString value = previousLexeme();

	if ( key == "READ" )
	    property->setGetter( value );
	else if ( key == "WRITE" )
	    property->setSetter( value );
	else if ( key == "STORED" )
	    property->setStored( value.lower() == "true" );
	else if ( key == "DESIGNABLE" )
	    property->setDesignable( value.lower() == "true" );
	else if ( key == "RESET" )
	    property->setResetter( value );
    }
    match( Tok_RightParen );
    return TRUE;
}

bool CppCodeParser::matchDeclList( InnerNode *parent )
{
    int braceDepth0 = tokenizer->braceDepth();
    if ( tok == Tok_RightBrace ) // prevents failure on empty body
	braceDepth0++;

    while ( tokenizer->braceDepth() >= braceDepth0 && tok != Tok_Eoi ) {
	switch ( tok ) {
	case Tok_Colon:
	    readToken();
	    break;
	case Tok_class:
	case Tok_struct:
	case Tok_union:
	    matchClassDecl( parent );
	    break;
	case Tok_template:
	    matchTemplateHeader();
	    break;
	case Tok_enum:
	    matchEnumDecl( parent );
	    break;
	case Tok_typedef:
	    matchTypedefDecl( parent );
	    break;
	case Tok_private:
	    readToken();
	    access = Node::Private;
	    metaness = FunctionNode::Plain;
	    break;
	case Tok_protected:
	    readToken();
	    access = Node::Protected;
	    metaness = FunctionNode::Plain;
	    break;
	case Tok_public:
	    readToken();
	    access = Node::Public;
	    metaness = FunctionNode::Plain;
	    break;
	case Tok_signals:
	    readToken();
	    access = Node::Public;
	    metaness = FunctionNode::Signal;
	    break;
	case Tok_slots:
	    readToken();
	    metaness = FunctionNode::Slot;
	    break;
	case Tok_Q_OBJECT:
	    readToken();
	    break;
	case Tok_Q_ENUMS:
	    readToken();
	    while ( tok != Tok_RightParen && tok != Tok_Eoi )
		readToken();
	    match( Tok_RightParen );
	    break;
	case Tok_Q_OVERRIDE:
	case Tok_Q_PROPERTY:
	    matchProperty( parent );
	    break;
	default:
	    if ( !matchFunctionDecl(parent) ) {
		while ( tok != Tok_Eoi &&
			(tokenizer->braceDepth() > braceDepth0 ||
			 !match(Tok_Semicolon)) )
		    readToken();
	    }
	}
    }
    return TRUE;
}

bool CppCodeParser::matchDocsAndStuff()
{
    StringSet topicsAvailable = topicCommands();
    StringSet otherMetaCommandsAvailable = otherMetaCommands();
    StringSet metaCommandsAvailable = reunion( topicsAvailable,
					       otherMetaCommandsAvailable );

    while ( tok != Tok_Eoi ) {
	if ( tok == Tok_Doc ) {
	    QString text = lexeme();
	    Location loc( location() );
	    readToken();

	    removeAsters( loc, text );
	    Doc doc( loc, text, metaCommandsAvailable );

	    QString command;
	    QStringList args;

	    if ( doc.metaCommands() != 0 ) {
		StringSet topicsUsed = intersection( topicsAvailable,
						     *doc.metaCommands() );
		if ( topicsUsed.count() > 0 ) {
		    command = topicsUsed.first();
		    args = doc.extractMetaCommand( command );
		}
	    }

	    NodeList nodes;

	    if ( command.isEmpty() ) {
		QStringList path;
		FunctionNode *clone = 0;
		FunctionNode *func = 0;

		if ( matchFunctionDecl(0, &path, &clone) ) {
		    func = tree()->findFunctionNode( path, clone );
		    if ( func == 0 ) {
			warning( 1, doc.location(),
				 "Cannot tie this documentation to anything" );
		    } else {
			func->borrowParameterNames( clone );
			nodes.append( func );
		    }
		    delete clone;		
		}
	    } else {
		QStringList::ConstIterator a = args.begin();
		while ( a != args.end() ) {
		    Node *node = processTopicCommand( command, *a, doc );
		    if ( node != 0 )
			nodes.append( node );
		    ++a;
		}
	    }

	    NodeList::Iterator n = nodes.begin();
	    while ( n != nodes.end() ) {
		Doc nodeDoc = doc;
		if ( nodeDoc.metaCommands() != 0 ) {
		    StringSet metaCommands = *nodeDoc.metaCommands();
		    StringSet::ConstIterator c = metaCommands.begin();
		    while ( c != metaCommands.end() ) {
			args = nodeDoc.extractMetaCommand( *c );
			QStringList::ConstIterator a = args.begin();
			while ( a != args.end() ) {
			    processOtherMetaCommand( *c, *a, *n );
			    ++a;
			}
			++c;
		    }
		}
		if ( (*n)->isInnerNode() ) {
		    InnerNode *inner = (InnerNode *) *n;
		    if ( inner->includes().isEmpty() )
			inner->addInclude( inner->location().fileName() );
		}
		(*n)->setDoc( nodeDoc );
		++n;
	    }
	} else {
	    QStringList path;
	    FunctionNode *clone = 0;
	    FunctionNode *node = 0;

	    if ( matchFunctionDecl(0, &path, &clone) ) {
		/*
		  The location of the definition is more interesting
		  than that of the declaration. People equipped with
		  a sophisticated text editor can respond to warnings
		  concerning undocumented functions very quickly.

		  Signals are implemented in uninteresting files
		  generated by moc.
		*/
		node = tree()->findFunctionNode( path, clone );
		if ( node != 0 && node->metaness() != FunctionNode::Signal )
		    node->setLocation( clone->location() );
		delete clone;
	    } else {
		if ( tok != Tok_Doc )
		    readToken();
	    }
	}
    }
    return TRUE;
}

void CppCodeParser::makeFunctionNode( const QString& synopsis,
				      QStringList *pathPtr,
				      FunctionNode **funcPtr )
{
    Tokenizer *outerTokenizer = tokenizer;
    int outerTok = tok;

    Location loc;
    StringTokenizer stringTokenizer( loc, synopsis.latin1(),
				     synopsis.length() );
    tokenizer = &stringTokenizer;
    readToken();
    matchFunctionDecl( 0, pathPtr, funcPtr );

    tokenizer = outerTokenizer;
    tok = outerTok;
}

void CppCodeParser::removeAsters( Location& location, QString& text )
{
    QString cleaned;
    Location m = location;
    bool metAsterColumn = TRUE;
    int asterColumn = location.columnNum() + 1;
    int i;

    for ( i = 0; i < (int) text.length(); i++ ) {
	if ( m.columnNum() == asterColumn ) {
	    if ( text[i] != '*' )
		break;
	    cleaned += ' ';
	    metAsterColumn = TRUE;
	} else {
	    if ( text[i] == '\n' ) {
		if ( !metAsterColumn )
		    break;
		metAsterColumn = FALSE;
	    }
	    cleaned += text[i];
	}
	m.advance( text[i] );
    }
    if ( cleaned.length() == text.length() )
	text = cleaned;

    for ( int i = 0; i < 3; i++ )
	location.advance( text[i] );
    text = text.mid( 3, text.length() - 5 );
}
