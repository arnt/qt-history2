/*
  qscodeparser.cpp
*/

#include <qregexp.h>

#include "qscodeparser.h"
#include "tree.h"

#define COMMAND_JUSTLIKEQT          Doc::alias( "justlikeqt" )
#define COMMAND_QUICKCLASS          Doc::alias( "quickclass" )
#define COMMAND_QUICKFN             Doc::alias( "quickfn" )
#define COMMAND_QUICKIFIED          Doc::alias( "quickified" )
#define COMMAND_QUICKPROPERTY       Doc::alias( "quickproperty" )
#define COMMAND_REPLACE             Doc::alias( "replace" )

QsCodeParser::QsCodeParser( Tree *cppTree )
    : cppTre( cppTree ), qsTre( 0 )
{
}

QsCodeParser::~QsCodeParser()
{
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
    /*
      This is a quick and dirty implementation. It will be rewritten
      when the rest of the class is implemented for real.
    */
    QRegExp funcRegExp( "\\s*([A-Za-z0-9_]+)\\.([A-Za-z0-9_]+)\\s*\\(([^)]*)\\)"
			"\\s*" );
    QRegExp paramRegExp( "\\s*([A-Za-z0-9_]+)(?:\\s+[A-Za-z0-9_]+)?\\s*" );

    if ( funcRegExp.exactMatch(synopsis) ) {
	ClassNode *classe = (ClassNode *) tree->findNode( funcRegExp.cap(1),
							  Node::Class );
	if ( classe == 0 )
	    return 0;

	FunctionNode clone( 0, funcRegExp.cap(2) );

	QString paramStr = funcRegExp.cap( 3 );
	QStringList params = QStringList::split( ",", paramStr );
	QStringList::ConstIterator p = params.begin();
	while ( p != params.end() ) {
	    if ( paramRegExp.exactMatch(*p) ) {
		clone.addParameter( Parameter(paramRegExp.cap(1)) );
	    } else {
		return 0;
	    }
	    ++p;
	}
	return classe->findFunctionNode( &clone );
    } else {
	return 0;
    }
}

Set<QString> QsCodeParser::topicCommands()
{
    return CppCodeParser::topicCommands() << COMMAND_QUICKCLASS
					  << COMMAND_QUICKFN
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
	quickifyDoc( quickClass, doc );
	return 0;
    } else if ( command == COMMAND_QUICKFN ) {
	FunctionNode *quickFunc = findFunctionNode( arg, qsTre );
	quickifyDoc( quickFunc, doc );
	return 0;
    } else if ( command == COMMAND_QUICKPROPERTY ) {
	QStringList path = QStringList::split( ".", arg );
	PropertyNode *quickProperty =
		(PropertyNode *) qsTre->findNode( path, Node::Property );
	if ( quickProperty == 0 )
	    doc.location().warning( tr("Cannot resolve '%1' specified with"
				       " '\\%2'")
				    .arg(arg).arg(command) );
	return 0;
    } else {
	return 0;
    }
}

ClassNode *QsCodeParser::tryClass( const QString& className )
{
    return (ClassNode *) cppTre->findNode( className, Node::Class );
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
		    quickifyDoc( quickFunc, qtFunc->doc() );
	    }
	} else {
	    quickifyDoc( quickFunc, func->doc() );
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
	quickifyDoc( quickProperty, property->doc() );

    blackList->insert( quickProperty->getter(), 0 );
    blackList->insert( quickProperty->setter(), 0 );
    blackList->insert( quickProperty->resetter(), 0 );
}

void QsCodeParser::quickifyDoc( Node *quickNode, const Doc& qtDoc )
{
    quickNode->setDoc( qtDoc, TRUE );
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
