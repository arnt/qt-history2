/*
  codeparser.cpp
*/

#include "codeparser.h"
#include "node.h"

#define COMMAND_DEPRECATED          Doc::alias( "deprecated" )
#define COMMAND_INGROUP             Doc::alias( "ingroup" )
#define COMMAND_INMODULE            Doc::alias( "inmodule" )
#define COMMAND_OBSOLETE            Doc::alias( "obsolete" )
#define COMMAND_PRELIMINARY         Doc::alias( "preliminary" )
#define COMMAND_PRIVATE             Doc::alias( "private" )

QValueList<CodeParser *> CodeParser::parsers;

CodeParser::CodeParser()
{
    parsers.prepend( this );
}

CodeParser::~CodeParser()
{
    parsers.remove( this );
}

void CodeParser::initializeParser( const Config& /* config */ )
{
}

void CodeParser::terminateParser()
{
}

void CodeParser::initialize( const Config& config )
{
    QValueList<CodeParser *>::ConstIterator p = parsers.begin();
    while ( p != parsers.end() ) {
	(*p)->initializeParser( config );
	++p;
    }
}

void CodeParser::terminate()
{
    QValueList<CodeParser *>::ConstIterator p = parsers.begin();
    while ( p != parsers.end() ) {
	(*p)->terminateParser();
	++p;
    }
}

CodeParser *CodeParser::parserForLanguage( const QString& language )
{
    QValueList<CodeParser *>::ConstIterator p = parsers.begin();
    while ( p != parsers.end() ) {
	if ( (*p)->language() == language )
	    return *p;
	++p;
    }
    return 0;
}

Set<QString> CodeParser::commonMetaCommands()
{
    return Set<QString>() << COMMAND_DEPRECATED << COMMAND_INGROUP
			  << COMMAND_INMODULE << COMMAND_OBSOLETE
			  << COMMAND_PRELIMINARY << COMMAND_PRIVATE;
}

void CodeParser::processCommonMetaCommand( const Location& location,
					   const QString& command,
					   const QString& /* arg */,
					   Node *node )
{
    bool noNode = FALSE;

    if ( command == COMMAND_DEPRECATED ) {
	if ( node == 0 ) {
	    noNode = TRUE;
	} else {
	    node->setStatus( Node::Deprecated );
	}
    } else if ( command == COMMAND_INGROUP ) {
	/* ... */
    } else if ( command == COMMAND_INMODULE ) {
	/* ... */
    } else if ( command == COMMAND_OBSOLETE ) {
	if ( node == 0 ) {
	    noNode = TRUE;
	} else {
	    node->setStatus( Node::Obsolete );
	}
    } else if ( command == COMMAND_PRELIMINARY ) {
	if ( node == 0 ) {
	    noNode = TRUE;
	} else {
	    node->setStatus( Node::Preliminary );
	}
    } else if ( command == COMMAND_PRIVATE ) {
	if ( node == 0 ) {
	    noNode = TRUE;
	} else {
	    node->setAccess( Node::Private );
	}
    }

    if ( noNode )
	location.warning( tr("Cannot use '\\%1' in stand-alone documentation")
			  .arg(command) );
}
