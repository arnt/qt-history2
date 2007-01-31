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


#include "qscriptgrammar_p.h"
#include "qscriptastfwd_p.h"

class QString;
class QScriptEnginePrivate;
class QScriptNameIdImpl;

class QScriptParser: protected QScriptGrammar
{
public:
    union Value {
      int ival;
      double dval;
      QScriptNameIdImpl *sval;
      QScript::AST::ArgumentList *ArgumentList;
      QScript::AST::CaseBlock *CaseBlock;
      QScript::AST::CaseClause *CaseClause;
      QScript::AST::CaseClauses *CaseClauses;
      QScript::AST::Catch *Catch;
      QScript::AST::DefaultClause *DefaultClause;
      QScript::AST::ElementList *ElementList;
      QScript::AST::Elision *Elision;
      QScript::AST::ExpressionNode *Expression;
      QScript::AST::Finally *Finally;
      QScript::AST::FormalParameterList *FormalParameterList;
      QScript::AST::FunctionBody *FunctionBody;
      QScript::AST::FunctionDeclaration *FunctionDeclaration;
      QScript::AST::Node *Node;
      QScript::AST::PropertyName *PropertyName;
      QScript::AST::PropertyNameAndValueList *PropertyNameAndValueList;
      QScript::AST::SourceElement *SourceElement;
      QScript::AST::SourceElements *SourceElements;
      QScript::AST::Statement *Statement;
      QScript::AST::StatementList *StatementList;
      QScript::AST::VariableDeclaration *VariableDeclaration;
      QScript::AST::VariableDeclarationList *VariableDeclarationList;
    };

public:
    QScriptParser();
    ~QScriptParser();

    bool parse(QScriptEnginePrivate *driver);

    inline QString errorMessage() const
    { return error_message; }

protected:
    inline void reallocateStack();

    inline Value &sym(int index)
    { return sym_stack [tos + index - 1]; }

protected:
    int tos;
    int stack_size;
    Value *sym_stack;
    int *state_stack;
    int *location_stack;
    QString error_message;
};

inline void QScriptParser::reallocateStack()
{
    if (! stack_size)
        stack_size = 128;
    else
        stack_size <<= 1;

    sym_stack = reinterpret_cast<Value*> (qRealloc(sym_stack, stack_size * sizeof(Value)));
    state_stack = reinterpret_cast<int*> (qRealloc(state_stack, stack_size * sizeof(int)));
    location_stack = reinterpret_cast<int*> (qRealloc(location_stack, stack_size * sizeof(int)));
}

