/*
  qscodeparser.cpp
*/

#include <qregexp.h>

#include "config.h"
#include "qscodeparser.h"
#include "tree.h"

#define COMMAND_QUICKCLASS          Doc::alias( "quickclass" )
#define COMMAND_QUICKFN             Doc::alias( "quickfn" )
#define COMMAND_QUICKIFIED          Doc::alias( "quickified" )
#define COMMAND_QUICKPROPERTY       Doc::alias( "quickproperty" )
#define COMMAND_REPLACE             Doc::alias( "replace" )

static QString balancedParentheses = "(?:[^()]+|\\([^()]*\\))*";

int QsCodeParser::tabSize;

QsCodeParser::QsCodeParser( Tree *cppTree )
    : cppTre( cppTree ), qsTre( 0 )
{
}

QsCodeParser::~QsCodeParser()
{
}

void QsCodeParser::initializeParser( const Config& config )
{
    tabSize = config.getInt( CONFIG_TABSIZE );
    CppCodeParser::initializeParser( config );
}

void QsCodeParser::terminateParser()
{
    CppCodeParser::terminateParser();
}

QString QsCodeParser::language()
{
    return "Qt Script";
}

void QsCodeParser::parseHeaderFile( const Location& location,
				    const QString& filePath, Tree *tree )
{
    qsTre = tree;
    CppCodeParser::parseSourceFile( location, filePath, tree );
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
    return Set<QString>() << COMMAND_QUICKCLASS << COMMAND_QUICKFN
			  << COMMAND_QUICKPROPERTY;
}

Node *QsCodeParser::processTopicCommand( const Doc& doc, const QString& command,
					 const QString& arg )
{
    ClassNode *wrapperClass = 0;
    ClassNode *qtClass = 0;

    if ( command == COMMAND_QUICKCLASS ) {
	QString qtClassName = "Q" + arg;

	if ( (wrapperClass = tryClass("Quick" + arg + "Interface")) != 0 ) {
	    if ( (qtClass = tryClass(qtClassName)) == 0 ) {
		doc.location().warning( tr("Cannot find Qt class '%1'"
					   " corresponding to '%2'")
					.arg(qtClassName)
					.arg(wrapperClass->name()) );
		return 0;
	    }
	} else if ( (wrapperClass = tryClass("Quick" + arg)) != 0 ) {
	    qtClass = tryClass( qtClassName );
	    if ( qtClass == 0 ) {
		qtClass = wrapperClass;
		wrapperClass = 0;
	    }
	} else if ( (wrapperClass = tryClass("Quick" + arg + "Ptr")) != 0 ) {
	    QRegExp ptrToQtType( "(Q[A-Za-z0-9_]+)\\s*\\*" );
	    FunctionNode *ctor =
		    wrapperClass->findFunctionNode( wrapperClass->name() );
	    if ( ctor != 0 && !ctor->parameters().isEmpty() &&
		 ptrToQtType.exactMatch(ctor->parameters().first().leftType()) )
		qtClassName = ptrToQtType.cap( 1 );

	    if ( (qtClass = tryClass(qtClassName)) == 0 ) {
		doc.location().warning( tr("Cannot find Qt class '%1'"
					   " corresponding to '%2'")
					.arg(qtClassName)
					.arg(wrapperClass->name()) );
		return 0;
	    }
	} else if ( (wrapperClass = tryClass("Q" + arg + "Ptr")) != 0 ) {
	    if ( (qtClass = tryClass(qtClassName)) == 0 ) {
		doc.location().warning( tr("Cannot find Qt class '%1'"
					   " corresponding to '%2'")
					.arg(qtClassName)
					.arg(wrapperClass->name()) );
		return 0;
	    }
	} else {
	    qtClass = tryClass( qtClassName );
	    if ( qtClass == 0 ) {
		doc.location().warning( tr("Cannot find C++ class"
					   " corresponding to Qt Script class"
					   " '%1'")
					.arg(arg) );
		return 0;
	    }
	}

	ClassNode *quickClass = new ClassNode( qsTre->root(), arg );
	quickifyClass( quickClass, qtClass, wrapperClass );
	setQuickDoc( quickClass, doc );
	return 0;
    } else if ( command == COMMAND_QUICKFN ) {
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
    } else {
	return 0;
    }
}

Set<QString> QsCodeParser::otherMetaCommands()
{
    return commonMetaCommands() << COMMAND_QUICKIFIED << COMMAND_REPLACE;
}

ClassNode *QsCodeParser::tryClass( const QString& className )
{
    return (ClassNode *) cppTre->findNode( className, Node::Class );
}

void QsCodeParser::extractTarget( const QString& target, QString *source,
				  const Doc& doc )
{
    QRegExp targetRegExp(
	    "(\\\\target\\s+(\\S+)[^\n]*\n"
	    "(?:(?!\\s*\\\\code)[^\n]+\n|\\s*\\\\code.*\\\\endcode\\s*\n)*)"
	    "(?:\\s*\n|[^\n]*$)" );
    targetRegExp.setMinimal( TRUE );

    int pos = 0;
    while ( (pos = source->find(targetRegExp, pos)) != -1 ) {
	if ( targetRegExp.cap(2) == target ) {
	    *source = targetRegExp.cap( 1 ) + "\n";
	    return;
	}
	pos += targetRegExp.matchedLength();
    }
    doc.location().warning( tr("Cannot find target '%1'").arg(target) );
}

void QsCodeParser::applyReplacementList( QString *source, const Doc& doc )
{
    static QRegExp replaceRegExp( "/(.+)/([^/]*)/" );

    QStringList args = doc.metaCommandArgs( COMMAND_REPLACE );
    QStringList::ConstIterator a = args.begin();
    while ( a != args.end() ) {
	if ( replaceRegExp.exactMatch(*a) ) {
	    QRegExp before( replaceRegExp.cap(1) );
	    QString after = replaceRegExp.cap( 2 );

	    if ( before.isValid() ) {
		uint oldLen = source->length();
		source->replace( before, after );

		// this condition is sufficient but not necessary
		if ( oldLen == source->length() && !source->contains(after) )
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

void QsCodeParser::quickifyClass( ClassNode *quickClass, ClassNode *qtClass,
				  ClassNode *wrapperClass )
{
    QMap<QString, int> blackList;

    NodeList children = qtClass->childNodes();
    if ( wrapperClass != 0 )
	children += wrapperClass->childNodes();

    NodeList::ConstIterator c = children.begin();
    while ( c != children.end() ) {
	if ( (*c)->access() == Node::Public &&
	     (*c)->status() == Node::Commendable ) {
	    if ( (*c)->type() == Node::Function )  {
		FunctionNode *func = (FunctionNode *) *c;
		quickifyFunction( quickClass, qtClass, func, &blackList );
	    } else if ( (*c)->type() == Node::Property ) {
		PropertyNode *property = (PropertyNode *) *c;
		quickifyProperty( quickClass, qtClass, property, &blackList );
	    }
	}
	++c;
    }
}

void QsCodeParser::quickifyFunction( ClassNode *quickClass, ClassNode *qtClass,
				     FunctionNode *func,
				     QMap<QString, int> *blackList )
{
    if ( func->metaness() != FunctionNode::Plain &&
	 !blackList->contains(func->name()) ) {
	FunctionNode *quickFunc = new FunctionNode( quickClass, func->name() );
	quickFunc->setLocation( func->location() );
	quickFunc->setReturnType( quickifiedDataType(func->returnType()) );

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
		FunctionNode *qtFunc = qtClass->findFunctionNode( func );
		if ( qtFunc != 0 && !qtFunc->doc().isEmpty() )
		    setQtDoc( quickFunc, qtFunc->doc() );
	    }
	} else {
	    setQtDoc( quickFunc, func->doc() );
	}
    }
}

void QsCodeParser::quickifyProperty( ClassNode *quickClass,
				     ClassNode * /* qtClass */,
				     PropertyNode *property,
				     QMap<QString, int> *blackList )
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

    if ( !property->doc().isEmpty() )
	setQtDoc( quickProperty, property->doc() );

    blackList->insert( quickProperty->getter(), 0 );
    blackList->insert( quickProperty->setter(), 0 );
    blackList->insert( quickProperty->resetter(), 0 );
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
	    return s.mid( 1 );
	}
	break;
    case 'b':
	if ( s == "bool" )
	    return "Boolean";
	break;
    case 'c':
	if ( s == "char" ) {
	    return "Number";
	} else if ( s == "const char *" ) {
	    return "String";
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
	    balancedParentheses + ")\\)(?:[ \t]*const)?(?=[ \n\t]*\\{)" );
    QRegExp paramRegExp(
	     "\\s*(const\\s+)?([^\\s=]+)\\b[^=]*\\W(\\w+)\\s*(?:=.*)?" );
    QRegExp qtVarRegExp(
	     "^([ \t]*)(const[ \t]+)?Q([A-Z][A-Za-z_0-9]*)\\s+"
	     "([a-z][A-Za-z_0-9]*)\\s*(?:\\((" + balancedParentheses +
	     ")\\))?\\s*;" );
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
		name.replace( "::", "." );
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
	    } else if ( code[i] == 'T' && leftWordBoundary(code, i) &&
			code.mid(i, 4) == "TRUE" &&
			rightWordBoundary(code, i + 4) ) {
		result += "true";
		i += 4;
	    } else if ( code[i] == 'F' && leftWordBoundary(code, i) &&
			code.mid(i, 5) == "FALSE" &&
			rightWordBoundary(code, i + 5) ) {
		result += "false";
		i += 5;
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
	    if ( source[i] == 'Q' ) {
		if ( source[i + 1].lower() == source[i + 1] ) {
		    result += source[i++];
		} else if ( source.mid(i, 8) == "QCString" ) {
		    i += 2;
		} else {
		    i++;
		}
	    } else if ( source[i] == 'T' && source.mid(i, 4) == "TRUE" &&
			rightWordBoundary(source, i + 4) ) {
		result += "true";
		i += 4;
	    } else if ( source[i] == 'F' && source.mid(i, 5) == "FALSE" &&
			rightWordBoundary(source, i + 5) ) {
		result += "false";
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
    return result;
}

void QsCodeParser::setQtDoc( Node *quickNode, const Doc& doc )
{
    Doc quickDoc( doc.location(), quickifiedDoc(doc.source()),
		  reunion(CppCodeParser::topicCommands(),
			  CppCodeParser::otherMetaCommands()) );
    quickNode->setDoc( quickDoc, TRUE );
}

void QsCodeParser::setQuickDoc( Node *quickNode, const Doc& doc )
{
    QRegExp quickifiedCommand(
	    "\\\\" + QRegExp::escape(COMMAND_QUICKIFIED) + "([^\n]*)(?:\n|$)" );

    if ( doc.metaCommandsUsed() != 0 &&
	 doc.metaCommandsUsed()->contains(COMMAND_QUICKIFIED) ) {
	QString source = doc.source();

	int pos = source.find( quickifiedCommand );
	if ( pos != -1 ) {
	    QString quickifiedSource = quickNode->doc().source();
	    applyReplacementList( &quickifiedSource, doc );

	    do {
		QString extract = quickifiedSource;
		QString target =
			quickifiedCommand.cap( 1 ).simplifyWhiteSpace();
		if ( !target.isEmpty() )
		    extractTarget( target, &extract, doc );
		source.replace( pos, quickifiedCommand.matchedLength(),
				extract );
		pos += quickifiedCommand.matchedLength();
	    } while ( (pos = source.find(quickifiedCommand)) != -1 );
	}

	Doc quickDoc( doc.location(), source,
		      (CppCodeParser::topicCommands() + topicCommands() +
		       CppCodeParser::otherMetaCommands()) <<
		      COMMAND_REPLACE );
	quickNode->setDoc( quickDoc, TRUE );
    } else {
	quickNode->setDoc( doc, TRUE );
    }
}

bool QsCodeParser::makeFunctionNode( const QString& synopsis,
				     QStringList *pathPtr,
				     FunctionNode **funcPtr )
{
    /*
      This is a quick and dirty implementation. It will be rewritten
      when the rest of the class is implemented for real.
    */
    QRegExp funcRegExp( "\\s*([A-Za-z0-9_]+)\\.([A-Za-z0-9_]+)\\s*\\((" +
			balancedParentheses + ")\\)\\s*" );
    QRegExp paramRegExp( "\\s*([A-Za-z0-9_]+)(\\s+[A-Za-z0-9_]+)?\\s*" );

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
	    clone->addParameter( Parameter(paramRegExp.cap(1), "",
					   paramRegExp.cap(2)) );
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
