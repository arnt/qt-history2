/*
  qsakernelparser.h
*/

#ifndef QSAKERNELPARSER_H
#define QSAKERNELPARSER_H

#include "codeparser.h"

class Tokenizer;

class QsaKernelParser : public CodeParser
{
public:
    QsaKernelParser( Tree *cppTree );
    ~QsaKernelParser();

    virtual QString language();
    virtual QString sourceFileNameFilter();
    virtual void parseSourceFile( const Location& location,
				  const QString& filePath, Tree *tree );
    virtual void doneParsingSourceFiles( Tree *tree );

private:
    void readToken();

    Tree *cppTre;
    Tokenizer *tokenizer;
    int tok;
};

#endif
