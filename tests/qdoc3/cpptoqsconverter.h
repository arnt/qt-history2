/*
  cpptoqsconverter.h
*/

#ifndef CPPTOQSCONVERTER_H
#define CPPTOQSCONVERTER_H

#include "tree.h"

class CppToQsConverter
{
public:
    CppToQsConverter() { }

    QString convertedDataType( Tree *qsTree, const QString& leftType,
			       const QString& rightType = "" );
    QString convertedCode( Tree *qsTree, const QString& code );

    static void initialize( const Config& config );
    static void terminate();

private:
    int convertCodeLine( Tree *qsTree, QString& code );
    void updateDelimDepths( const QString& code );

    int indent;
    int braceDepth;
    int parenDepth;

    static int columnForIndex( const QString& str, int index );

    static int tabSize;
};

#endif
