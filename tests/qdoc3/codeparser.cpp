/*
  codeparser.cpp
*/

#include "codeparser.h"
#include "node.h"

#define COMMAND_DEPRECATED              Doc::alias("deprecated")
#define COMMAND_INGROUP                 Doc::alias("ingroup")
#define COMMAND_INMODULE                Doc::alias("inmodule")
#define COMMAND_NONREENTRANT            Doc::alias("nonreentrant")
#define COMMAND_OBSOLETE                Doc::alias("obsolete")
#define COMMAND_PRELIMINARY             Doc::alias("preliminary")
#define COMMAND_PRIVATE                 Doc::alias("private")
#define COMMAND_PROTECTED               Doc::alias("protected")
#define COMMAND_PUBLIC                  Doc::alias("public")
#define COMMAND_REENTRANT               Doc::alias("reentrant")
#define COMMAND_THREADSAFE              Doc::alias("threadsafe")

QList<CodeParser *> CodeParser::parsers;

CodeParser::CodeParser()
{
    parsers.prepend( this );
}

CodeParser::~CodeParser()
{
    parsers.remove( this );
}

void CodeParser::initializeParser(const Config & /* config */)
{
}

void CodeParser::terminateParser()
{
}

QString CodeParser::headerFileNameFilter()
{
    return sourceFileNameFilter();
}

void CodeParser::parseHeaderFile( const Location& location,
				  const QString& filePath, Tree *tree )
{
    parseSourceFile( location, filePath, tree );
}

void CodeParser::doneParsingHeaderFiles( Tree *tree )
{
    doneParsingSourceFiles( tree );
}

void CodeParser::initialize( const Config& config )
{
    QList<CodeParser *>::ConstIterator p = parsers.begin();
    while ( p != parsers.end() ) {
	(*p)->initializeParser( config );
	++p;
    }
}

void CodeParser::terminate()
{
    QList<CodeParser *>::ConstIterator p = parsers.begin();
    while ( p != parsers.end() ) {
	(*p)->terminateParser();
	++p;
    }
}

CodeParser *CodeParser::parserForLanguage( const QString& language )
{
    QList<CodeParser *>::ConstIterator p = parsers.begin();
    while ( p != parsers.end() ) {
	if ( (*p)->language() == language )
	    return *p;
	++p;
    }
    return 0;
}

Set<QString> CodeParser::commonMetaCommands()
{
    return Set<QString>() << COMMAND_DEPRECATED << COMMAND_INGROUP << COMMAND_INMODULE
			  << COMMAND_NONREENTRANT << COMMAND_OBSOLETE << COMMAND_PRELIMINARY
                          << COMMAND_PRIVATE << COMMAND_PROTECTED << COMMAND_PUBLIC
                          << COMMAND_REENTRANT << COMMAND_THREADSAFE;
}

void CodeParser::processCommonMetaCommand( const Location& /* location */,
					   const QString& command,
					   const QString& /* arg */,
					   Node *node )
{
    if ( command == COMMAND_DEPRECATED ) {
	node->setStatus( Node::Deprecated );
    } else if ( command == COMMAND_INGROUP ) {
	/* ... */
    } else if ( command == COMMAND_INMODULE ) {
	/* ... */
    } else if ( command == COMMAND_OBSOLETE ) {
	node->setStatus( Node::Obsolete );
    } else if ( command == COMMAND_NONREENTRANT ) {
	node->setThreadSafeness(Node::NonReentrant);
    } else if ( command == COMMAND_PRELIMINARY ) {
	node->setStatus( Node::Preliminary );
    } else if ( command == COMMAND_PRIVATE ) {
	node->setAccess( Node::Private );
    } else if ( command == COMMAND_PROTECTED ) {
	node->setAccess( Node::Protected );
    } else if ( command == COMMAND_PUBLIC ) {
	node->setAccess( Node::Public );
    } else if (command == COMMAND_REENTRANT) {
	node->setThreadSafeness(Node::Reentrant);
    } else if (command == COMMAND_THREADSAFE) {
	node->setThreadSafeness(Node::ThreadSafe);
    }
}
