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

    const FunctionNode *findFunctionNode( const QString& synopsis, Tree *tree );

protected:
    virtual Set<QString> topicCommands();
    virtual Node *processTopicCommand( Doc *doc, const QString& command,
				       const QString& arg );

private:
    ClassNode *tryClass( const QString& className );
    void quickifyClass( ClassNode *quickClass, const ClassNode *qtClass,
			const ClassNode *wrapperClass );

    void quickifyFunction( ClassNode *quickClass, const ClassNode *qtClass,
			   const FunctionNode *func,
			   QMap<QString, int> *blackList );
    void quickifyProperty( ClassNode *quickClass, const ClassNode *qtClass,
			   const PropertyNode *property,
			   QMap<QString, int> *blackList );
    void quickifyDoc( Node *quickNode, const Doc& qtDoc );
    QString quickifiedDataType( const QString& leftType,
				const QString& rightType = "" );

    Tree *cppTre;
    Tree *qsTre;
};

#endif
