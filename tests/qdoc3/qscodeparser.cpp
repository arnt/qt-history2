/*
  qscodeparser.cpp
*/

#include "qscodeparser.h"

QsCodeParser::QsCodeParser()
{
}

QsCodeParser::~QsCodeParser()
{
}

QString QsCodeParser::language()
{
    return "Qt Script";
}

void QsCodeParser::parseHeaderFile( const Location& location,
                                    const QString& filePath, Tree *tree )
{
}

void QsCodeParser::parseSourceFile( const Location& location,
				    const QString& filePath, Tree *tree )
{
}

const FunctionNode *QsCodeParser::findFunctionNode( const QString& synopsis,
						    Tree *tree )
{
}
