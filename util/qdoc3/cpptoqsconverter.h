/*
  cpptoqsconverter.h
*/

#ifndef CPPTOQSCONVERTER_H
#define CPPTOQSCONVERTER_H

#include <qregexp.h>

#include "tree.h"

class CppToQsConverter
{
public:
    CppToQsConverter() { }

    ClassNode *findClassNode( Tree *qsTree, const QString& qtName );
    QString convertedDataType( Tree *qsTree, const QString& leftType,
			       const QString& rightType = "" );
    QString convertedCode( Tree *qsTree, const QString& code,
			   const QSet<QString>& classesWithNoQ );

    static void initialize( const Config& config );
    static void terminate();

private:
    void clearState();
    QString convertCodeLine( Tree *qsTree, const QStringList& program,
			     const QString& code,
			     const QSet<QString>& classesWithNoQ );
    QString convertComment( Tree *qsTree, const QString& comment,
			    const QSet<QString>& classesWithNoQ );
    QString convertExpr( Tree *qsTree, const QString& expr,
			 const QSet<QString>& classesWithNoQ );
    void updateDelimDepths( const QString& code );

    static QRegExp qClassRegExp;
    static QRegExp addressOperatorRegExp;
    static QRegExp gulbrandsenRegExp;
    static int tabSize;
};

#endif
