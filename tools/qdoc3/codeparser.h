/*
  codeparser.h
*/

#ifndef CODEPARSER_H
#define CODEPARSER_H

#include <QSet>

#include "location.h"

class Config;
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
    virtual QString language() = 0;
    virtual QString headerFileNameFilter();
    virtual QString sourceFileNameFilter() = 0;
    virtual void parseHeaderFile( const Location& location,
				  const QString& filePath, Tree *tree );
    virtual void parseSourceFile( const Location& location,
				  const QString& filePath, Tree *tree ) = 0;
    virtual void doneParsingHeaderFiles( Tree *tree );
    virtual void doneParsingSourceFiles( Tree *tree ) = 0;

    static void initialize( const Config& config );
    static void terminate();
    static CodeParser *parserForLanguage( const QString& language );

protected:
    QSet<QString> commonMetaCommands();
    void processCommonMetaCommand(const Location& location,
				  const QString& command, const QString& arg,
				  Node *node, Tree *tree);

private:
    static QList<CodeParser *> parsers;
};

#endif
