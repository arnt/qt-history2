/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

/*
  qsakernelparser.h
*/

#ifndef QSAKERNELPARSER_H
#define QSAKERNELPARSER_H

#include "codeparser.h"

QT_BEGIN_NAMESPACE

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

QT_END_NAMESPACE

#endif
