/*
  cppcodeparser.h
*/

#ifndef CPPCODEPARSER_H
#define CPPCODEPARSER_H

#include "codeparser.h"
#include "node.h"
#include "stringset.h"

class ClassNode;
class CodeChunk;
class FunctionNode;
class InnerNode;
class Tokenizer;

class CppCodeParser : public CodeParser
{
public:
    CppCodeParser();

    virtual void parseHeaderFile( const QString& filePath, Tree *tree );
    virtual void parseSourceFile( const QString& filePath, Tree *tree );
    virtual void convertTree( const Tree *tree );
    const FunctionNode *findFunctionNode( const QString& synopsys, Tree *tree );

protected:
    virtual StringSet topicCommands();
    virtual Node *processTopicCommand( const QString& command,
				       const QString& arg, const Doc& doc );
    virtual StringSet otherMetaCommands();
    virtual void processOtherMetaCommand( const QString& command,
					  const QString& arg, Node *node );
    Tree *tree() const { return tre; }

private:
    void reinit( Tree *tree );
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

    void makeFunctionNode( const QString& synopsys, QStringList *pathPtr,
			   FunctionNode **funcPtr );

    Tree *tre;
    Tokenizer *tokenizer;
    int tok;
    Node::Access access;
    FunctionNode::Metaness metaness;
    QMap<QString, Node::Type> nodeTypeMap;
};

#endif
