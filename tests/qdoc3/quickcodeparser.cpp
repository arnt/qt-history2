/*
  quickcodeparser.cpp
*/

#include "quickcodeparser.h"

QuickCodeParser::QuickCodeParser()
{
}

QuickCodeParser::~QuickCodeParser()
{
}

void QuickCodeParser::convertTree( const Tree *tree )
{
    CppCodeParser::convertTree( tree );
    /* create a JavaScript tree */
}

StringSet QuickCodeParser::topicCommands()
{
    return CppCodeParser::topicCommands() << "quickclass" << "quickfn";
}

Node *QuickCodeParser::processTopicCommand( const QString& command,
					    const QString& arg, const Doc& doc )
{
    if ( command == "quickclass" ) {
	return 0;
    } else if ( command == "quickfn" ) {
	return 0;
    } else {
	return CppCodeParser::processTopicCommand( command, arg, doc );
    }
}
