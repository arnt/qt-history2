/*
  qscodeparser.h
*/

#ifndef QSCODEPARSER_H
#define QSCODEPARSER_H

#include "cppcodeparser.h"
#include "cpptoqsconverter.h"

class QsCodeParser : public CppCodeParser
{
public:
    QsCodeParser( Tree *cppTree );
    ~QsCodeParser();

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

    FunctionNode *findFunctionNode( const QString& synopsis, Tree *tree );

protected:
    virtual QSet<QString> topicCommands();
    virtual Node *processTopicCommand( const Doc& doc, const QString& command,
				       const QString& arg );
    virtual QSet<QString> otherMetaCommands();
    virtual void processOtherMetaCommand( const Doc& doc,
					  const QString& command,
					  const QString& arg, Node *node );

private:
    ClassNode *tryClass( const QString& className );
    FunctionNode *findKernelFunction( const QStringList& parentPath,
				      const QString& name );
    void extractRegExp( const QRegExp& regExp, QString& source,
			const Doc& doc );
    void extractTarget( const QString& target, QString& source,
			const Doc& doc );
    void renameParameters( QString& source, const Doc& doc,
			   const QStringList& qtNames,
			   const QStringList& quickNames );
    void applyReplacementList( QString& source, const Doc& doc );
    void quickifyClass( ClassNode *quickClass );
    void quickifyEnum( ClassNode *quickClass, EnumNode *enume );
    void quickifyFunction( ClassNode *quickClass, ClassNode *qtClass,
			   FunctionNode *func, bool onBlackList );
    void quickifyProperty( ClassNode *quickClass, ClassNode *qtClass,
			   PropertyNode *property );
    QString quickifiedDoc( const QString& source );
    void setQtDoc( Node *quickNode, const Doc& doc );
    void setQuickDoc( Node *quickNode, const Doc& doc,
		      const QStringList& qtParams = QStringList(),
		      const QStringList& quickParams = QStringList() );
    bool makeFunctionNode( const QString& synopsis, QStringList *parentPathPtr,
			   FunctionNode **funcPtr );

    static bool isWord( QChar ch );
    static bool leftWordBoundary( const QString& str, int pos );
    static bool rightWordBoundary( const QString& str, int pos );

    QMap<QString, Node::Type> nodeTypeMap;
    QMap<QString, Node *> classesWithNoQuickDoc;
    QList<QRegExp> replaceBefores;
    QStringList replaceAfters;
    QSet<QString> classesWithNoQ;
    Tree *cppTre;
    Tree *qsTre;
    QRegExp replaceRegExp;
    CppToQsConverter cpp2qs;

    static int tabSize;
};

#endif
