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

#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H

#include "parser.h"
#include <QList>
#include <QSet>
#include <stdio.h>

struct Macro
{
    Symbols symbols;
};

#ifdef USE_LEXEM_STORE
typedef QByteArray MacroName;
#else
typedef SubArray MacroName;
#endif
typedef QHash<MacroName, Macro> Macros;
typedef QVector<MacroName> MacroSafeSet;


class Preprocessor : public Parser
{
public:
    Preprocessor(){}
    static bool preprocessOnly;
    struct IncludePath
    {
        inline explicit IncludePath(const QByteArray &_path)
            : path(_path), isFrameworkPath(false) {}
        QByteArray path;
        bool isFrameworkPath;
    };
    QList<IncludePath> includes;
    QList<QByteArray> frameworks;
    QSet<QByteArray> preprocessedIncludes;
    Macros macros;
    Symbols preprocessed(const QByteArray &filename, FILE *file);


    void skipUntilEndif();
    bool skipBranch();

    void substituteMacro(const MacroName &macro, Symbols &substituted, MacroSafeSet safeset = MacroSafeSet());
    void substituteUntilNewline(Symbols &substituted, MacroSafeSet safeset = MacroSafeSet());

    int evaluateCondition();


private:
    void until(Token);

    void preprocess(const QByteArray &filename, Symbols &preprocessed);
};


#endif // PREPROCESSOR_H
