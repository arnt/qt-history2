/*
  qscodeparser.h
*/

#ifndef QSCODEPARSER_H
#define QSCODEPARSER_H

#include "cppcodeparser.h"

class QsCodeParser : public CppCodeParser
{
public:
    QsCodeParser( Tree *cppTree );
    ~QsCodeParser();

    virtual QString language();
    virtual void parseHeaderFile( const Location& location,
				  const QString& filePath, Tree *tree );
    virtual void parseSourceFile( const Location& location,
				  const QString& filePath, Tree *tree );

    FunctionNode *findFunctionNode( const QString& synopsis, Tree *tree );

protected:
    virtual Set<QString> topicCommands();
    virtual Node *processTopicCommand( const Doc& doc, const QString& command,
				       const QString& arg );
    virtual Set<QString> otherMetaCommands();

private:
    ClassNode *tryClass( const QString& className );
    void applyReplacementList( QString *source, const Doc& doc );
    void quickifyClass( ClassNode *quickClass, ClassNode *qtClass,
			ClassNode *wrapperClass );
    void quickifyFunction( ClassNode *quickClass, ClassNode *qtClass,
			   FunctionNode *func, QMap<QString, int> *blackList );
    void quickifyProperty( ClassNode *quickClass, ClassNode *qtClass,
			   PropertyNode *property,
			   QMap<QString, int> *blackList );
    QString quickifiedDataType( const QString& leftType,
				const QString& rightType = "" );
    QString quickifiedCode( const QString& code );
    QString quickifiedDoc( const QString& source );
    void setQtDoc( Node *quickNode, const Doc& doc );
    void setQuickDoc( Node *quickNode, const Doc& doc );

    Tree *cppTre;
    Tree *qsTre;
};

#endif
