/*
  cppcodeparser.cpp
*/

#include <qfile.h>

#include <stdio.h>

#include "codechunk.h"
#include "cppcodeparser.h"
#include "tokenizer.h"
#include "tree.h"

/* qmake ignore Q_OBJECT */

#define COMMAND_CLASS                   Doc::alias("class")
#define COMMAND_ENUM                    Doc::alias("enum")
#define COMMAND_FILE                    Doc::alias("file")
#define COMMAND_FN                      Doc::alias("fn")
#define COMMAND_GROUP                   Doc::alias("group")
#define COMMAND_HEADERFILE              Doc::alias("headerfile")
#define COMMAND_INHEADERFILE            Doc::alias("inheaderfile")
#define COMMAND_MODULE                  Doc::alias("module")
#define COMMAND_NAMESPACE               Doc::alias("namespace")
#define COMMAND_OVERLOAD                Doc::alias("overload")
#define COMMAND_PAGE                    Doc::alias("page")
#define COMMAND_PROPERTY                Doc::alias("property")
#define COMMAND_REIMP                   Doc::alias("reimp")
#define COMMAND_RELATES                 Doc::alias("relates")
#define COMMAND_TYPEDEF                 Doc::alias("typedef")

CppCodeParser::CppCodeParser()
    : varComment("/\\*\\s([a-zA-Z_0-9]+)\\s\\*/"), sep("(?:<[^>]+>)?::")
{
    reset( 0 );
}

CppCodeParser::~CppCodeParser()
{
}

void CppCodeParser::initializeParser(const Config &config)
{
    CodeParser::initializeParser(config);

    nodeTypeMap.insert(COMMAND_NAMESPACE, Node::Namespace);
    nodeTypeMap.insert(COMMAND_CLASS, Node::Class);
    nodeTypeMap.insert(COMMAND_ENUM, Node::Enum);
    nodeTypeMap.insert(COMMAND_TYPEDEF, Node::Typedef);
    nodeTypeMap.insert(COMMAND_FN, Node::Function);
    nodeTypeMap.insert(COMMAND_PROPERTY, Node::Property);
}

void CppCodeParser::terminateParser()
{
    nodeTypeMap.clear();
    CodeParser::terminateParser();
}

QString CppCodeParser::language()
{
    return "Cpp";
}

QString CppCodeParser::headerFileNameFilter()
{
    return "*.ch *.h *.h++ *.hh *.hpp *.hxx";
}

QString CppCodeParser::sourceFileNameFilter()
{
    return "*.c++ *.cc *.cpp *.cxx";
}

void CppCodeParser::parseHeaderFile( const Location& location,
				     const QString& filePath, Tree *tree )
{
    FILE *in = fopen(QFile::encodeName(filePath), "r");
    if (!in) {
	location.error(tr("Cannot open C++ header file '%1'").arg(filePath));
	return;
    }

    reset( tree );
    Location fileLocation( filePath );
    FileTokenizer fileTokenizer( fileLocation, in );
    tokenizer = &fileTokenizer;
    readToken();
    matchDeclList( tree->root() );
    if (!fileTokenizer.version().isEmpty())
	tree->setVersion(fileTokenizer.version());
    fclose(in);

    if (fileLocation.fileName() == "qiterator.h")
	parseQiteratorDotH(location, filePath, tree);
}

void CppCodeParser::parseSourceFile( const Location& location,
				     const QString& filePath, Tree *tree )
{
    FILE *in = fopen( QFile::encodeName(filePath), "r" );
    if (!in) {
	location.error( tr("Cannot open C++ source file '%1'").arg(filePath) );
	return;
    }

    reset( tree );
    Location fileLocation( filePath );
    FileTokenizer fileTokenizer( fileLocation, in );
    tokenizer = &fileTokenizer;
    readToken();
    matchDocsAndStuff();
    fclose( in );
}

void CppCodeParser::doneParsingHeaderFiles( Tree *tree )
{
    tree->resolveInheritance();
    tree->resolveProperties();

    QMapIterator<QString, QString> i(linearIteratorClasses);
    while (i.hasNext()) {
	i.next();
	instantiateIteratorMacro(i.key(), i.value(), linearIteratorDefinition, tree);
    }
    i = associativeIteratorClasses;
    while (i.hasNext()) {
	i.next();
	instantiateIteratorMacro(i.key(), i.value(), associativeIteratorDefinition, tree);
    }
    linearIteratorDefinition.clear();
    associativeIteratorDefinition.clear();
    linearIteratorClasses.clear();
    associativeIteratorClasses.clear();
}

void CppCodeParser::doneParsingSourceFiles( Tree *tree )
{
    tree->root()->makeUndocumentedChildrenInternal();
    tree->root()->normalizeOverloads();
}

const FunctionNode *CppCodeParser::findFunctionNode( const QString& synopsis,
						     Tree *tree )
{
    QStringList parentPath;
    FunctionNode *clone;
    FunctionNode *func = 0;

    reset( tree );
    if ( makeFunctionNode(synopsis, &parentPath, &clone) ) {
	func = tree->findFunctionNode( parentPath, clone );
	delete clone;
    }
    return func;
}

Set<QString> CppCodeParser::topicCommands()
{
    return Set<QString>() << COMMAND_CLASS << COMMAND_ENUM << COMMAND_FILE << COMMAND_FN
			  << COMMAND_GROUP << COMMAND_HEADERFILE << COMMAND_MODULE
#ifdef QDOC2_COMPAT
			  << COMMAND_OVERLOAD
#endif
			  << COMMAND_PAGE << COMMAND_PROPERTY << COMMAND_TYPEDEF;
}

Node *CppCodeParser::processTopicCommand( const Doc& doc,
					  const QString& command,
					  const QString& arg )
{
#ifdef QDOC2_COMPAT
    if ( command == COMMAND_FN || command == COMMAND_OVERLOAD ) {
#else
    if ( command == COMMAND_FN ) {
#endif
	QStringList parentPath;
	FunctionNode *func = 0;
	FunctionNode *clone;

	if ( !makeFunctionNode(arg, &parentPath, &clone) &&
	     !makeFunctionNode("void " + arg, &parentPath, &clone) ) {
#ifdef QDOC2_COMPAT
	    if ( command != COMMAND_OVERLOAD )
#endif
	    doc.location().warning( tr("Invalid syntax in '\\%1'")
				    .arg(COMMAND_FN) );
	} else {
	    func = tre->findFunctionNode( parentPath, clone );
	    if ( func == 0 ) {
		if ( parentPath.isEmpty() && !lastPath.isEmpty() )
		    func = tre->findFunctionNode( lastPath, clone );
		if ( func == 0 ) {
		    doc.location().warning( tr("Cannot resolve '%1' in '\\%2'")
					    .arg(clone->name() + "()")
					    .arg(COMMAND_FN) );
		} else {
		    doc.location().warning( tr("Missing '%1::' for '%2' in '\\%3'")
					    .arg(lastPath.join("::"))
					    .arg(clone->name() + "()")
					    .arg(COMMAND_FN) );
		}
	    } else {
		lastPath = parentPath;
	    }

	    if ( func != 0 ) {
#ifdef QDOC2_COMPAT
		if ( command == COMMAND_OVERLOAD )
		    func->setOverload( TRUE );
#endif
		func->borrowParameterNames( clone );
	    }
	    delete clone;
	}
	return func;
    } else if ( nodeTypeMap.contains(command) ) {
	// ### split(" ") hack is there to support header file syntax
	QStringList path = arg.split(" ")[0].split("::");
	Node *node = tre->findNode(path
#ifndef QDOC2_COMPAT
				   , nodeTypeMap[command]
#endif
				   );
	if ( node == 0 ) {
	    doc.location().warning(tr("Cannot resolve '%1' specified with '\\%2'")
				   .arg(arg).arg(command));
	    lastPath = path;
	}
	return node;
    } else if ( command == COMMAND_FILE ) {
	return new FakeNode( tre->root(), arg, FakeNode::File );
    } else if ( command == COMMAND_GROUP ) {
	return new FakeNode( tre->root(), arg, FakeNode::Group );
    } else if (command == COMMAND_HEADERFILE) {
	return new FakeNode( tre->root(), arg, FakeNode::HeaderFile );
    } else if ( command == COMMAND_MODULE ) {
	return new FakeNode( tre->root(), arg, FakeNode::Module );
    } else if ( command == COMMAND_PAGE ) {
	return new FakeNode( tre->root(), arg, FakeNode::Page );
    } else {
	return 0;
    }
}

Set<QString> CppCodeParser::otherMetaCommands()
{
    return commonMetaCommands() << COMMAND_INHEADERFILE << COMMAND_OVERLOAD
				<< COMMAND_REIMP << COMMAND_RELATES;
}

void CppCodeParser::processOtherMetaCommand( const Doc& doc,
					     const QString& command,
					     const QString& arg,
					     Node *node )
{
    if ( command == COMMAND_INHEADERFILE ) {
	if ( node != 0 && node->isInnerNode() ) {
	    ((InnerNode *) node)->addInclude( arg );
	} else {
	    doc.location().warning(tr("Ignored '\\%1'").arg(COMMAND_INHEADERFILE));
	}
    } else if ( command == COMMAND_OVERLOAD ) {
	if ( node != 0 && node->type() == Node::Function ) {
	    ((FunctionNode *) node)->setOverload( TRUE );
	} else {
	    doc.location().warning( tr("Ignored '\\%1'")
				    .arg(COMMAND_OVERLOAD) );
	}
    } else if ( command == COMMAND_REIMP ) {
	if ( node != 0 && node->type() == Node::Function ) {
	    FunctionNode *func = (FunctionNode *) node;
	    if ( func->reimplementedFrom() == 0 ) {
		doc.location().warning( tr("Cannot find base function for '\\%1'")
					.arg(COMMAND_REIMP) );
	    } else {
		func->setAccess( Node::Private );
	    }
	} else {
	    doc.location().warning( tr("Ignored '\\%1'").arg(COMMAND_REIMP) );
	}
    } else if (command == COMMAND_RELATES) {
	InnerNode *pseudoParent;
	if (arg.startsWith("<") || arg.startsWith("\"")) {
	    pseudoParent = static_cast<InnerNode *>(tre->findNode(arg, Node::Fake));
	} else {
	    pseudoParent = static_cast<InnerNode *>(tre->findNode(arg, Node::Class));
        }
	if (!pseudoParent) {
	    doc.location().warning(tr("Cannot resolve '%1' in '\\%2'")
				   .arg(arg).arg(COMMAND_RELATES));
	} else {
	    node->setRelates(pseudoParent);
        }
    } else {
	processCommonMetaCommand( doc.location(), command, arg, node );
    }
}

void CppCodeParser::processOtherMetaCommands( const Doc& doc, Node *node )
{
    const Set<QString> metaCommands = doc.metaCommandsUsed();
    Set<QString>::ConstIterator c = metaCommands.begin();
    while ( c != metaCommands.end() ) {
	QStringList args = doc.metaCommandArgs(*c);
	QStringList::ConstIterator a = args.begin();
	while ( a != args.end() ) {
	    processOtherMetaCommand( doc, *c, *a, node );
	    ++a;
	}
	++c;
    }
}

void CppCodeParser::reset( Tree *tree )
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

bool CppCodeParser::matchDataType( CodeChunk *dataType, QString *var )
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

bool CppCodeParser::matchFunctionDecl(InnerNode *parent, QStringList *parentPathPtr,
				      FunctionNode **funcPtr)
{
    CodeChunk returnType;
    QStringList parentPath;
    QString name;

    if (match(Tok_friend))
	return false;
    bool sta = match(Tok_static);
    FunctionNode::Virtualness vir = FunctionNode::NonVirtual;
    if ( match(Tok_virtual) )
	vir = FunctionNode::ImpureVirtual;

    if ( !matchDataType(&returnType) )
	return FALSE;

    if (tok == Tok_operator &&
	 (returnType.toString().isEmpty() || returnType.toString().endsWith("::"))) {
	// 'QString::operator const char *()'
	parentPath = returnType.toString().split(sep);
        parentPath.remove(QString());
	returnType = CodeChunk();
	readToken();

	CodeChunk restOfName;
	if ( !matchDataType(&restOfName) )
	    return FALSE;
	name = "operator " + restOfName.toString();
    } else if ( tok == Tok_LeftParen ) {
	// constructor or destructor
	parentPath = returnType.toString().split(sep);
	if ( !parentPath.isEmpty() ) {
	    name = parentPath.last();
	    parentPath.erase( parentPath.end() - 1 );
	}
	returnType = CodeChunk();
    } else {
	while ( match(Tok_Ident) ) {
	    name = previousLexeme();
	    matchTemplateAngles();

	    if ( match(Tok_Gulbrandsen) ) {
		parentPath.append( name );
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

    FunctionNode *func = new FunctionNode(parent, name);
    func->setAccess(access);
    func->setLocation(location());
    func->setReturnType(returnType.toString());

    func->setMetaness(metaness);
    if (parent) {
	if (name == parent->name()) {
	    func->setMetaness(FunctionNode::Ctor);
	} else if (name.startsWith("~"))  {
	    func->setMetaness(FunctionNode::Dtor);
	}
    }
    func->setStatic(sta);

    if (tok != Tok_RightParen) {
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
    if ( parentPathPtr != 0 )
	*parentPathPtr = parentPath;
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
    if (!matchDataType(&baseClass))
	return false;

    tre->addBaseClass(classe, access, baseClass.toPath(), baseClass.toString());
    return true;
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

bool CppCodeParser::matchEnumItem( EnumNode *enume )
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
    enume->addItem( EnumItem(name, val.toString()) );
    return TRUE;
}

bool CppCodeParser::matchEnumDecl( InnerNode *parent )
{
    QString name;

    if ( !match(Tok_enum) )
	return FALSE;
    if ( !match(Tok_Ident) )
	return FALSE;
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
	    tre->addPropertyFunction(property, value, PropertyNode::Getter);
	else if ( key == "WRITE" )
	    tre->addPropertyFunction(property, value, PropertyNode::Setter);
	else if ( key == "STORED" )
	    property->setStored( value.toLower() == "true" );
	else if ( key == "DESIGNABLE" )
	    property->setDesignable( value.toLower() == "true" );
	else if ( key == "RESET" )
	    tre->addPropertyFunction(property, value, PropertyNode::Resetter);
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
	case Tok_Q_DECLARE_ITERATOR:
	    readToken();
            if (match(Tok_LeftParen) && match(Tok_Ident))
		linearIteratorClasses.insert(previousLexeme(), location().fileName());
            match(Tok_RightParen);
	    break;
        case Tok_Q_DECLARE_ASSOCIATIVE_ITERATOR:
	    readToken();
            if (match(Tok_LeftParen) && match(Tok_Ident))
		associativeIteratorClasses.insert(previousLexeme(), location().fileName());
            match(Tok_RightParen);
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
    Set<QString> topicsAvailable = topicCommands();
    Set<QString> otherMetaCommandsAvailable = otherMetaCommands();
    Set<QString> metaCommandsAvailable = topicsAvailable +
					 otherMetaCommandsAvailable;

    while ( tok != Tok_Eoi ) {
	if ( tok == Tok_Doc ) {
	    QString comment = lexeme();
	    Location loc( location() );
	    readToken();

	    Doc::trimCStyleComment( loc, comment );
	    Doc doc( loc, comment, metaCommandsAvailable );

	    QString command;
	    QStringList args;

	    Set<QString> topicsUsed = intersection(topicsAvailable, doc.metaCommandsUsed());
	    if ( topicsUsed.count() > 0 ) {
		command = topicsUsed.first();
		args = doc.metaCommandArgs( command );
		// ### what if topicsUsed.count() > 1 ?
	    }

	    NodeList nodes;
	    QList<Doc> docs;

	    if ( command.isEmpty() ) {
		QStringList parentPath;
		FunctionNode *clone;
		FunctionNode *func = 0;

		if ( matchFunctionDecl(0, &parentPath, &clone) ) {
		    func = tre->findFunctionNode( parentPath, clone );
		    if ( func == 0 ) {
			doc.location().warning(tr("Cannot tie this documentation to anything"));
		    } else {
			func->borrowParameterNames( clone );
			nodes.append( func );
			docs.append( doc );
		    }
		    delete clone;		
		}
	    } else {
		QStringList::ConstIterator a = args.begin();
		while ( a != args.end() ) {
		    Doc nodeDoc = doc;
		    Node *node = processTopicCommand( nodeDoc, command, *a );
		    if ( node != 0 ) {
			nodes.append( node );
			docs.append( nodeDoc );
		    }
		    ++a;
		}
	    }

	    NodeList::Iterator n = nodes.begin();
	    QList<Doc>::Iterator d = docs.begin();
	    while ( n != nodes.end() ) {
		processOtherMetaCommands( *d, *n );
		(*n)->setDoc( *d );
		if ((*n)->isInnerNode() && ((InnerNode *)*n)->includes().isEmpty())
		    ((InnerNode *)*n)->addInclude((*n)->location().fileName());
		++d;
		++n;
	    }
	} else {
	    QStringList parentPath;
	    FunctionNode *clone;
	    FunctionNode *node = 0;

	    if ( matchFunctionDecl(0, &parentPath, &clone) ) {
		/*
		  The location of the definition is more interesting
		  than that of the declaration. People equipped with
		  a sophisticated text editor can respond to warnings
		  concerning undocumented functions very quickly.

		  Signals are implemented in uninteresting files
		  generated by moc.
		*/
		node = tre->findFunctionNode( parentPath, clone );
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

bool CppCodeParser::makeFunctionNode(const QString& synopsis, QStringList *parentPathPtr,
				     FunctionNode **funcPtr)
{
    Tokenizer *outerTokenizer = tokenizer;
    int outerTok = tok;

    Location loc;
    StringTokenizer stringTokenizer( loc, synopsis.latin1(),
				     synopsis.length() );
    tokenizer = &stringTokenizer;
    readToken();

    bool ok = matchFunctionDecl( 0, parentPathPtr, funcPtr );
    // potential memory leak with funcPtr

    tokenizer = outerTokenizer;
    tok = outerTok;

    return ok;
}

void CppCodeParser::parseQiteratorDotH(const Location &location, const QString &filePath,
				       Tree * /* tree */)
{
    QFile file(filePath);
    if (!file.open(IO_ReadOnly))
	return;

    QString text = file.readAll();
    text.remove("\r");
    text.replace("\\\n", "");
    QStringList lines = text.split("\n");
    lines = lines.find("Q_DECLARE");
    lines.replace(QRegExp("#define Q[A-Z_]*\\(C\\)"), "");

    if (lines.size() == 2) {
        linearIteratorDefinition = lines[0];
        associativeIteratorDefinition = lines[1];
    } else {
	location.warning(tr("The qiterator.h hack failed"));
    }
}

void CppCodeParser::instantiateIteratorMacro(const QString &container, const QString &includeFile,
					     const QString &macroDef, Tree *tree)
{
    QString resultingCode = macroDef;
    resultingCode.replace(QRegExp("\\bC\\b"), container);
    resultingCode.replace(QRegExp("\\s*##\\s*"), "");

    Location loc(includeFile);   // hack to get the include file for free
    StringTokenizer stringTokenizer(loc, resultingCode.latin1(), resultingCode.length());
    tokenizer = &stringTokenizer;
    readToken();
    matchDeclList(tree->root());
}
