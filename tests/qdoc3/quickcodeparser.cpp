/*
  quickcodeparser.cpp
*/

#include <qregexp.h>

#include "messages.h"
#include "quickcodeparser.h"
#include "tree.h"

QuickCodeParser::QuickCodeParser( Tree *cppTree )
    : cppTre( cppTree ), quickTre( 0 )
{
}

QuickCodeParser::~QuickCodeParser()
{
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

QString QuickCodeParser::language()
{
    return "Quick";
}

Set<QString> QuickCodeParser::topicCommands()
{
    return CppCodeParser::topicCommands() << "quickclass" << "quickfn";
}

Node *QuickCodeParser::processTopicCommand( Doc *doc, const QString& command,
					    const QString& arg )
{
    ClassNode *wrapperNode = 0;
    ClassNode *qtNode = 0;

    if ( command == "quickclass" ) {
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
	// ###
	return quickNode;
    } else if ( command == "quickfn" ) {
	return 0;
    } else {
	return CppCodeParser::processTopicCommand( doc, command, arg );
    }
}

ClassNode *QuickCodeParser::tryClass( const QString& className )
{
    return (ClassNode *) cppTre->findNode( className, Node::Class );
}
