/*
  qscodeparser.h
*/

#ifndef QSCODEPARSER_H
#define QSCODEPARSER_H

#include "codeparser.h"

class QsCodeParser : public CodeParser
{
public:
    QsCodeParser();
    ~QsCodeParser();

    virtual QString language();
    virtual void parseHeaderFile( const Location& location,
                                  const QString& filePath, Tree *tree );
    virtual void parseSourceFile( const Location& location,
                                  const QString& filePath, Tree *tree );

    const FunctionNode *findFunctionNode( const QString& synopsis, Tree *tree );
};

#endif
