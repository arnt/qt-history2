/*
  qscodeparser.cpp
*/

#include <qfile.h>
#include <qregexp.h>

#include "config.h"
#include "qscodeparser.h"
#include "text.h"
#include "tokenizer.h"
#include "tree.h"

#define CONFIG_QUICK                "quick"
#define CONFIG_REPLACES             "replaces"

#define COMMAND_BRIEF               Doc::alias( "brief" )
#define COMMAND_FILE                Doc::alias( "file" )
#define COMMAND_GROUP               Doc::alias( "group" )
#define COMMAND_MODULE              Doc::alias( "module" )
#define COMMAND_PAGE                Doc::alias( "page" )
#define COMMAND_QUICKCLASS          Doc::alias( "quickclass" )
#define COMMAND_QUICKENUM           Doc::alias( "quickenum" )
#define COMMAND_QUICKFN             Doc::alias( "quickfn" )
#define COMMAND_QUICKIFY            Doc::alias( "quickify" )
#define COMMAND_QUICKPROPERTY       Doc::alias( "quickproperty" )
#define COMMAND_PROTECTED           Doc::alias( "protected" )
#define COMMAND_REPLACE             Doc::alias( "replace" )

static QString balancedParens = "(?:[^()]+|\\([^()]*\\))*";

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

    nodeTypeMap.insert( COMMAND_QUICKCLASS, Node::Class );
    nodeTypeMap.insert( COMMAND_QUICKENUM, Node::Enum );
    nodeTypeMap.insert( COMMAND_QUICKPROPERTY, Node::Property );
    nodeTypeMap.insert( COMMAND_QUICKFN, Node::Function );

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
    nodeTypeMap.clear();
    classesWithNoQuickDoc.clear();
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

    Location fileLocation( filePath );
    FileTokenizer fileTokenizer( fileLocation, in );
    int tok = fileTokenizer.getToken();
    while ( tok != Tok_Eoi ) {
	if ( tok == Tok_Ident ) {
	    ClassNode *quickClass = new ClassNode( qsTre->root(),
						   fileTokenizer.lexeme() );
	    quickClass->setLocation( fileTokenizer.location() );
	} else {
	    fileTokenizer.location().error( tr("Unexpected token '%1' in Qt"
					       " Script class list")
					    .arg(fileTokenizer.lexeme()) );
	    break;
	}
	tok = fileTokenizer.getToken();
    }
    fclose( in );
}

void QsCodeParser::parseSourceFile( const Location& location,
				    const QString& filePath, Tree *tree )
{
    qsTre = tree;
    CppCodeParser::parseSourceFile( location, filePath, tree );
}

void QsCodeParser::doneParsingHeaderFiles( Tree *tree )
{
    NodeList::ConstIterator c = tree->root()->childNodes().begin();
    while ( c != tree->root()->childNodes().end() ) {
	if ( (*c)->type() == Node::Class )
	    quickifyClass( (ClassNode *) *c );
	++c;
    }
    tree->resolveInheritance();
}

void QsCodeParser::doneParsingSourceFiles( Tree *tree )
{
    tree->root()->normalizeOverloads();

    NodeList::ConstIterator c = tree->root()->childNodes().begin();
    while ( c != tree->root()->childNodes().end() ) {
	if ( (*c)->type() == Node::Class ) {
	    QMap<QString, Node *>::ConstIterator cwnqd =
		    classesWithNoQuickDoc.find( (*c)->name() );
	    if ( cwnqd != classesWithNoQuickDoc.end() ) {
		(*cwnqd)->location().warning( tr("No '\\%1' documentation for"
						 " class '%2'")
					      .arg(COMMAND_QUICKCLASS)
					      .arg(cwnqd.key()) );
		(*cwnqd)->setDoc( Doc(), TRUE );
	    }
	}
	++c;
    }

    // ### check which enum types are used
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
			  << COMMAND_QUICKENUM << COMMAND_QUICKFN
			  << COMMAND_QUICKPROPERTY;
}

Node *QsCodeParser::processTopicCommand( const Doc& doc, const QString& command,
					 const QString& arg )
{
    if ( command == COMMAND_QUICKFN ) {
	QStringList path;
	FunctionNode *quickFunc = 0;
	FunctionNode *clone;

	if ( makeFunctionNode(arg, &path, &clone) ) {
	    quickFunc = qsTre->findFunctionNode( path, clone );
	    if ( quickFunc == 0 ) {
		doc.location().warning( tr("Cannot resolve '%1' specified with"
					   " '\\%2'")
					.arg(arg).arg(command) );
	    } else {
		QStringList qtParams = quickFunc->parameterNames();
		quickFunc->borrowParameterNames( clone );
		QStringList quickParams = quickFunc->parameterNames();
		setQuickDoc( quickFunc, doc, qtParams, quickParams );
	    }
	    delete clone;
	} else {
	    doc.location().warning( tr("Cannot resolve '%1' specified with"
				       " '\\%2'")
				    .arg(arg).arg(command) );
	}
	return 0;
    } else if ( nodeTypeMap.contains(command) ) {
	QStringList path = QStringList::split( ".", arg );
	Node *quickNode = qsTre->findNode( path, nodeTypeMap[command] );
	if ( quickNode == 0 ) {
	    doc.location().warning( tr("Cannot resolve '%1' specified with"
				       " '\\%2'")
				    .arg(arg).arg(command) );
	} else {
	    setQuickDoc( quickNode, doc );
	    if ( quickNode->type() == Node::Class ) {
		classesWithNoQuickDoc.remove( quickNode->name() );
		if ( doc.briefText().isEmpty() )
		    doc.location().warning( tr("Missing '\\%1' for class '%3'")
					    .arg(COMMAND_BRIEF)
					    .arg(quickNode->name()) );
	    }
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

void QsCodeParser::processOtherMetaCommand( const Doc& doc,
			 		    const QString& command,
					    const QString& arg, Node *node )
{
    if ( command == COMMAND_PROTECTED ) {
	doc.location().warning( tr("Cannot use '\\%1' in %2")
				.arg(COMMAND_PROTECTED).arg(language()) );
    } else {
	CppCodeParser::processOtherMetaCommand( doc, command, arg, node );
    }
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

void QsCodeParser::renameParameters( QString& source, const Doc& /* doc */,
				     const QStringList& qtParams,
				     const QStringList& quickParams )
{
    QRegExp paramRegExp( "(\\\\a\\s*\\{?\\s*)([A-Za-z0-9_]+)" );

    int pos = 0;
    while ( (pos = paramRegExp.search(source, pos)) != -1 ) {
	pos += paramRegExp.cap( 1 ).length();
	QString before = paramRegExp.cap( 2 );
	int index = qtParams.findIndex( before );
	if ( index != -1 ) {
	    QString after = quickParams[index];
	    source.replace( pos, before.length(), after );
	}
    }
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
	    replaceBefores << QRegExp( QRegExp::escape(qtClassName) );
	    replaceAfters << bare;
	}
    }

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

    QValueList<RelatedClass>::ConstIterator b = qtClass->baseClasses().begin();
    while ( b != qtClass->baseClasses().end() ) {
	quickClass->addBaseClass( (*b).access, (*b).node, (*b).templateArgs );
	++b;
    }

    Set<QString> blackList;

    NodeList children = qtClass->childNodes();
    if ( wrapperClass != 0 ) {
	children += wrapperClass->childNodes();

	// we don't want the wrapper class constructor and destructor
	blackList.insert( wrapperClass->name() );
	blackList.insert( "~" + wrapperClass->name() );
    }

    for ( int pass = 0; pass < 2; pass++ ) {
	NodeList::ConstIterator c = children.begin();
	while ( c != children.end() ) {
	    if ( (*c)->access() != Node::Private &&
		 (*c)->status() == Node::Commendable ) {
		if ( pass == 0 ) {
		    if ( (*c)->type() == Node::Enum ) {
			EnumNode *enume = (EnumNode *) *c;
			quickifyEnum( quickClass, enume );
		    } else if ( (*c)->type() == Node::Property ) {
			PropertyNode *property = (PropertyNode *) *c;
			quickifyProperty( quickClass, qtClass, property );
			blackList.insert( property->getter() );
			blackList.insert( property->setter() );
			blackList.insert( property->resetter() );
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
    classesWithNoQuickDoc.insert( quickClass->name(), quickClass );
}

void QsCodeParser::quickifyEnum( ClassNode *quickClass, EnumNode *enume )
{
    EnumNode *quickEnum = new EnumNode( quickClass, enume->name() );
    quickEnum->setLocation( enume->location() );
#if 0 // ### not yet
    quickEnum->setAccess( Node::Protected );
#endif

    QValueList<EnumItem>::ConstIterator it = enume->items().begin();
    while ( it != enume->items().end() ) {
	QString name = (*it).name();
	QString value = (*it).value();
	quickEnum->addItem( EnumItem(name, value) );
	++it;
    }
    setQtDoc( quickEnum, enume->doc() );
}

void QsCodeParser::quickifyFunction( ClassNode *quickClass, ClassNode *qtClass,
				     FunctionNode *func )
{
    if ( func->metaness() == FunctionNode::Dtor )
	return;

    QString quickName = func->name();
    if ( func->metaness() == FunctionNode::Ctor )
	quickName = quickClass->name();
    FunctionNode *quickFunc = new FunctionNode( quickClass, quickName );

    quickFunc->setLocation( func->location() );
    if ( func->metaness() == FunctionNode::Plain )
	quickFunc->setAccess( Node::Protected );
    quickFunc->setReturnType( cpp2qs.convertedDataType(qsTre,
						       func->returnType()) );
    if ( func->metaness() != FunctionNode::Slot )
	quickFunc->setMetaness( func->metaness() );
    quickFunc->setOverload( func->isOverload() );

    QValueList<Parameter>::ConstIterator q = func->parameters().begin();
    while ( q != func->parameters().end() ) {
	Parameter param( cpp2qs.convertedDataType(qsTre, (*q).leftType(),
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

void QsCodeParser::quickifyProperty( ClassNode *quickClass,
				     ClassNode * /* qtClass */,
				     PropertyNode *property )
{
    PropertyNode *quickProperty =
	    new PropertyNode( quickClass, property->name() );
    quickProperty->setLocation( property->location() );
    quickProperty->setDataType(
	    cpp2qs.convertedDataType(qsTre, property->dataType()) );
    quickProperty->setGetter( property->getter() );
    quickProperty->setSetter( property->setter() );
    quickProperty->setResetter( property->resetter() );
    quickProperty->setStored( property->isStored() );
    quickProperty->setDesignable( property->isDesignable() );

    setQtDoc( quickProperty, property->doc() );
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
	    // ### make independent of the command name
	    if ( source.mid(i, 5) == "\\code" ) {
		do {
		    result += source[i++];
		} while ( source[i - 1] != '\n' );

		int begin = i;
		int end = source.find( "\\endcode", i );
		if ( end != -1 ) {
		    QString code = source.mid( begin, end - begin );
		    result += cpp2qs.convertedCode( qsTre, code );
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

void QsCodeParser::setQuickDoc( Node *quickNode, const Doc& doc,
				const QStringList& qtParams,
				const QStringList& quickParams )
{
    QRegExp quickifyCommand( "\\\\" + COMMAND_QUICKIFY + "([^\n]*)(?:\n|$)" );

    if ( quickNode->type() == Node::Function ) {
	FunctionNode *quickFunc = (FunctionNode *) quickNode;
	quickFunc->setOverload( FALSE );
    }

    if ( doc.metaCommandsUsed() != 0 &&
	 doc.metaCommandsUsed()->contains(COMMAND_QUICKIFY) ) {
	QString source = doc.source();
	int pos = source.find( quickifyCommand );
	if ( pos != -1 ) {
	    QString quickifiedSource = quickNode->doc().source();
	    if ( !qtParams.isEmpty() && qtParams != quickParams )
		renameParameters( quickifiedSource, doc, qtParams,
				  quickParams );
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
	processOtherMetaCommands( quickDoc, quickNode );
    } else {
	quickNode->setDoc( doc, TRUE );
	processOtherMetaCommands( doc, quickNode );
    }
}

bool QsCodeParser::makeFunctionNode( const QString& synopsis,
				     QStringList *pathPtr,
				     FunctionNode **funcPtr )
{
    QRegExp funcRegExp(
	    "\\s*([A-Za-z0-9_]+)\\.([A-Za-z0-9_]+)\\s*\\((" + balancedParens +
	    ")\\)\\s*" );
    QRegExp paramRegExp(
	    "(?:\\s*([A-Za-z0-9_]+)\\s*:)?\\s*([A-Za-z0-9_]+)\\s*" );

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
