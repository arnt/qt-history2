/*
  cppparser.cpp
*/

#include <qfile.h>
#include <qregexp.h>

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "binarywriter.h"
#include "config.h"
#include "decl.h"
#include "doc.h"
#include "emitter.h"
#include "messages.h"
#include "stringset.h"
#include "tokenizer.h"

/* qmake ignore Q_OBJECT */

/*
  What is parsing? Even though source files and header files are
  written in the same language (C++), qdoc distinguishes them. It
  first reads the header files using a parser that looks for
  declarations and class definitions. Then it reads the source files
  using another parser that looks for slashasterbang comments. Both
  parsers have much in common, but they are also quite different. For
  that reason, they are tangled here in one file.

  The two parsers are quite peculiar. They look like typical
  recursive-descent parsers (supposedly one of the least maintainable
  and most hard to read parsing techniques). However, these beasts
  have very little to do with Standard C++. They parse something that
  could be called Intuitive C++, or ad hoc C++. They are quite gross
  in the details. The header parser will try to parse anything that
  starts with 'class' as a class definition, until it meets something
  that doesn't fit. What does it do then? The answer is not backtrack.
  It just looks for something to hold on and goes on, without ever
  emitting a warning about syntax.
*/

/*
  Sets the address of a Doc. That information is used by the emitter
  to emit some index file, and by the Doc itself to know who it is.
*/
static void setLink( DocEmitter *emitter, Doc *doc, const QString& link,
		     const QString& text )
{
    if ( !doc->isInternal() && !doc->isObsolete() )
	emitter->addLink( link, text );
    doc->setLink( link, text );
}

/*
  The first part of the file contains functions and global variables used by
  both the C++ header file parser and the C++ source file parser.
*/

enum State { State_Normal, State_InSignals, State_InSlots };

static Tokenizer *yyTokenizer;
static int yyTok;
static Decl *yyLastDecl;
static State yyState;

static int getToken()
{
    return yyTokenizer->getToken();
}

static bool match( int tok )
{
    bool matches = ( yyTok == tok );
    if ( matches )
	yyTok = getToken();
    return matches;
}

static bool matchTemplateAngles( CodeChunk *type = 0 )
{
    bool matches = ( yyTok == Tok_LeftAngle );
    if ( matches ) {
	int depth = 0;
	do {
	    if ( yyTok == Tok_LeftAngle )
		depth++;
	    else if ( yyTok == Tok_RightAngle )
		depth--;
	    if ( type != 0 )
		type->append( yyTokenizer->lexeme() );
	    yyTok = getToken();
	} while ( depth > 0 && yyTok != Tok_Eoi );
    }
    return matches;
}

static bool matchTemplateHeader()
{
    yyTok = getToken();
    return matchTemplateAngles();
}

/*
  Tries to match a data type and possibly a variable name. The variable name
  belongs here because of cases such as 'char *xpm[]' and 'int (*f)(int)'. The
  inventors of C are to blame for these inside-out declarations.
*/
static bool matchDataType( CodeChunk *type, QString *var = 0 )
{
    static QRegExp varComment( QString("/\\*\\s([a-zA-Z_0-9]+)\\s\\*/") );

    /*
      This code is really hard to follow... sorry. The loop is there to match
      Alpha::Beta::Gamma::...::Omega.
    */
    while ( TRUE ) {
	bool virgin = TRUE;

	if ( yyTok != Tok_Ident ) {
	    /*
	      There is special processing for 'Foo::operator int()' and such
	      elsewhere. This is the only case where we return something with a
	      trailing gulbrandsen ('Foo::').
	    */
	    if ( yyTok == Tok_operator )
		return TRUE;

	    /*
	      People may write 'const unsigned short' or
	      'short unsigned const' or any other permutation.
	    */
	    while ( match(Tok_const) || match(Tok_volatile) )
		type->append( yyTokenizer->previousLexeme() );
	    while ( match(Tok_signed) || match(Tok_unsigned) ||
		    match(Tok_short) || match(Tok_long) ) {
		type->append( yyTokenizer->previousLexeme() );
		virgin = FALSE;
	    }
	    while ( match(Tok_const) || match(Tok_volatile) )
		type->append( yyTokenizer->previousLexeme() );

	    if ( match(Tok_Tilde) )
		type->append( yyTokenizer->previousLexeme() );
	}

	if ( virgin ) {
	    if ( match(Tok_Ident) )
		type->appendBase( yyTokenizer->previousLexeme() );
	    else if ( match(Tok_void) || match(Tok_int) || match(Tok_char) ||
		      match(Tok_double) || match(Tok_Ellipsis) )
		type->append( yyTokenizer->previousLexeme() );
	    else
		return FALSE;
	} else if ( match(Tok_int) || match(Tok_char) || match(Tok_double) ) {
	    type->append( yyTokenizer->previousLexeme() );
	}

	matchTemplateAngles( type );

	while ( match(Tok_const) || match(Tok_volatile) )
	    type->append( yyTokenizer->previousLexeme() );

	if ( match(Tok_Gulbrandsen) )
	    type->appendBase( yyTokenizer->previousLexeme() );
	else
	    break;
    }

    while ( match(Tok_Ampersand) || match(Tok_Aster) || match(Tok_const) )
	type->append( yyTokenizer->previousLexeme() );

    if ( match(Tok_LeftParenAster) ) {
	/*
	  A function pointer. This would be rather hard to handle without a
	  tokenizer hack, because a type can be followed with a left parenthesis
	  in some cases (e.g., 'operator int()'). The tokenizer recognizes '(*'
	  as a single token.
	*/
	type->append( yyTokenizer->previousLexeme() );
	type->appendHotspot();
	if ( var != 0 && match(Tok_Ident) )
	    *var = yyTokenizer->previousLexeme();
	if ( !match(Tok_RightParen) || yyTok != Tok_LeftParen )
	    return FALSE;
	type->append( yyTokenizer->previousLexeme() );

	int parenDepth0 = yyTokenizer->parenDepth();
	while ( yyTokenizer->parenDepth() >= parenDepth0 && yyTok != Tok_Eoi ) {
	    type->append( yyTokenizer->lexeme() );
	    yyTok = getToken();
	}
	if ( match(Tok_RightParen) )
	    type->append( yyTokenizer->previousLexeme() );
    } else {
	/*
	  The common case: Look for an optional identifier, then for some array
	  brackets.
	*/
	type->appendHotspot();

	if ( var != 0 ) {
	    if ( match(Tok_Ident) ) {
		*var = yyTokenizer->previousLexeme();
	    } else if ( match(Tok_Comment) ) {
		/*
		  A neat hack: Commented-out parameter names are recognized by
		  qdoc. It's impossible to illustrate here inside a C comment,
		  because it requires an asterslash. It's also impossible to
		  illustrate inside a C++ comment, because the explanation does
		  not fit on one line.
		*/
		if ( varComment.exactMatch(yyTokenizer->previousLexeme()) )
		    *var = varComment.cap( 1 );
	    }
	}

	if ( yyTok == Tok_LeftBracket ) {
	    int bracketDepth0 = yyTokenizer->bracketDepth();
	    while ( (yyTokenizer->bracketDepth() >= bracketDepth0 &&
		     yyTok != Tok_Eoi) ||
		    yyTok == Tok_RightBracket ) {
		type->append( yyTokenizer->lexeme() );
		yyTok = getToken();
	    }
	}
    }
    return TRUE;
}

static bool matchParameter( FunctionDecl *funcDecl )
{
    CodeChunk type;
    QString name;
    CodeChunk defaultValue;

    if ( !matchDataType(&type, &name) )
	return FALSE;
    match( Tok_Comment );
    if ( match(Tok_Equal) ) {
	int parenDepth0 = yyTokenizer->parenDepth();

	while ( yyTokenizer->parenDepth() >= parenDepth0 &&
		(yyTok != Tok_Comma ||
		 yyTokenizer->parenDepth() > parenDepth0) &&
		yyTok != Tok_Eoi ) {
	    defaultValue.append( yyTokenizer->lexeme() );
	    yyTok = getToken();
	}
    } else if ( name.startsWith(QString("Q_")) ) {
	// Q_NAME and Q_PARENT
	name = name.mid( 2 ).lower();
	defaultValue.append( QString("0") );
    }
    funcDecl->addParameter( Parameter(type, name, defaultValue) );
    return TRUE;
}

static bool matchFunctionDecl( Decl *context )
{
    CodeChunk returnType;
    QString name;

    bool stat = match( Tok_static );
    bool vir = match( Tok_virtual );

    if ( !matchDataType(&returnType) )
	return FALSE;

    bool gotFullName = FALSE;

    /*
      Here is the hack for 'Foo::operator int()'.
    */
    if ( yyTok == Tok_operator &&
	 (returnType.isEmpty() ||
	  returnType.toString().right(2) == QString("::")) ) {
	name = returnType.toString() + yyTokenizer->lexeme() + QChar( ' ' );
	returnType = CodeChunk();
	yyTok = getToken();

	CodeChunk restOfName;
	if ( !matchDataType(&restOfName) )
	    return FALSE;
	name += restOfName.toString();
	gotFullName = TRUE;
    }

    while ( match(Tok_Ident) ) {
	name += yyTokenizer->previousLexeme();

	matchTemplateAngles();

	if ( match(Tok_Gulbrandsen) ) {
	    name += yyTokenizer->previousLexeme();
	} else {
	    gotFullName = TRUE;
	    break;
	}
    }

    if ( !gotFullName ) {
	if ( match(Tok_operator) ) {
	    name += yyTokenizer->previousLexeme();
	    do {
		name += yyTokenizer->lexeme();
		yyTok = getToken();
	    } while ( yyTok != Tok_LeftParen && yyTok != Tok_Eoi );
	} else if ( yyTok == Tok_LeftParen ) {
	    name = returnType.toString();
	    returnType = CodeChunk();
	} else {
	    return FALSE;
	}
    }

    if ( !match(Tok_LeftParen) )
	return FALSE;

    FunctionDecl *funcDecl = new FunctionDecl( yyTokenizer->location(), name,
					       context, returnType );
    yyLastDecl = funcDecl;
    funcDecl->setStatic( stat );
    funcDecl->setVirtual( vir );
    if ( yyState == State_InSignals )
	funcDecl->setSignal( TRUE );
    else if ( yyState == State_InSlots )
	funcDecl->setSlot( TRUE );

    if ( yyTok != Tok_RightParen ) {
	do {
	    if ( !matchParameter(funcDecl) )
		return FALSE;
	} while ( match(Tok_Comma) );
    }
    if ( !match(Tok_RightParen) )
	return FALSE;

    funcDecl->setConst( match(Tok_const) );

    // pure virtual
    if ( match(Tok_Equal) )
	match( Tok_Number );

    if ( match(Tok_Colon) ) {
	while ( yyTok != Tok_LeftBrace && yyTok != Tok_Eoi )
	    yyTok = getToken();
    }

    if ( !match(Tok_Semicolon) && yyTok != Tok_Eoi ) {
	int braceDepth0 = yyTokenizer->braceDepth();

	if ( !match(Tok_LeftBrace) )
	    return FALSE;
	while ( yyTokenizer->braceDepth() >= braceDepth0 && yyTok != Tok_Eoi )
	    yyTok = getToken();
	match( Tok_RightBrace );
    }
    return TRUE;
}

/*
  The second part of the file constitutes the C++ header file parser proper.
*/

static bool matchBaseSpecifier( ClassDecl *classDecl )
{
    bool pub = ( yyTok != Tok_private && yyTok != Tok_protected );
    if ( yyTok == Tok_public || !pub )
	yyTok = getToken();

    CodeChunk superType;
    bool matches = matchDataType( &superType );
    if ( matches && pub )
	classDecl->addSuperType( superType );
    return matches;
}

static bool matchBaseList( ClassDecl *classDecl )
{
    while ( TRUE ) {
	if ( !matchBaseSpecifier(classDecl) )
	    return FALSE;
	if ( yyTok == Tok_LeftBrace )
	    return TRUE;
	if ( !match(Tok_Comma) )
	    return FALSE;
    }
}

static bool matchDeclList( Decl *context );

static bool matchClassDecl( Decl *context )
{
    bool isClass = ( yyTok == Tok_class );
    yyTok = getToken();
    if ( yyTok != Tok_Ident )
	return FALSE;
    while ( yyTok == Tok_Ident )
	yyTok = getToken();
    if ( yyTok != Tok_Colon && yyTok != Tok_LeftBrace )
	return FALSE;

    /*
      So far, so good. We have 'class Foo {' or 'class Foo :'. This is enough
      to recognize a class definition.
    */
    ClassDecl *classDecl = new ClassDecl( yyTokenizer->location(),
					  yyTokenizer->previousLexeme(),
					  context );
    classDecl->setHeaderFile( yyTokenizer->location().fileName() );
    yyLastDecl = classDecl;
    classDecl->setCurrentChildAccess( isClass ? Decl::Private : Decl::Public );

    if ( match(Tok_Colon) && !matchBaseList(classDecl) )
	return FALSE;
    if ( !match(Tok_LeftBrace) )
	return FALSE;

    State state0 = yyState;
    yyState = State_Normal;

    bool matches = ( matchDeclList(classDecl) && match(Tok_RightBrace) &&
		     match(Tok_Semicolon) );
    yyState = state0;
    return matches;
}

static bool matchEnumItem( EnumDecl *enumDecl )
{
    if ( !match(Tok_Ident) )
	return FALSE;

    QString name = yyTokenizer->previousLexeme();
    CodeChunk val;

    if ( match(Tok_Equal) ) {
	while ( yyTok != Tok_Comma && yyTok != Tok_RightBrace &&
		yyTok != Tok_Eoi ) {
	    val.append( yyTokenizer->lexeme() );
	    yyTok = getToken();
	}
    }
    enumDecl->addItem( new EnumItemDecl(yyTokenizer->location(), name, enumDecl,
					val) );
    return TRUE;
}

static bool matchEnumDecl( Decl *context )
{
    QString name;

    if ( !match(Tok_enum) )
	return FALSE;
    if ( match(Tok_Ident) )
	name = yyTokenizer->previousLexeme();

    if ( yyTok != Tok_LeftBrace )
	return FALSE;
    EnumDecl *enumDecl = new EnumDecl( yyTokenizer->location(), name, context );
    yyLastDecl = enumDecl;
    yyTok = getToken();

    if ( !matchEnumItem(enumDecl) )
	return FALSE;

    while ( match(Tok_Comma) ) {
	if ( !matchEnumItem(enumDecl) )
	    return FALSE;
    }
    return match( Tok_RightBrace ) && match( Tok_Semicolon );
}

static bool matchTypedefDecl( Decl *context )
{
    CodeChunk type;
    QString name;

    if ( !match(Tok_typedef) )
	return FALSE;
    if ( !matchDataType(&type, &name) )
	return FALSE;
    if ( !match(Tok_Semicolon) )
	return FALSE;

    yyLastDecl = new TypedefDecl( yyTokenizer->location(), name, context,
				  type );
    return TRUE;
}

static bool matchProperty( ClassDecl *classDecl )
{
    if ( !match(Tok_Q_PROPERTY) && !match(Tok_Q_OVERRIDE) )
	return FALSE;
    if ( !match(Tok_LeftParen) )
	return FALSE;

    QString name;
    CodeChunk type( yyTokenizer->lexeme() );

    yyTok = getToken();
    matchTemplateAngles( &type );

    if ( !match(Tok_Ident) )
	return FALSE;
    name = yyTokenizer->previousLexeme();

    PropertyDecl *propertyDecl = new PropertyDecl( yyTokenizer->location(),
						   name, classDecl, type );

    while ( yyTok != Tok_RightParen && yyTok != Tok_Eoi ) {
	if ( !match(Tok_Ident) )
	    return FALSE;
	QString key = yyTokenizer->previousLexeme();

	if ( !match(Tok_Ident) )
	    return FALSE;
	QString value = yyTokenizer->previousLexeme();

	if ( key == QString("READ") )
	    propertyDecl->setReadFunction( value );
	else if ( key == QString("WRITE") )
	    propertyDecl->setWriteFunction( value );
	else if ( key == QString("STORED") )
	    propertyDecl->setStored( value.lower() == QString("true") );
	else if ( key == QString("DESIGNABLE") )
	    propertyDecl->setDesignable( value.lower() == QString("true") );
	else if ( key == QString("RESET") )
	    propertyDecl->setResetFunction( value );
    }
    match( Tok_RightParen );
    return TRUE;
}

static bool matchDeclList( Decl *context )
{
    int braceDepth0 = yyTokenizer->braceDepth();
    if ( yyTok == Tok_RightBrace ) // prevents failure on empty body
	braceDepth0++;

    while ( yyTokenizer->braceDepth() >= braceDepth0 && yyTok != Tok_Eoi ) {
	switch ( yyTok ) {
	case Tok_class:
	case Tok_struct:
	case Tok_union:
	    matchClassDecl( context );
	    break;
	case Tok_template:
	    matchTemplateHeader();
	    break;
	case Tok_enum:
	    matchEnumDecl( context );
	    break;
	case Tok_typedef:
	    matchTypedefDecl( context );
	    break;
	case Tok_private:
	    yyTok = getToken();
	    yyState = match( Tok_slots ) ? State_InSlots : State_Normal;
	    if ( match(Tok_Colon) )
		context->setCurrentChildAccess( Decl::Private );
	    break;
	case Tok_protected:
	    yyTok = getToken();
	    yyState = match( Tok_slots ) ? State_InSlots : State_Normal;
	    if ( match(Tok_Colon) )
		context->setCurrentChildAccess( Decl::Protected );
	    break;
	case Tok_public:
	    yyTok = getToken();
	    yyState = match(Tok_slots) ? State_InSlots : State_Normal;
	    if ( match(Tok_Colon) )
		context->setCurrentChildAccess( Decl::Public );
	    break;
	case Tok_signals:
	    yyTok = getToken();
	    if ( match(Tok_Colon) ) {
		yyState = State_InSignals;
		context->setCurrentChildAccess( Decl::Public );
	    }
	    break;
	case Tok_Q_OBJECT:
	    yyTok = getToken();
	    if ( yyTok == Tok_Semicolon )
		warning( 2, yyTokenizer->location(),
			 "Needless ';' after 'Q_OBJECT' in C++ source" );
	    break;
	case Tok_Q_ENUMS:
	    yyTok = getToken();
	    while ( yyTok != Tok_RightParen && yyTok != Tok_Eoi )
		yyTok = getToken();
	    match( Tok_RightParen );
	    break;
	case Tok_Q_OVERRIDE:
	case Tok_Q_PROPERTY:
	    if ( context->kind() == Decl::Class )
		matchProperty( (ClassDecl *) context);
	    else
		yyTok = getToken();
	    break;
	default:
	    if ( !matchFunctionDecl(context) ) {
		while ( yyTok != Tok_Eoi &&
			(yyTokenizer->braceDepth() > braceDepth0 ||
			 !match(Tok_Semicolon)) )
		    yyTok = getToken();
	    }
	}
    }
    yyState = State_Normal;
    return TRUE;
}

void parseCppHeaderFile( DocEmitter *emitter, const QString& filePath )
{
    Location loc( filePath );
    FILE *in = fopen( QFile::encodeName(filePath), "r" );
    if ( in == 0 ) {
	syswarning( "Cannot open C++ header file '%s'", filePath.latin1() );
	return;
    }

    FileTokenizer tokenizer;
    tokenizer.start( loc, in );
    yyTokenizer = &tokenizer;
    yyTok = getToken();
    yyLastDecl = 0;
    yyState = State_Normal;

    matchDeclList( emitter->rootDecl() );

    yyTokenizer = 0;
    tokenizer.stop();
    fclose( in );
}

/*
  The third part of the file constitutes the C++ source file parser proper.
*/

static void matchDocsAndStuff( DocEmitter *emitter )
{
    Decl *decl = 0;

    while ( yyTok != Tok_Eoi ) {
	if ( yyTok == Tok_Doc ) {
	    QString t = yyTokenizer->lexeme();
	    Location loc = yyTokenizer->location();
	    loc.advance( '/' );
	    loc.advance( '*' );
	    loc.advance( '!' );

	    Doc *doc = Doc::create( loc, t.mid(3, t.length() - 5) );
	    bool deleteDoc = TRUE;
	    yyTok = getToken();

	    /*
	      Have you read tokenizer.h? If so, you must have noticed the two
	      concrete subclasses of Tokenizer, namely FileTokenizer and
	      StringTokenizer. A FileTokenizer is obviously useful for parsing
	      source files. If you don't understand the need for a
	      StringTokenizer yet, consider docs containing a function
	      prototype, like

		  \fn int QWidget::width() const

	      The comment is first slurped by a FileTokenizer. Then
	      Doc::create() discovers the '\fn' line and makes it available
	      throught FnDoc::prototype(). Finally the C++ source file parser is
	      tweaked to use a StringTokenizer instead of the usual
	      FileTokenizer.
	    */

	    if ( doc->kind() == Doc::Fn ) {
		FnDoc *fn = (FnDoc *) doc;
		StringTokenizer stringTokenizer;
		Tokenizer *tokenizer0 = yyTokenizer;
		int tok0 = yyTok;

		if ( !fn->prototype().isEmpty() ) {
		    stringTokenizer.start( fn->location(),
					   fn->prototype().latin1(),
					   fn->prototype().length() );
		    yyTokenizer = &stringTokenizer;
		    yyTok = getToken();
		}

		Decl *relates = 0;
		Decl *root = 0;

		if ( !fn->relates().isEmpty() ) {
		    relates = emitter->resolveMangled( fn->relates() );
		    if ( relates != 0 && relates->kind() == Decl::Class ) {
			root = relates->rootContext();
		    } else {
			warning( 1, fn->location(),
				 "Class '%s' specified with '\\relates' not"
				 " found", fn->relates().latin1() );
			relates = 0;
		    }
		}

		if ( matchFunctionDecl(root) ) {
		    if ( root == 0 ) {
			decl = emitter->resolveMangled(
				yyLastDecl->mangledName() );
			if ( decl != 0 && decl->kind() != Decl::Function )
			    decl = 0;
		    } else {
			decl = yyLastDecl;
			decl->setRelates( relates );
		    }

		    if ( decl != 0 ) {
			decl->setDoc( fn );
			setLink( emitter, fn,
				 config->classRefHref(
					 decl->relatesContext()->name()) +
					 QChar('#') + decl->ref(),
				 decl->fullName() );
			deleteDoc = FALSE;

			((FunctionDecl *) decl)->borrowParameterNames(
				((FunctionDecl *) yyLastDecl)->parameters()
				.begin() );

			/*
			  Check unexisting parameters now. Check undocumented
			  parameters elsewhere, when we know who overloads who.
			*/
			StringSet diff;
			StringSet::ConstIterator s;

			diff = difference( fn->documentedParameters(),
				((FunctionDecl *) decl)->parameterNames() );
			s = diff.begin();
			while ( s != diff.end() ) {
			    warning( 3, fn->location(),
				     "No such parameter '%s'", (*s).latin1() );
			    ++s;
			}
		    } else if ( !fn->prototype().isEmpty() ) {
			warning( 2, fn->location(),
				 "Function '%s' specified with '\\fn' not"
				 " found",
				 yyLastDecl->fullMangledName().latin1() );
			if ( yyLastDecl->fullMangledName()
			     .contains(QChar(':')) == 0 )
			    warning( 2, fn->location(),
				     "(the class name appears to be missing)" );
		    }

		    if ( relates == 0 ) {
			delete yyLastDecl;
			yyLastDecl = 0;
		    }
		} else {
		    warning( 3, fn->location(),
			     "Cannot tie this documentation to anything" );
		}

		if ( !fn->prototype().isEmpty() ) {
		    yyTokenizer = tokenizer0;
		    yyTok = tok0;
		    stringTokenizer.stop();
		}
	    } else if ( doc->kind() == Doc::Class ) {
		ClassDoc *cl = (ClassDoc *) doc;
		decl = emitter->resolveMangled( cl->name() );
		if ( decl != 0 && decl->kind() == Decl::Class ) {
		    decl->setDoc( doc );
		    setLink( emitter, cl, config->classRefHref(decl->name()),
			     decl->name() );
		    deleteDoc = FALSE;
		} else {
		    warning( 1, doc->location(),
			     "Class '%s' specified with '\\class' not found",
			     cl->name().latin1() );
		}
	    } else if ( doc->kind() == Doc::Enum ) {
		EnumDoc *en = (EnumDoc *) doc;
		decl = emitter->resolveMangled( en->name() );
		if ( decl != 0 && (decl->kind() == Decl::Enum ||
				   decl->kind() == Decl::Typedef) ) {
		    decl->setDoc( en );
		    setLink( emitter, en,
			     config->classRefHref(decl->context()->name()) +
				     QChar('#') + decl->mangledName(),
			     decl->fullName() );
		    deleteDoc = FALSE;

		    if ( decl->kind() == Decl::Enum ) {
			QMap<QString, QString> itemValues;
			StringSet itemNames;
			EnumDecl::ItemIterator ii;

			ii = ((EnumDecl *) decl)->itemBegin();
			while ( ii != ((EnumDecl *) decl)->itemEnd() ) {
			    itemValues.insert( (*ii)->name(),
					       (*ii)->value().toString() );
			    itemNames.insert( (*ii)->name() );
			    ++ii;
			}

			StringSet diff;
			StringSet::ConstIterator s;

			diff = difference( en->documentedValues(), itemNames );
			s = diff.begin();
			while ( s != diff.end() ) {
			    warning( 3, en->location(),
				     "No such enum value '%s'", (*s).latin1() );
			    ++s;
			}

			// the new syntax is not widely adopted yet
			if ( !en->documentedValues().isEmpty() ) {
			    diff = difference( itemNames,
					       en->documentedValues() );
			    s = diff.begin();
			    while ( s != diff.end() ) {
				// it's OK to have Foo = Bar and only Bar
				// documented
				if ( !en->documentedValues()
					     .contains(itemValues[*s]) )
				    warning( 3, en->location(),
					     "Undocumented enum value '%s'",
					     (*s).latin1() );
				++s;
			    }
			}
		    }
		} else {
		    warning( 1, doc->location(),
			     "Enum or typedef '%s' specified with '\\enum' not"
			     " found",
			     en->name().latin1() );
		}
	    } else if ( doc->kind() == Doc::Property ) {
		PropertyDoc *pd = (PropertyDoc *) doc;
		decl = emitter->resolveMangled( pd->name() );
		if ( decl != 0 && decl->kind() == Decl::Property ) {
		    PropertyDecl *propDecl = (PropertyDecl *) decl;
		    decl->setDoc( pd );
		    deleteDoc = FALSE;

		    if ( pd->isObsolete() && propDecl->designable() )
			warning( 1, doc->location(),
				 "Obsolete property '%s' should not be"
				 " designable",
				 pd->name().latin1() );
		} else {
		    warning( 1, doc->location(),
			     "Property '%s' specified with '\\property' not"
			     " found",
			     pd->name().latin1() );
		}
	    } else if ( doc->kind() == Doc::Page ) {
		PageDoc *pa = (PageDoc *) doc;
		setLink( emitter, pa, pa->fileName(), pa->title() );
		emitter->addPage( pa );
		deleteDoc = FALSE;
	    } else if ( doc->kind() == Doc::Base64 ) {
		Base64Doc *ba = (Base64Doc *) doc;
		BinaryWriter out( ba->fileName() );
		ba->print( out );
	    } else if ( doc->kind() == Doc::Plainpage ) {
		PlainpageDoc *ba = (PlainpageDoc *) doc;
		BinaryWriter out( ba->fileName() );
		ba->print( out );
	    } else if ( doc->kind() == Doc::Defgroup ) {
		DefgroupDoc *df = (DefgroupDoc *) doc;
		emitter->addGroup( df );
		setLink( emitter, df, df->fileName(), df->name() );
		deleteDoc = FALSE;
	    } else if ( doc->kind() == Doc::Example ) {
		emitter->addExample( (ExampleDoc *) doc );
		deleteDoc = FALSE;
	    }

	    if ( deleteDoc )
		delete doc;
	    else
		emitter->addGroupie( doc );
	} else {
	    if ( matchFunctionDecl((FunctionDecl *) 0) ) {
		decl = emitter->resolveMangled( yyLastDecl->mangledName() );
		// signals are defined in MOC files
		if ( decl != 0 && decl->kind() == Decl::Function &&
		     !((FunctionDecl *) decl)->isSignal() )
		    decl->setLocation( yyLastDecl->location() );
	    } else {
		while ( yyTok != Tok_Doc && yyTok != Tok_Eoi &&
			(yyTokenizer->braceDepth() > 0 ||
			 !match(Tok_Semicolon)) )
		    yyTok = getToken();
	    }
	}
    }
}

void parseCppSourceFile( DocEmitter *emitter, const QString& filePath )
{
    Location loc( filePath );
    FILE *in = fopen( QFile::encodeName(filePath), "r" );
    if ( in == 0 ) {
	syswarning( "Cannot open C++ source file '%s'", filePath.latin1() );
	return;
    }

    FileTokenizer tokenizer;
    tokenizer.start( loc, in );
    yyTokenizer = &tokenizer;
    yyTok = getToken();
    yyLastDecl = 0;
    yyState = State_Normal;

    matchDocsAndStuff( emitter );

    yyTokenizer = 0;
    tokenizer.stop();
    fclose( in );
}
