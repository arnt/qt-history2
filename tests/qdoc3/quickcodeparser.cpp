/*
  quickcodeparser.cpp
*/

#include <qregexp.h>

#include "messages.h"
#include "quickcodeparser.h"
#include "tree.h"

#define COMMAND_QUICKCLASS          Doc::alias( "quickclass" )
#define COMMAND_QUICKFN             Doc::alias( "quickfn" )

QuickCodeParser::QuickCodeParser( Tree *cppTree )
    : cppTre( cppTree ), quickTre( 0 )
{
}

QuickCodeParser::~QuickCodeParser()
{
}

QString QuickCodeParser::language()
{
    return "Quick";
}

void QuickCodeParser::parseHeaderFile( const Location& location,
				       const QString& filePath, Tree *tree )
{
    quickTre = tree;
    CppCodeParser::parseHeaderFile( location, filePath, cppTre );
}

void QuickCodeParser::parseSourceFile( const Location& location,
				       const QString& filePath, Tree *tree )
{
    quickTre = tree;
    CppCodeParser::parseSourceFile( location, filePath, cppTre );
}

Set<QString> QuickCodeParser::topicCommands()
{
    return CppCodeParser::topicCommands() << COMMAND_QUICKCLASS
					  << COMMAND_QUICKFN;
}

Node *QuickCodeParser::processTopicCommand( Doc *doc, const QString& command,
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

	ClassNode *quickNode = new ClassNode( quickTre->root(), arg );
	merge( quickNode, qtNode, wrapperNode );
	return quickNode;
    } else if ( command == COMMAND_QUICKFN ) {
	return 0;
    } else {
	return CppCodeParser::processTopicCommand( doc, command, arg );
    }
}

ClassNode *QuickCodeParser::tryClass( const QString& className )
{
    return (ClassNode *) cppTre->findNode( className, Node::Class );
}

QString QuickCodeParser::quickifiedDataType( const QString& leftType,
					     const QString& rightType )
{
    QString s = leftType;

    if ( s.startsWith("const ") )
	s = s.mid( 6 );
    if ( s.endsWith("*") || s.endsWith("&") || s.endsWith(" ") )
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
    }
    return s;
}

void QuickCodeParser::merge( ClassNode *quickClass, const ClassNode *qtClass,
			     const ClassNode *wrapperClass )
{
    qDebug( "%s + %s -> %s", qtClass ? qtClass->name().latin1() : "(nil)",
	    wrapperClass ? wrapperClass->name().latin1() : "(nil)",
	    quickClass->name().latin1() );

    QMap<QString, int> blackList;

    NodeList::ConstIterator c = qtClass->childNodes().begin();
    while ( c != qtClass->childNodes().end() ) {
	if ( (*c)->access() == Node::Public &&
	     (*c)->status() == Node::Commendable ) {
	    if ( (*c)->type() == Node::Function )  {
		const FunctionNode *qtFunc = (const FunctionNode *) *c;
		if ( qtFunc->metaness() != FunctionNode::Plain &&
		     !blackList.contains(qtFunc->name()) ) {
		    FunctionNode *quickFunc =
			    new FunctionNode( quickClass, qtFunc->name() );
		    quickFunc->setLocation( qtFunc->location() );
		    quickFunc->setReturnType(
			    quickifiedDataType(qtFunc->returnType()) );

		    QValueList<Parameter>::ConstIterator q =
			    qtFunc->parameters().begin();
		    while ( q != qtFunc->parameters().end() ) {
			Parameter param( quickifiedDataType((*q).leftType(),
							    (*q).rightType()),
					 "", (*q).name() );
			quickFunc->addParameter( param );
			++q;
		    }
		}
	    } else if ( (*c)->type() == Node::Property ) {
		const PropertyNode *qtProp = (const PropertyNode *) *c;
		PropertyNode *quickProp =
			new PropertyNode( quickClass, qtProp->name() );
		quickProp->setLocation( quickProp->location() );
		quickProp->setDataType(
			quickifiedDataType(qtProp->dataType()) );
		quickProp->setGetter( qtProp->getter() );
		quickProp->setSetter( qtProp->setter() );
		quickProp->setResetter( qtProp->resetter() );
		quickProp->setStored( qtProp->isStored() );
		quickProp->setDesignable( qtProp->isDesignable() );

		blackList.insert( quickProp->getter(), 0 );
		blackList.insert( quickProp->setter(), 0 );
#if 0
		blackList.insert( quickProp->resetter(), 0 );
#endif
	    }
	}
	++c;
    }
}
