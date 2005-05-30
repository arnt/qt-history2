/*
  codeparser.cpp
*/

#include "codeparser.h"
#include "node.h"
#include "tree.h"

#define COMMAND_COMPAT                  Doc::alias("compat")
#define COMMAND_DEPRECATED              Doc::alias("deprecated") // ### don't document
#define COMMAND_INGROUP                 Doc::alias("ingroup")
#define COMMAND_INMODULE                Doc::alias("inmodule")  // ### don't document
#define COMMAND_INTERNAL                Doc::alias("internal")
#define COMMAND_MAINCLASS		Doc::alias("mainclass")
#define COMMAND_NONREENTRANT            Doc::alias("nonreentrant")
#define COMMAND_OBSOLETE                Doc::alias("obsolete")
#define COMMAND_PRELIMINARY             Doc::alias("preliminary")
#define COMMAND_REENTRANT               Doc::alias("reentrant")
#define COMMAND_THREADSAFE              Doc::alias("threadsafe")
#define COMMAND_TITLE			Doc::alias("title")

QList<CodeParser *> CodeParser::parsers;

CodeParser::CodeParser()
{
    parsers.prepend( this );
}

CodeParser::~CodeParser()
{
    parsers.removeAll( this );
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

QSet<QString> CodeParser::commonMetaCommands()
{
    return QSet<QString>() << COMMAND_COMPAT << COMMAND_DEPRECATED << COMMAND_INGROUP
                           << COMMAND_INMODULE << COMMAND_INTERNAL << COMMAND_MAINCLASS
                           << COMMAND_NONREENTRANT << COMMAND_OBSOLETE << COMMAND_PRELIMINARY
                           << COMMAND_REENTRANT << COMMAND_THREADSAFE << COMMAND_TITLE;
}

void CodeParser::processCommonMetaCommand(const Location &location, const QString &command,
					  const QString &arg, Node *node, Tree *tree)
{
    if (command == COMMAND_COMPAT) {
        node->setStatus(Node::Compat);
    } else if ( command == COMMAND_DEPRECATED ) {
	node->setStatus( Node::Deprecated );
    } else if ( command == COMMAND_INGROUP ) {
	tree->addToGroup(node, arg);
    } else if ( command == COMMAND_INMODULE ) {
	node->setModuleName(arg);
    } else if (command == COMMAND_MAINCLASS) {
	node->setStatus(Node::Main);
    } else if ( command == COMMAND_OBSOLETE ) {
        if (node->status() != Node::Compat)
            node->setStatus( Node::Obsolete );
    } else if ( command == COMMAND_NONREENTRANT ) {
	node->setThreadSafeness(Node::NonReentrant);
    } else if ( command == COMMAND_PRELIMINARY ) {
	node->setStatus( Node::Preliminary );
    } else if (command == COMMAND_INTERNAL) {
	node->setAccess( Node::Private );
    } else if (command == COMMAND_REENTRANT) {
	node->setThreadSafeness(Node::Reentrant);
    } else if (command == COMMAND_THREADSAFE) {
	node->setThreadSafeness(Node::ThreadSafe);
    } else if (command == COMMAND_TITLE) {
	if (node->type() == Node::Fake) {
	    FakeNode *fake = static_cast<FakeNode *>(node);
            fake->setTitle(arg);
        } else {
	    location.warning(tr("Ignored '\\%1'").arg(COMMAND_TITLE));
	}
    }
}
