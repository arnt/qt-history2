/*
  quickcodeparser.h
*/

#ifndef QUICKCODEPARSER_H
#define QUICKCODEPARSER_H

#include "cppcodeparser.h"

class QuickCodeParser : public CppCodeParser
{
public:
    QuickCodeParser( Tree *cppTree );
    ~QuickCodeParser();

    virtual QString language();
    virtual void parseHeaderFile( const Location& location,
				  const QString& filePath, Tree *tree );
    virtual void parseSourceFile( const Location& location,
				  const QString& filePath, Tree *tree );

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
    Tree *quickTre;
};

#endif
