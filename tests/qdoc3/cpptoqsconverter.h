/*
  cpptoqsconverter.h
*/

#ifndef CPPTOQSCONVERTER_H
#define CPPTOQSCONVERTER_H

#include "tree.h"

class CppToQsConverter
{
public:
    CppToQsConverter();

    ClassNode *findClassNode( Tree *qsTree, const QString& qtName );
    QString convertedDataType( Tree *qsTree, const QString& leftType,
			       const QString& rightType = "" );
    QString convertedCode( Tree *qsTree, const QString& code );

    static void initialize( const Config& config );
    static void terminate();

private:
    void clearState();
    QString convertCodeLine( Tree *qsTree, const QStringList& program,
			     QString code );
    void updateDelimDepths( const QString& code );

    QString returnType;

    static int tabSize;
};

#endif
