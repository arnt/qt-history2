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
    QString quickifiedDataType( const QString& leftType,
				const QString& rightType = "" );
    void merge( ClassNode *quickClass, const ClassNode *qtClass,
		const ClassNode *wrapperClass );

    Tree *cppTre;
    Tree *quickTre;
};

#endif
