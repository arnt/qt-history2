/*
  qscodeparser.cpp
*/

#include <qregexp.h>

#include "messages.h"
#include "qscodeparser.h"
#include "tree.h"

#define COMMAND_QUICKCLASS          Doc::alias( "quickclass" )
#define COMMAND_QUICKFN             Doc::alias( "quickfn" )

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
    CppCodeParser::parseHeaderFile( location, filePath, tree );
}

void QsCodeParser::parseSourceFile( const Location& location,
				    const QString& filePath, Tree *tree )
{
    qsTre = tree;
    CppCodeParser::parseSourceFile( location, filePath, tree );
}

Set<QString> QsCodeParser::topicCommands()
{
    return CppCodeParser::topicCommands() << COMMAND_QUICKCLASS
					  << COMMAND_QUICKFN;
}

Node *QsCodeParser::processTopicCommand( Doc *doc, const QString& command,
					 const QString& arg )
{
    ClassNode *wrapperNode = 0;
    ClassNode *qtNode = 0;

    if ( command == COMMAND_QUICKCLASS ) {
	QString qtClass = "Q" + arg;

	if ( (wrapperNode = tryClass("Quick" + arg + "Interface")) != 0 ) {
	    if ( (qtNode = tryClass(qtClass)) == 0 ) {
		Messages::warning( doc->location(),
				   Qdoc::tr("Cannot find Qt class '%1'"
					    " corresponding to '%2'")
				   .arg(qtClass).arg(wrapperNode->name()) );
		return 0;
	    }
	} else if ( (wrapperNode = tryClass("Quick" + arg)) != 0 ) {
	    qtNode = tryClass( qtClass );
	    if ( qtNode == 0 ) {
		qtNode = wrapperNode;
		wrapperNode = 0;
	    }
	} else if ( (wrapperNode = tryClass("Quick" + arg + "Ptr")) != 0 ) {
	    QRegExp ptrToQtType( "(Q[A-Za-z0-9_]+)\\s*\\*" );
	    FunctionNode *ctor =
		    wrapperNode->findFunctionNode( wrapperNode->name() );
	    if ( ctor != 0 && !ctor->parameters().isEmpty() &&
		 ptrToQtType.exactMatch(ctor->parameters().first().leftType()) )
		qtClass = ptrToQtType.cap( 1 );

	    if ( (qtNode = tryClass(qtClass)) == 0 ) {
		Messages::warning( doc->location(),
				   Qdoc::tr("Cannot find Qt class '%1'"
					    " corresponding to '%2'")
				   .arg(qtClass).arg(wrapperNode->name()) );
		return 0;
	    }
	} else if ( (wrapperNode = tryClass("Q" + arg + "Ptr")) != 0 ) {
	    if ( (qtNode = tryClass(qtClass)) == 0 ) {
		Messages::warning( doc->location(),
				   Qdoc::tr("Cannot find Qt class '%1'"
					    " corresponding to '%2'")
				   .arg(qtClass).arg(wrapperNode->name()) );
		return 0;
	    }
	} else {
	    qtNode = tryClass( qtClass );
	    if ( qtNode == 0 ) {
		Messages::warning( doc->location(),
				   Qdoc::tr("Cannot find C++ class"
					    " corresponding to Qt Script class"
					    " '%1'")
				   .arg(arg) );
		return 0;
	    }
	}

	ClassNode *quickNode = new ClassNode( qsTre->root(), arg );
	quickifyClass( quickNode, qtNode, wrapperNode );
	return quickNode;
    } else if ( command == COMMAND_QUICKFN ) {
	return 0;
    } else {
	return CppCodeParser::processTopicCommand( doc, command, arg );
    }
}

ClassNode *QsCodeParser::tryClass( const QString& className )
{
    return (ClassNode *) cppTre->findNode( className, Node::Class );
}

void QsCodeParser::quickifyClass( ClassNode *quickClass,
				  const ClassNode *qtClass,
				  const ClassNode *wrapperClass )
{
    qDebug( "%s + %s -> %s", qtClass ? qtClass->name().latin1() : "(nil)",
	    wrapperClass ? wrapperClass->name().latin1() : "(nil)",
	    quickClass->name().latin1() );

    QMap<QString, int> blackList;

    NodeList children = qtClass->childNodes();
    if ( wrapperClass != 0 )
	children += wrapperClass->childNodes();

    NodeList::ConstIterator c = children.begin();
    while ( c != children.end() ) {
	if ( (*c)->access() == Node::Public &&
	     (*c)->status() == Node::Commendable ) {
	    if ( (*c)->type() == Node::Function )  {
		const FunctionNode *func = (const FunctionNode *) *c;
		quickifyFunction( quickClass, qtClass, func, &blackList );
	    } else if ( (*c)->type() == Node::Property ) {
		const PropertyNode *property = (const PropertyNode *) *c;
		quickifyProperty( quickClass, qtClass, property, &blackList );
	    }
	}
	++c;
    }
}

void QsCodeParser::quickifyFunction( ClassNode *quickClass,
				     const ClassNode *qtClass,
				     const FunctionNode *func,
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
	    if ( func->parent() != (const InnerNode *) qtClass ) {
		const FunctionNode *qtFunc = qtClass->findFunctionNode( func );
		if ( qtFunc != 0 && !qtFunc->doc().isEmpty() )
		    quickifyDoc( quickFunc, qtFunc->doc() );
	    }
	} else {
	    quickifyDoc( quickFunc, func->doc() );
	}
    }
}

void QsCodeParser::quickifyProperty( ClassNode *quickClass,
				     const ClassNode * /* qtClass */,
				     const PropertyNode *property,
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
    quickNode->setDoc( qtDoc );
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
