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
    void readToken();
    const Location& location();
    QString previousLexeme();
    QString lexeme();

    bool match( int target );
    bool matchTemplateAngles( CodeChunk *type = 0 );
    bool matchTemplateHeader();
    bool matchDataType( CodeChunk *type, QString *var = 0 );
    bool matchParameter( FunctionNode *func );
    bool matchFunctionDecl( InnerNode *parent, QStringList *pathPtr = 0,
			    FunctionNode **funcPtr = 0 );

    bool matchBaseSpecifier( ClassNode *classe );
    bool matchBaseList( ClassNode *classe );
    bool matchClassDecl( InnerNode *parent );
    bool matchEnumItem( EnumNode *enume );
    bool matchEnumDecl( InnerNode *parent );
    bool matchTypedefDecl( InnerNode *parent );
    bool matchProperty( InnerNode *parent );
    bool matchDeclList( InnerNode *parent );

    bool matchDocsAndStuff();

    void makeFunctionNode( const QString& synopsis, QStringList *pathPtr,
			   FunctionNode **funcPtr );

    QMap<QString, Node::Type> nodeTypeMap;
    Tree *tre;
    Tokenizer *tokenizer;
    int tok;
    Node::Access access;
    FunctionNode::Metaness metaness;
    QStringList lastPath;
};

#endif
