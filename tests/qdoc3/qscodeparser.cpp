/*
  qscodeparser.cpp
*/

#include <qfile.h>
#include <qregexp.h>

#include "config.h"
#include "qscodeparser.h"
#include "tokenizer.h"
#include "tree.h"

#define CONFIG_QUICK                "quick"

#define CONFIG_REPLACES             "replaces"

#define COMMAND_FILE                Doc::alias( "file" )
#define COMMAND_GROUP               Doc::alias( "group" )
#define COMMAND_MODULE              Doc::alias( "module" )
#define COMMAND_PAGE                Doc::alias( "page" )
#define COMMAND_QUICKCLASS          Doc::alias( "quickclass" )
#define COMMAND_QUICKFN             Doc::alias( "quickfn" )
#define COMMAND_QUICKIFY            Doc::alias( "quickify" )
#define COMMAND_QUICKPROPERTY       Doc::alias( "quickproperty" )
#define COMMAND_REPLACE             Doc::alias( "replace" )

static QString balancedParens = "(?:[^()]+|\\([^()]*\\))*";

int QsCodeParser::tabSize;
QValueList<QRegExp> QsCodeParser::replaceBefores;
QStringList QsCodeParser::replaceAfters;

QsCodeParser::QsCodeParser( Tree *cppTree )
    : cppTre( cppTree ), qsTre( 0 ), replaceRegExp( "/(.+)/([^/]*)/" )
{
}

QsCodeParser::~QsCodeParser()
{
}

void QsCodeParser::initializeParser( const Config& config )
{
    CppCodeParser::initializeParser( config );

    tabSize = config.getInt( CONFIG_TABSIZE );

    QString quickDotReplaces = CONFIG_QUICK + Config::dot + CONFIG_REPLACES;
    QStringList replaces = config.getStringList( CONFIG_QUICK + Config::dot +
						 CONFIG_REPLACES );
    QStringList::ConstIterator r = replaces.begin();
    while ( r != replaces.end() ) {
	if ( replaceRegExp.exactMatch(*r) ) {
	    QRegExp before( replaceRegExp.cap(1) );
	    before.setMinimal( TRUE );
	    QString after = replaceRegExp.cap( 2 );

	    if ( before.isValid() ) {
		replaceBefores << before;
		replaceAfters << after;
	    } else {
		config.lastLocation().warning(
			tr("Invalid regular expression '%1'")
			.arg(before.pattern()) );
	    }
	} else {
	    config.lastLocation().warning( tr("Bad syntax in '%1'")
					   .arg(quickDotReplaces) );
	}
	++r;
    }
}

void QsCodeParser::terminateParser()
{
    tabSize = 0;
    replaceBefores.clear();
    replaceAfters.clear();
    CppCodeParser::terminateParser();
}

QString QsCodeParser::language()
{
    return "Qt Script";
}

QString QsCodeParser::headerFileNameFilter()
{
    return "*";
}

QString QsCodeParser::sourceFileNameFilter()
{
    return "*.qs *.qsd";
}

void QsCodeParser::parseHeaderFile( const Location& location,
				    const QString& filePath, Tree *tree )
{
    qsTre = tree;

    FILE *in = fopen( QFile::encodeName(filePath), "r" );
    if ( in == 0 ) {
	location.error( tr("Cannot open Qt Script class list '%1'")
			.arg(filePath) );
	return;
    }

    NodeList quickClasses;

    Location fileLocation( filePath );
    FileTokenizer fileTokenizer( fileLocation, in );
    int tok = fileTokenizer.getToken();
    while ( tok != Tok_Eoi ) {
	if ( tok == Tok_Ident ) {
	    ClassNode *quickClass = new ClassNode( qsTre->root(),
						   fileTokenizer.lexeme() );
	    quickClass->setLocation( fileTokenizer.location() );
	    quickClasses << quickClass;
	} else {
	    fileTokenizer.location().error( tr("Unexpected token '%1' in Qt"
					       " Script class list")
					    .arg(fileTokenizer.lexeme()) );
	    break;
	}
	tok = fileTokenizer.getToken();
    }
    fclose( in );

    NodeList::ConstIterator c = quickClasses.begin();
    while ( c != quickClasses.end() ) {
	quickifyClass( (ClassNode *) *c );
	++c;
    }
}

void QsCodeParser::parseSourceFile( const Location& location,
				    const QString& filePath, Tree *tree )
{
    qsTre = tree;
    CppCodeParser::parseSourceFile( location, filePath, tree );
}

FunctionNode *QsCodeParser::findFunctionNode( const QString& synopsis,
					      Tree *tree )
{
    QStringList path;
    FunctionNode *clone;
    FunctionNode *func = 0;

    if ( makeFunctionNode(synopsis, &path, &clone) ) {
	func = tree->findFunctionNode( path, clone );
	delete clone;
    }
    return func;
}

Set<QString> QsCodeParser::topicCommands()
{
    return Set<QString>() << COMMAND_FILE << COMMAND_GROUP << COMMAND_MODULE
			  << COMMAND_PAGE << COMMAND_QUICKCLASS
			  << COMMAND_QUICKFN << COMMAND_QUICKPROPERTY;
}

Node *QsCodeParser::processTopicCommand( const Doc& doc, const QString& command,
					 const QString& arg )
{
    if ( command == COMMAND_QUICKFN ) {
	QStringList path;
	FunctionNode *clone;

	if ( makeFunctionNode(arg, &path, &clone) ) {
	    FunctionNode *quickFunc = qsTre->findFunctionNode( path, clone );
	    if ( quickFunc == 0 ) {
		doc.location().warning( tr("Cannot resolve '%1' specified with"
					   " '\\%2'")
					.arg(arg).arg(command) );
	    } else {
		quickFunc->borrowParameterNames( clone );
		setQuickDoc( quickFunc, doc );
	    }
	    delete clone;
	} else {
	    doc.location().warning( tr("Cannot resolve '%1' specified with"
				       " '\\%2'")
				    .arg(arg).arg(command) );
	}
	return 0;
    } else if ( command == COMMAND_QUICKPROPERTY ) {
	QStringList path = QStringList::split( ".", arg );
	PropertyNode *quickProperty =
		(PropertyNode *) qsTre->findNode( path, Node::Property );
	if ( quickProperty == 0 ) {
	    doc.location().warning( tr("Cannot resolve '%1' specified with"
				       " '\\%2'")
				    .arg(arg).arg(command) );
	} else {
	    setQuickDoc( quickProperty, doc );
	}
	return 0;
    } else if ( command == COMMAND_QUICKCLASS ) {
	QStringList path = QStringList::split( ".", arg );
	ClassNode *quickClass =
		(ClassNode *) qsTre->findNode( path, Node::Class );
	if ( quickClass == 0 ) {
	    doc.location().warning( tr("Cannot resolve '%1' specified with"
				       " '\\%2'")
				    .arg(arg).arg(command) );
	} else {
	    setQuickDoc( quickClass, doc );	
	}
	return 0;
    } else {
	return CppCodeParser::processTopicCommand( doc, command, arg );
    }
}

Set<QString> QsCodeParser::otherMetaCommands()
{
    return commonMetaCommands() << COMMAND_QUICKIFY << COMMAND_REPLACE;
}

ClassNode *QsCodeParser::tryClass( const QString& className )
{
    return (ClassNode *) cppTre->findNode( className, Node::Class );
}

void QsCodeParser::extractRegExp( const QRegExp& regExp, QString& source,
				  const Doc& doc )
{
    QRegExp blankLineRegExp(
	    "[ \t]*(?:\n(?:[ \t]*\n)+[ \t]*|[ \n\t]*\\\\code|"
	    "\\\\endcode[ \n\t]*)" );
    QStringList paras = QStringList::split( blankLineRegExp,
					    source.stripWhiteSpace() );
    paras = paras.grep( regExp );
    if ( paras.count() == 0 ) {
	doc.location().warning( tr("Cannot find regular expression '%1'")
				.arg(regExp.pattern()) );
    } else if ( paras.count() > 1 ) {
	doc.location().warning( tr("Regular rexpression '%1' matches multiple"
				   "times").arg(regExp.pattern()) );
    } else {
	source = paras.first() + "\n\n";
    }
}

void QsCodeParser::extractTarget( const QString& target, QString& source,
				  const Doc& doc )
{
    QRegExp targetRegExp(
	    "(\\\\target\\s+(\\S+)[^\n]*\n"
	    "(?:(?!\\s*\\\\code)[^\n]+\n|\\s*\\\\code.*\\\\endcode\\s*\n)*)"
	    "(?:\\s*\n|[^\n]*$)" );
    targetRegExp.setMinimal( TRUE );

    int pos = 0;
    while ( (pos = source.find(targetRegExp, pos)) != -1 ) {
	if ( targetRegExp.cap(2) == target ) {
	    source = targetRegExp.cap( 1 ) + "\n\n";
	    return;
	}
	pos += targetRegExp.matchedLength();
    }
    doc.location().warning( tr("Cannot find target '%1'").arg(target) );
}

void QsCodeParser::applyReplacementList( QString& source, const Doc& doc )
{
    QStringList args = doc.metaCommandArgs( COMMAND_REPLACE );
    QStringList::ConstIterator a = args.begin();
    while ( a != args.end() ) {
	if ( replaceRegExp.exactMatch(*a) ) {
	    QRegExp before( replaceRegExp.cap(1) );
	    before.setMinimal( TRUE );
	    QString after = replaceRegExp.cap( 2 );

	    if ( before.isValid() ) {
		uint oldLen = source.length();
		source.replace( before, after );

		// this condition is sufficient but not necessary
		if ( oldLen == source.length() && !source.contains(after) )
		    doc.location().warning(
			    tr("Regular expression '%1' did not match anything")
			    .arg(before.pattern()) );
	    } else {
		doc.location().warning(
			tr("Invalid regular expression '%1'")
			.arg(before.pattern()) );
	    }
	} else {
	    doc.location().warning( tr("Bad syntax in '\\%1'")
				    .arg(COMMAND_REPLACE) );
	}
	++a;
    }
}

void QsCodeParser::quickifyClass( ClassNode *quickClass )
{
    QString qtClassName = quickClass->name();
    QString bare = quickClass->name();
    if ( quickClass->name() != "Qt" ) {
	if ( quickClass->name().startsWith("Q") ) {
	    bare = bare.mid( 1 );
	} else {
	    qtClassName.prepend( "Q" );
	}
    }

qDebug( "Quickifying '%s'", qtClassName.latin1() );

    ClassNode *qtClass = 0;
    ClassNode *wrapperClass = 0;

    if ( (wrapperClass = tryClass("Quick" + bare + "Interface")) != 0 ) {
	qtClass = tryClass( qtClassName );
    } else if ( (wrapperClass = tryClass("Quick" + bare)) != 0 ) {
	qtClass = tryClass( qtClassName );
	if ( qtClass == 0 ) {
	    qtClass = wrapperClass;
	    wrapperClass = 0;
	}
    } else if ( (wrapperClass = tryClass("Quick" + bare + "Ptr")) != 0 ) {
	QRegExp ptrToQtType( "(Q[A-Za-z0-9_]+)\\s*\\*" );
	FunctionNode *ctor =
		wrapperClass->findFunctionNode( wrapperClass->name() );
	if ( ctor != 0 && !ctor->parameters().isEmpty() &&
	     ptrToQtType.exactMatch(ctor->parameters().first().leftType()) )
	    qtClassName = ptrToQtType.cap( 1 );
	qtClass = tryClass( qtClassName );
    } else if ( (wrapperClass = tryClass("Q" + bare + "Ptr")) != 0 ) {
	qtClass = tryClass( qtClassName );
    } else {
	qtClass = tryClass( qtClassName );
    }

    if ( qtClass == 0 ) {
	if ( wrapperClass == 0 ) {
	    quickClass->location().warning( tr("Cannot find Qt class '%1'")
					    .arg(qtClassName) );
	} else {
	    quickClass->location().warning( tr("Cannot find Qt class '%1'"
					       " wrapped by '%2'")
					    .arg(qtClassName)
					    .arg(wrapperClass->name()) );
	}
	return;
    }

    NodeList children = qtClass->childNodes();
    if ( wrapperClass != 0 )
	children += wrapperClass->childNodes();

    QMap<QString, int> blackList;
    for ( int pass = 0; pass < 2; pass++ ) {
	NodeList::ConstIterator c = children.begin();
	while ( c != children.end() ) {
	    if ( (*c)->access() == Node::Public &&
		 (*c)->status() == Node::Commendable ) {
		if ( pass == 0 ) {
		    if ( (*c)->type() == Node::Enum ) {
			EnumNode *enume = (EnumNode *) *c;
			quickifyEnum( quickClass, enume );
		    } else if ( (*c)->type() == Node::Property ) {
			PropertyNode *property = (PropertyNode *) *c;
			quickifyProperty( quickClass, qtClass, property );
			blackList.insert( property->getter(), 0 );
			blackList.insert( property->setter(), 0 );
			blackList.insert( property->resetter(), 0 );
		    }
		} else {
		    if ( (*c)->type() == Node::Function &&
			  !blackList.contains((*c)->name()) )  {
			FunctionNode *func = (FunctionNode *) *c;
			quickifyFunction( quickClass, qtClass, func );
		    }
		}
	    }
	    ++c;
	}
    }
    setQtDoc( quickClass, qtClass->doc() );
}

void QsCodeParser::quickifyEnum( ClassNode *quickClass, EnumNode *enume )
{
    EnumNode *quickEnum = new EnumNode( quickClass, enume->name() );
    quickEnum->setLocation( enume->location() );

    QValueList<EnumItem>::ConstIterator it = enume->items().begin();
    while ( it != enume->items().end() ) {
	QString name = (*it).name();
	QString value = (*it).value();
	// ### drop value in most cases
	quickEnum->addItem( EnumItem(name, value) );
	++it;
    }
    setQtDoc( quickEnum, enume->doc() );
}

void QsCodeParser::quickifyFunction( ClassNode *quickClass, ClassNode *qtClass,
				     FunctionNode *func )
{
    if ( func->metaness() != FunctionNode::Plain ) {
	FunctionNode *quickFunc = new FunctionNode( quickClass, func->name() );
	quickFunc->setLocation( func->location() );
	quickFunc->setReturnType( quickifiedDataType(func->returnType()) );
	if ( func->metaness() == FunctionNode::Signal )
	    quickFunc->setMetaness( FunctionNode::Signal );
	quickFunc->setOverload( func->isOverload() );

	QValueList<Parameter>::ConstIterator q = func->parameters().begin();
	while ( q != func->parameters().end() ) {
	    Parameter param( quickifiedDataType((*q).leftType(),
						(*q).rightType()),
			     "", (*q).name() );
	    quickFunc->addParameter( param );
	    ++q;
	}

	if ( func->doc().isEmpty() ) {
	    if ( func->parent() != (InnerNode *) qtClass ) {
		func = qtClass->findFunctionNode( func );
		if ( func != 0 )
		    setQtDoc( quickFunc, func->doc() );
	    }
	} else {
	    setQtDoc( quickFunc, func->doc() );
	}
    }
}

void QsCodeParser::quickifyProperty( ClassNode *quickClass,
				     ClassNode * /* qtClass */,
				     PropertyNode *property )
{
    PropertyNode *quickProperty =
	    new PropertyNode( quickClass, property->name() );
    quickProperty->setLocation( property->location() );
    quickProperty->setDataType( quickifiedDataType(property->dataType()) );
    quickProperty->setGetter( property->getter() );
    quickProperty->setSetter( property->setter() );
    quickProperty->setResetter( property->resetter() );
    quickProperty->setStored( property->isStored() );
    quickProperty->setDesignable( property->isDesignable() );

    setQtDoc( quickProperty, property->doc() );
}

QString QsCodeParser::quickifiedDataType( const QString& leftType,
					  const QString& /* rightType */ )
{
    QString s = leftType;

    if ( s.startsWith("const ") )
	s = s.mid( 6 );
    while ( s.endsWith("*") || s.endsWith("&") || s.endsWith(" ") )
	s.truncate( s.length() - 1 );

    switch ( s[0].unicode() ) {
    case 'Q':
	if ( s == "QCString" ) {
	    return "String";
	} else {
	    Node *node = qsTre->findNode( s, Node::Class );
	    if ( node == 0 )
		node = qsTre->findNode( s.mid(1), Node::Class );

	    if ( node == 0 ) {
		return "UNKNOWN";
	    } else {
		return node->name();
	    }
	}
	break;
    case 'b':
	if ( s == "bool" )
	    return "Boolean";
	break;
    case 'c':
	if ( s == "char" ) {
	    if ( leftType == "const char *" ) {
		return "String";
	    } else {
		return "Number";	    
	    }
	}
	break;
    case 'd':
	if ( s == "double" )
	    return "Number";
	break;
    case 'f':
	if ( s == "float" )
	    return "Number";
    case 'i':
	if ( s == "int" )
	    return "Number";
	break;
    case 'l':
	if ( s == "long" || s == "long int" || s == "long long" ||
	     s == "long long int" || s == "long double" )
	    return "Number";
	break;
    case 's':
	if ( s == "short" || s == "short int" || s == "signed char" ||
	     s == "signed short" || s == "signed short int" || s == "signed" ||
	     s == "signed int" || s == "signed long" || s == "signed long int" )
	    return "Number";
	break;
    case 'u':
	if ( s == "uchar" || s == "unsigned" || s == "unsigned char" ||
	     s == "ushort" || s == "unsigned short" ||
	     s == "unsigned short int" || s == "uint" || s == "unsigned int" ||
	     s == "ulong" || s == "unsigned long" || s == "unsigned long int" )
	    return "Number";
	break;
    case 'v':
	if ( s == "void" )
	    return "";
    }
    return s;
}

QString QsCodeParser::quickifiedCode( const QString& code )
{
    QRegExp funcRegExp(
	    "^([ \t]*)(?:[\\w<>,&*]+[ \t]+)+((?:\\w+::)*\\w+)\\((" +
	    balancedParens + ")\\)(?:[ \t]*const)?(?=[ \n\t]*\\{)" );
    QRegExp paramRegExp(
	     "\\s*(const\\s+)?([^\\s=]+)\\b[^=]*\\W(\\w+)\\s*(?:=.*)?" );
    QRegExp qtVarRegExp(
	     "^([ \t]*)(const[ \t]+)?Q([A-Z][A-Za-z_0-9]*)\\s+"
	     "([a-z][A-Za-z_0-9]*)\\s*(?:\\((" + balancedParens +
	     ")\\))?\\s*;" );
    QRegExp signalOrSlotRegExp(
	     "^(SIGNAL|SLOT)\\((" + balancedParens + ")\\)" );
    QString result;
    bool inString = FALSE;
    QChar quote;
    int i = 0;

    while ( i < (int) code.length() ) {
	if ( inString ) {
	    if ( code[i] == quote ) {
		result += code[i++];
		inString = FALSE;
	    } else if ( code[i] == '\\' ) {
		result += code[i++];
		result += code[i++];
	    } else {
		result += code[i++];
	    }
	} else {
	    bool newLine = ( i == 0 || code[i - 1] == '\n' );

	    if ( newLine && funcRegExp.search(code.mid(i)) != -1 ) {
		QString indent = funcRegExp.cap( 1 );
		QString name = funcRegExp.cap( 2 );
		// ### remove QRegExp cast once Mark has Qt 3.1 up and running
		name.replace( QRegExp("::"), "." );
		QStringList params =
			QStringList::split( ",", funcRegExp.cap(3) );
		QStringList::Iterator p = params.begin();
		while ( p != params.end() ) {
		    if ( paramRegExp.exactMatch(*p) ) {
#if 0
			bool isVar = paramRegExp.cap( 1 ).isEmpty();
#endif
			QString qtDataType = paramRegExp.cap( 2 );
			QString name = paramRegExp.cap( 3 );
			*p = "var " + name + " : " +
			     quickifiedDataType( qtDataType );
		    }
		    ++p;
		}
		result += indent + "function " + name + "( " +
			  params.join(", ") + " )";
		i += funcRegExp.matchedLength();
	    } else if ( newLine && qtVarRegExp.search(code.mid(i)) != -1 ) {
		QString indent = qtVarRegExp.cap( 1 );
		bool isVar = qtVarRegExp.cap( 2 ).isEmpty();
		QString quickClass = qtVarRegExp.cap( 3 );
		QString name = qtVarRegExp.cap( 4 );
		QString initializer = qtVarRegExp.cap( 5 ).simplifyWhiteSpace();
		result += indent + ( isVar ? "var " : "const " ) + name;
		if ( initializer.isEmpty() ) {
		    result += " = new " + quickClass + ";";
		} else {
		    result += " = new " + quickClass + "( " + initializer +
			      " );";
		}
		i += qtVarRegExp.matchedLength();
	    } else if ( code[i] == '"' || code[i] == '\'' ) {
		quote = code[i];
		result += code[i++];
		inString = TRUE;
	    } else if ( code[i] == 'F' && leftWordBoundary(code, i) &&
			code.mid(i, 5) == "FALSE" &&
			rightWordBoundary(code, i + 5) ) {
		result += "false";
		i += 5;
	    } else if ( code[i] == 'Q' && leftWordBoundary(code, i) ) {
		if ( code.mid(i - 4, 3) == "new" ) {
		    if ( code[i + 1].lower() == code[i + 1] ) {
			result += code[i++];
		    } else {
			i++;
		    }
		} else {
		    result += "var ";
		    while ( i < (int) code.length() &&
			    code[i].isLetterOrNumber() )
			i++;
		    while ( i < (int) code.length() &&
			    !code[i].isLetterOrNumber() && code[i] != '\n' )
			i++;
		}
	    } else if ( code[i] == 'S' && leftWordBoundary(code, i) &&
			(code.mid(i, 7) == "SIGNAL(" ||
			 code.mid(i, 5) == "SLOT(") ) {
		if ( signalOrSlotRegExp.search(code, i
#if QT_VERSION >= 0x030100
						      , QRegExp::CaretAtOffset
#endif
					 ) == i ) {
		    result += signalOrSlotRegExp.cap( 1 ) + "(\"" +
			      signalOrSlotRegExp.cap( 2 ) + "\")";
		    i += signalOrSlotRegExp.matchedLength();
		}
	    } else if ( code[i] == 'T' && leftWordBoundary(code, i) &&
			code.mid(i, 4) == "TRUE" &&
			rightWordBoundary(code, i + 4) ) {
		result += "true";
		i += 4;
	    } else if ( code[i] == '/' && code[i + 1] == '/' ) {
		int numSpaces = columnForIndex( code, i ) -
				columnForIndex( result, result.length() );
		if ( numSpaces > 0 ) {
		    while ( numSpaces-- > 0 )
			result += " ";
		} else if ( numSpaces < 0 ) {
		    while ( numSpaces++ < 0 && result.right(1) == " " )
			result.truncate( result.length() - 1 );
		}
		while ( i < (int) code.length() && code[i] != '\n' )
		    result += code[i++];
	    } else if ( (code[i] == ':' && code[i + 1] == ':') ||
			(code[i] == '-' && code[i + 1] == '>') ) {
		result += '.';
		i += 2;
	    } else {
		result += code[i++];
	    }
	}
    }
    return result;
}

QString QsCodeParser::quickifiedDoc( const QString& source )
{
    QString result;
    int i = 0;

    while ( i < (int) source.length() ) {
	if ( leftWordBoundary(source, i) ) {
	    if ( source[i] == 'C' && source.mid(i, 7) == "CString" ) {
		i++;
	    } else if ( source[i] == 'T' && source.mid(i, 4) == "TRUE" &&
			rightWordBoundary(source, i + 4) ) {
		result += "\\c{true}";
		i += 4;
	    } else if ( source[i] == 'F' && source.mid(i, 5) == "FALSE" &&
			rightWordBoundary(source, i + 5) ) {
		result += "\\c{false}";
		i += 5;
	    } else if ( source[i] == 'c' && source.mid(i, 6) == "const " ) {
		i += 6;
	    } else {
		result += source[i++];
	    }
	} else if ( (source[i] == ':' && source[i + 1] == ':') ||
		    (source[i] == '-' && source[i + 1] == '>') ) {
	    result += '.';
	    i += 2;
	} else if ( source[i] == '\\' ) {
	    if ( source.mid(i, 5) == "\\code" ) {
		do {
		    result += source[i++];
		} while ( source[i - 1] != '\n' );

		int begin = i;
		int end = source.find( "\\endcode", i );
		if ( end != -1 ) {
		    result += quickifiedCode( source.mid(begin, end - begin) );
		    i = end;
		}
	    } else {
		result += source[i++];
	    }
	} else {
	    result += source[i++];
	}
    }

    QValueList<QRegExp>::ConstIterator b = replaceBefores.begin();
    QStringList::ConstIterator a = replaceAfters.begin();
    while ( a != replaceAfters.end() ) {
	result.replace( *b, *a );
	++b;
	++a;
    }
    return result;
}

void QsCodeParser::setQtDoc( Node *quickNode, const Doc& doc )
{
    if ( !doc.isEmpty() ) {
	Doc quickDoc( doc.location(), quickifiedDoc(doc.source()),
		      reunion(CppCodeParser::topicCommands(),
			      CppCodeParser::otherMetaCommands()) );
	quickNode->setDoc( quickDoc, TRUE );
    }
}

void QsCodeParser::setQuickDoc( Node *quickNode, const Doc& doc )
{
    QRegExp quickifyCommand( "\\\\" + COMMAND_QUICKIFY + "([^\n]*)(?:\n|$)" );

    if ( doc.metaCommandsUsed() != 0 &&
	 doc.metaCommandsUsed()->contains(COMMAND_QUICKIFY) ) {
	QString source = doc.source();
	int pos = source.find( quickifyCommand );
	if ( pos != -1 ) {
	    QString quickifiedSource = quickNode->doc().source();
	    applyReplacementList( quickifiedSource, doc );

	    do {
		QString extract = quickifiedSource;
		QString arg = quickifyCommand.cap( 1 ).simplifyWhiteSpace();
		if ( !arg.isEmpty() ) {
		    if ( arg.startsWith("/") && arg.endsWith("/") &&
			 arg.length() > 2 ) {
			QString pattern = arg.mid( 1, arg.length() - 2 );
			extractRegExp( QRegExp(pattern), extract, doc );
		    } else {
			extractTarget( arg, extract, doc );
		    }
		}
		source.replace( pos, quickifyCommand.matchedLength(), extract );
		pos += quickifyCommand.matchedLength();
	    } while ( (pos = source.find(quickifyCommand)) != -1 );
	}

	Doc quickDoc( doc.location(), source,
		      (CppCodeParser::topicCommands() + topicCommands() +
		       CppCodeParser::otherMetaCommands()) << COMMAND_REPLACE );
	quickNode->setDoc( quickDoc, TRUE );
    } else if ( !doc.isEmpty() ) {
	quickNode->setDoc( doc, TRUE );
    }
}

bool QsCodeParser::makeFunctionNode( const QString& synopsis,
				     QStringList *pathPtr,
				     FunctionNode **funcPtr )
{
    QRegExp funcRegExp( "\\s*([A-Za-z0-9_]+)\\.([A-Za-z0-9_]+)\\s*\\((" +
			balancedParens + ")\\)\\s*" );
    QRegExp paramRegExp( "\\s*var\\s+([A-Za-z0-9_]+)?"
			 "\\s*(?::\\s*([A-Za-z0-9_]+)\\s*)?" );

    if ( !funcRegExp.exactMatch(synopsis) )
	return FALSE;

    ClassNode *classe = (ClassNode *) qsTre->findNode( funcRegExp.cap(1),
						       Node::Class );
    if ( classe == 0 )
	return FALSE;

    FunctionNode *clone = new FunctionNode( 0, funcRegExp.cap(2) );

    QString paramStr = funcRegExp.cap( 3 );
    QStringList params = QStringList::split( ",", paramStr );
    QStringList::ConstIterator p = params.begin();
    while ( p != params.end() ) {
	if ( paramRegExp.exactMatch(*p) ) {
	    clone->addParameter( Parameter(paramRegExp.cap(2), "",
					   paramRegExp.cap(1)) );
	} else {
	    delete clone;
	    return FALSE;
	}
	++p;
    }
    if ( pathPtr != 0 )
	*pathPtr = QStringList() << classe->name();
    if ( funcPtr != 0 )
	*funcPtr = clone;
    return TRUE;
}

bool QsCodeParser::isWord( QChar ch )
{
    return ch.isLetterOrNumber() || ch == QChar( '_' );
}

bool QsCodeParser::leftWordBoundary( const QString& str, int pos )
{
    return !isWord( str[pos - 1] ) && isWord( str[pos] );
}

bool QsCodeParser::rightWordBoundary( const QString& str, int pos )
{
    return isWord( str[pos - 1] ) && !isWord( str[pos] );
}

int QsCodeParser::columnForIndex( const QString& str, int index )
{
    int endOfPrevLine = str.findRev( "\n", index - 1 );
    int column = 0;

    for ( int i = endOfPrevLine + 1; i < index; i++ ) {
	if ( str[i] == '\t' ) {
	    column = ( (column / tabSize) + 1 ) * tabSize;
	} else {
	    column++;
	}
    }
    return column;
}
