/*
  cppcodeparser.h
*/

#ifndef CPPCODEPARSER_H
#define CPPCODEPARSER_H

#include <qregexp.h>

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
    virtual QString headerFileNameFilter();
    virtual QString sourceFileNameFilter();
    virtual void parseHeaderFile( const Location& location,
				  const QString& filePath, Tree *tree );
    virtual void parseSourceFile( const Location& location,
				  const QString& filePath, Tree *tree );
    virtual void doneParsingHeaderFiles( Tree *tree );
    virtual void doneParsingSourceFiles( Tree *tree );

    const FunctionNode *findFunctionNode(const QString& synopsis, Tree *tree, Node *relative = 0);

protected:
    virtual QSet<QString> topicCommands();
    virtual Node *processTopicCommand( const Doc& doc, const QString& command,
				       const QString& arg );
    virtual QSet<QString> otherMetaCommands();
    virtual void processOtherMetaCommand( const Doc& doc,
					  const QString& command,
					  const QString& arg, Node *node );
    void processOtherMetaCommands( const Doc& doc, Node *node );

private:
    void reset( Tree *tree );
    void readToken();
    const Location& location();
    QString previousLexeme();
    QString lexeme();
    bool match( int target );
    bool matchCompat();
    bool matchTemplateAngles( CodeChunk *type = 0 );
    bool matchTemplateHeader();
    bool matchDataType( CodeChunk *type, QString *var = 0 );
    bool matchParameter( FunctionNode *func );
    bool matchFunctionDecl( InnerNode *parent, QStringList *parentPathPtr = 0,
			    FunctionNode **funcPtr = 0 );
    bool matchBaseSpecifier( ClassNode *classe );
    bool matchBaseList( ClassNode *classe );
    bool matchClassDecl( InnerNode *parent );
    bool matchNamespaceDecl(InnerNode *parent);
    bool matchEnumItem( InnerNode *parent, EnumNode *enume );
    bool matchEnumDecl( InnerNode *parent );
    bool matchTypedefDecl( InnerNode *parent );
    bool matchProperty( InnerNode *parent );
    bool matchDeclList( InnerNode *parent );
    bool matchDocsAndStuff();
    bool makeFunctionNode(const QString &synopsis, QStringList *parentPathPtr,
			  FunctionNode **funcPtr);
    void parseQiteratorDotH(const Location &location, const QString &filePath);
    void instantiateIteratorMacro(const QString &container, const QString &includeFile,
				  const QString &macroDef, Tree *tree);
    void createExampleFileNodes(FakeNode *fake);

    QMap<QString, Node::Type> nodeTypeMap;
    Tree *tre;
    Tokenizer *tokenizer;
    int tok;
    Node::Access access;
    FunctionNode::Metaness metaness;
    QStringList lastPath;
    QRegExp varComment;
    QRegExp sep;

    QString sequentialIteratorDefinition;
    QString mutableSequentialIteratorDefinition;
    QString associativeIteratorDefinition;
    QString mutableAssociativeIteratorDefinition;
    QMap<QString, QString> sequentialIteratorClasses;
    QMap<QString, QString> mutableSequentialIteratorClasses;
    QMap<QString, QString> associativeIteratorClasses;
    QMap<QString, QString> mutableAssociativeIteratorClasses;

    static QStringList exampleFiles;
    static QStringList exampleDirs;
};

#endif
