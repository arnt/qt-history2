/*
  codeparser.cpp
*/

#include "codeparser.h"
#include "messages.h"
#include "node.h"

CodeParser::~CodeParser()
{
}

StringSet CodeParser::commonMetaCommands()
{
    return StringSet() << "deprecated" << "ingroup" << "inmodule" << "obsolete"
		       << "preliminary" << "private";
}

void CodeParser::processCommonMetaCommand( const Location& location,
					   const QString& command,
					   const QString& /* arg */,
					   Node *node )
{
    bool noNode = FALSE;

    if ( command == "deprecated" ) {
	if ( node == 0 ) {
	    noNode = TRUE;
	} else {
	    node->setStatus( Node::Deprecated );
	}
    } else if ( command == "ingroup" ) {
	/* ... */
    } else if ( command == "inmodule" ) {
	/* ... */
    } else if ( command == "obsolete" ) {
	if ( node == 0 ) {
	    noNode = TRUE;
	} else {
	    node->setStatus( Node::Obsolete );
	}
    } else if ( command == "preliminary" ) {
	if ( node == 0 ) {
	    noNode = TRUE;
	} else {
	    node->setStatus( Node::Preliminary );
	}
    } else if ( command == "private" ) {
	if ( node == 0 ) {
	    noNode = TRUE;
	} else {
	    node->setAccess( Node::Private );
	}
    }

    if ( noNode )
	warning( 1, location, "Cannot use '\\%s' in stand-alone documentation",
		 command.latin1() );
}
