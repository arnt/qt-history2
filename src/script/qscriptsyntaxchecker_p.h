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

#ifndef QSCRIPTSYNTAXCHECKER_H
#define QSCRIPTSYNTAXCHECKER_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/qglobal.h>
#include "qscriptgrammar_p.h"

class QScriptEngine;

namespace QScript {

class Lexer;

class SyntaxChecker: protected QScriptGrammar
{
public:
    SyntaxChecker();
    ~SyntaxChecker();

    bool parse(const QString &code);

protected:
    bool automatic(QScript::Lexer *lexer, int token) const;
    inline void reallocateStack();

protected:
    int tos;
    int stack_size;
    int *state_stack;
};

inline void SyntaxChecker::reallocateStack()
{
    if (! stack_size)
        stack_size = 128;
    else
        stack_size <<= 1;

    state_stack = reinterpret_cast<int*> (qRealloc(state_stack, stack_size * sizeof(int)));
}

} // namespace QScript

#endif

