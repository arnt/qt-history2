/*
  codeparser.h
*/

#ifndef CODEPARSER_H
#define CODEPARSER_H

#include "set.h"

class Config;
class Location;
class Node;
class QString;
class Tree;

class CodeParser
{
public:
    CodeParser();
    virtual ~CodeParser();

    virtual void initializeParser( const Config& config );
    virtual void terminateParser();
    virtual void parseHeaderFile( const Location& location,
				  const QString& filePath,
				  Tree *tree ) = 0;
    virtual void parseSourceFile( const Location& location,
				  const QString& filePath,
				  Tree *tree ) = 0;

    virtual QString language() = 0;

    static void initialize( const Config& config );
    static void terminate();
    static CodeParser *parserForLanguage( const QString& language );

protected:
    Set<QString> commonMetaCommands();
    void processCommonMetaCommand( const Location& location,
				   const QString& command, const QString& arg,
				   Node *node );

private:
    static QValueList<CodeParser *> parsers;
};

#endif
