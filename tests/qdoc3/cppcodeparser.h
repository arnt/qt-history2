/*
  cppcodeparser.h
*/

#ifndef CPPCODEPARSER_H
#define CPPCODEPARSER_H

#include "codeparser.h"
#include "node.h"

class ClassNode;
class CodeChunk;
class CppCodeParserPrivate;
class FunctionNode;
class InnerNode;
class Tokenizer;

class CppCodeParser : public CodeParser
{
public:
    CppCodeParser();
    ~CppCodeParser();

    virtual void initializeParser( const Config& config );
    virtual void terminateParser();
    virtual QString language();
    virtual void parseHeaderFile( const Location& location,
				  const QString& filePath, Tree *tree );
    virtual void parseSourceFile( const Location& location,
				  const QString& filePath, Tree *tree );

    const FunctionNode *findFunctionNode( const QString& synopsis, Tree *tree );

protected:
    virtual Set<QString> topicCommands();
    virtual Node *processTopicCommand( Doc *doc, const QString& command,
				       const QString& arg );
    virtual Set<QString> otherMetaCommands();
    virtual void processOtherMetaCommand( Doc *doc, const QString& command,
					  const QString& arg,
					  Node *node );

private:
    void reset( Tree *tree );

    CppCodeParserPrivate *priv;
    QMap<QString, Node::Type> nodeTypeMap;

    friend class CppCodeParserPrivate;
};

#endif
