/*
  codeparser.cpp
*/

#include "codeparser.h"
#include "messages.h"
#include "node.h"

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
    return Set<QString>() << "deprecated" << "ingroup" << "inmodule"
			  << "obsolete" << "preliminary" << "private";
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
	Messages::warning( location,
			   Qdoc::tr("Cannot use '\\%1' in stand-alone"
				    " documentation")
			   .arg(command) );
}
