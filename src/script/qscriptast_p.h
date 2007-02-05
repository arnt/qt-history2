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

#ifndef QSCRIPTAST_P_H
#define QSCRIPTAST_P_H

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

#include <QtCore/QString>

#include "qscriptastfwd_p.h"
#include "qscriptastvisitor_p.h"

class QScriptNameIdImpl;

namespace QSOperator // ### rename
{

enum Op {
    Add,
    And,
    InplaceAnd,
    Assign,
    BitAnd,
    BitOr,
    BitXor,
    InplaceSub,
    Div,
    InplaceDiv,
    Equal,
    Ge,
    Gt,
    In,
    InplaceAdd,
    InstanceOf,
    Le,
    LShift,
    InplaceLeftShift,
    Lt,
    Mod,
    InplaceMod,
    Mul,
    InplaceMul,
    NotEqual,
    Or,
    InplaceOr,
    RShift,
    InplaceRightShift,
    StrictEqual,
    StrictNotEqual,
    Sub,
    URShift,
    InplaceURightShift,
    InplaceXor
};

} // namespace QSOperator

namespace QScript { namespace AST {

class Node
{
public:
    inline Node():
        startLine(0), startColumn(0) {}

    virtual ~Node() {}

    virtual ExpressionNode *expressionCast();
    virtual Statement *statementCast();

    inline void accept(Visitor *visitor)
    {
        if (visitor->preVisit(this)) {
            accept0(visitor);
            visitor->postVisit(this);
        }
    }

    static void acceptChild(Node *node, Visitor *visitor)
    {
        if (node)
            node->accept(visitor);
    }

    virtual void accept0(Visitor *visitor) = 0;

    int startLine;
    int startColumn;
};

class ExpressionNode: public Node
{
public:
    virtual ~ExpressionNode() {}

    virtual ExpressionNode *expressionCast();
};

class Statement: public Node
{
public:
    virtual ~Statement() {}

    virtual Statement *statementCast();
};

class ThisExpression: public ExpressionNode
{
public:
    ThisExpression() {}
    virtual ~ThisExpression() {}

    virtual void accept0(Visitor *visitor);
};

class IdentifierExpression: public ExpressionNode
{
public:
    IdentifierExpression(QScriptNameIdImpl *n):
        name (n) {}

    virtual ~IdentifierExpression() {}

    virtual void accept0(Visitor *visitor);

// attributes
    QScriptNameIdImpl *name;
};

class NullExpression: public ExpressionNode
{
public:
    NullExpression() {}
    virtual ~NullExpression() {}

    virtual void accept0(Visitor *visitor);
};

class TrueLiteral: public ExpressionNode
{
public:
    TrueLiteral() {}
    virtual ~TrueLiteral() {}

    virtual void accept0(Visitor *visitor);
};

class FalseLiteral: public ExpressionNode
{
public:
    FalseLiteral() {}
    virtual ~FalseLiteral() {}

    virtual void accept0(Visitor *visitor);
};

class NumericLiteral: public ExpressionNode
{
public:
    NumericLiteral(double v):
        value (v) {}
    virtual ~NumericLiteral() {}

    virtual void accept0(Visitor *visitor);

// attributes:
    double value;
};

class StringLiteral: public ExpressionNode
{
public:
    StringLiteral(QScriptNameIdImpl *v):
        value (v) {}

    virtual ~StringLiteral() {}

    virtual void accept0(Visitor *visitor);

// attributes:
    QScriptNameIdImpl *value;
};

class RegExpLiteral: public ExpressionNode
{
public:
    RegExpLiteral(QScriptNameIdImpl *p, QScriptNameIdImpl *f):
        pattern (p), flags (f) {}

    virtual ~RegExpLiteral() {}

    virtual void accept0(Visitor *visitor);

// attributes:
    QScriptNameIdImpl *pattern;
    QScriptNameIdImpl *flags;
};

class ArrayLiteral: public ExpressionNode
{
public:
    ArrayLiteral(Elision *e):
        elements (0), elision (e) {}

    ArrayLiteral(ElementList *elts):
        elements (elts), elision (0) {}

    ArrayLiteral(ElementList *elts, Elision *e):
        elements (elts), elision (e) {}

    virtual ~ArrayLiteral() {}

    virtual void accept0(Visitor *visitor);

// attributes
    ElementList *elements;
    Elision *elision;
};

class ObjectLiteral: public ExpressionNode
{
public:
    ObjectLiteral():
        properties (0) {}

    ObjectLiteral(PropertyNameAndValueList *plist):
        properties (plist) {}

    virtual ~ObjectLiteral() {}

    virtual void accept0(Visitor *visitor);

// attributes
    PropertyNameAndValueList *properties;
};

class ElementList: public Node
{
public:
    ElementList(Elision *e, ExpressionNode *expr):
        elision (e), expression (expr), next (this) {}

    ElementList(ElementList *previous, Elision *e, ExpressionNode *expr):
        elision (e), expression (expr)
    {
        next = previous->next;
        previous->next = this;
    }

    virtual ~ElementList() {}

    inline ElementList *finish ()
    {
        ElementList *front = next;
        next = 0;
        return front;
    }

    virtual void accept0(Visitor *visitor);

// attributes
    Elision *elision;
    ExpressionNode *expression;
    ElementList *next;
};

class Elision: public Node
{
public:
    Elision():
        next (this) {}

    Elision(Elision *previous)
    {
        next = previous->next;
        previous->next = this;
    }

    virtual ~Elision() {}

    virtual void accept0(Visitor *visitor);

    inline Elision *finish ()
    {
        Elision *front = next;
        next = 0;
        return front;
    }

// attributes
    Elision *next;
};

class PropertyNameAndValueList: public Node
{
public:
    PropertyNameAndValueList(PropertyName *n, ExpressionNode *v):
        name (n), value (v), next (this) {}

    PropertyNameAndValueList(PropertyNameAndValueList *previous, PropertyName *n, ExpressionNode *v):
        name (n), value (v)
    {
        next = previous->next;
        previous->next = this;
    }

    virtual ~PropertyNameAndValueList() {}

    virtual void accept0(Visitor *visitor);

    inline PropertyNameAndValueList *finish ()
    {
        PropertyNameAndValueList *front = next;
        next = 0;
        return front;
    }

// attributes
    PropertyName *name;
    ExpressionNode *value;
    PropertyNameAndValueList *next;
};

class PropertyName: public Node
{
public:
    virtual ~PropertyName() {}
};

class IdentifierPropertyName: public PropertyName
{
public:
    IdentifierPropertyName(QScriptNameIdImpl *n):
        id (n) {}

    virtual ~IdentifierPropertyName() {}

    virtual void accept0(Visitor *visitor);

// attributes
    QScriptNameIdImpl *id;
};

class StringLiteralPropertyName: public PropertyName
{
public:
    StringLiteralPropertyName(QScriptNameIdImpl *n):
        id (n) {}
    virtual ~StringLiteralPropertyName() {}

    virtual void accept0(Visitor *visitor);

// attributes
    QScriptNameIdImpl *id;
};

class NumericLiteralPropertyName: public PropertyName
{
public:
    NumericLiteralPropertyName(double n):
        id (n) {}
    virtual ~NumericLiteralPropertyName() {}

    virtual void accept0(Visitor *visitor);

// attributes
    double id;
};

class ArrayMemberExpression: public ExpressionNode
{
public:
    ArrayMemberExpression(ExpressionNode *b, ExpressionNode *e):
        base (b), expression (e) {}

    virtual ~ArrayMemberExpression() {}

    virtual void accept0(Visitor *visitor);

// attributes
    ExpressionNode *base;
    ExpressionNode *expression;
};

class FieldMemberExpression: public ExpressionNode
{
public:
    FieldMemberExpression(ExpressionNode *b, QScriptNameIdImpl *n):
        base (b), name (n) {}

    virtual ~FieldMemberExpression() {}

    virtual void accept0(Visitor *visitor);

// attributes
    ExpressionNode *base;
    QScriptNameIdImpl *name;
};

class NewMemberExpression: public ExpressionNode
{
public:
    NewMemberExpression(ExpressionNode *b, ArgumentList *a):
        base (b), arguments (a) {}

    virtual ~NewMemberExpression() {}

    virtual void accept0(Visitor *visitor);

// attributes
    ExpressionNode *base;
    ArgumentList *arguments;
};

class NewExpression: public ExpressionNode
{
public:
    NewExpression(ExpressionNode *e):
        expression (e) {}

    virtual ~NewExpression() {}

    virtual void accept0(Visitor *visitor);

// attributes
    ExpressionNode *expression;
};

class CallExpression: public ExpressionNode
{
public:
    CallExpression(ExpressionNode *b, ArgumentList *a):
        base (b), arguments (a) {}

    virtual ~CallExpression() {}

    virtual void accept0(Visitor *visitor);

// attributes
    ExpressionNode *base;
    ArgumentList *arguments;
};

class ArgumentList: public Node
{
public:
    ArgumentList(ExpressionNode *e):
        expression (e), next (this) {}

    ArgumentList(ArgumentList *previous, ExpressionNode *e):
        expression (e)
    {
        next = previous->next;
        previous->next = this;
    }

    virtual ~ArgumentList() {}

    virtual void accept0(Visitor *visitor);

    inline ArgumentList *finish ()
    {
        ArgumentList *front = next;
        next = 0;
        return front;
    }

// attributes
    ExpressionNode *expression;
    ArgumentList *next;
};

class PostIncrementExpression: public ExpressionNode
{
public:
    PostIncrementExpression(ExpressionNode *b):
        base (b) {}

    virtual ~PostIncrementExpression() {}

    virtual void accept0(Visitor *visitor);

// attributes
    ExpressionNode *base;
};

class PostDecrementExpression: public ExpressionNode
{
public:
    PostDecrementExpression(ExpressionNode *b):
        base (b) {}

    virtual ~PostDecrementExpression() {}

    virtual void accept0(Visitor *visitor);

// attributes
    ExpressionNode *base;
};

class DeleteExpression: public ExpressionNode
{
public:
    DeleteExpression(ExpressionNode *e):
        expression (e) {}
    virtual ~DeleteExpression() {}

    virtual void accept0(Visitor *visitor);

// attributes
    ExpressionNode *expression;
};

class VoidExpression: public ExpressionNode
{
public:
    VoidExpression(ExpressionNode *e):
        expression (e) {}

    virtual ~VoidExpression() {}

    virtual void accept0(Visitor *visitor);

// attributes
    ExpressionNode *expression;
};

class TypeOfExpression: public ExpressionNode
{
public:
    TypeOfExpression(ExpressionNode *e):
        expression (e) {}

    virtual ~TypeOfExpression() {}

    virtual void accept0(Visitor *visitor);

// attributes
    ExpressionNode *expression;
};

class PreIncrementExpression: public ExpressionNode
{
public:
    PreIncrementExpression(ExpressionNode *e):
        expression (e) {}

    virtual ~PreIncrementExpression() {}

    virtual void accept0(Visitor *visitor);

// attributes
    ExpressionNode *expression;
};

class PreDecrementExpression: public ExpressionNode
{
public:
    PreDecrementExpression(ExpressionNode *e):
        expression (e) {}

    virtual ~PreDecrementExpression() {}

    virtual void accept0(Visitor *visitor);

// attributes
    ExpressionNode *expression;
};

class UnaryPlusExpression: public ExpressionNode
{
public:
    UnaryPlusExpression(ExpressionNode *e):
        expression (e) {}

    virtual ~UnaryPlusExpression() {}

    virtual void accept0(Visitor *visitor);

// attributes
    ExpressionNode *expression;
};

class UnaryMinusExpression: public ExpressionNode
{
public:
    UnaryMinusExpression(ExpressionNode *e):
        expression (e) {}

    virtual ~UnaryMinusExpression() {}

    virtual void accept0(Visitor *visitor);

// attributes
    ExpressionNode *expression;
};

class TildeExpression: public ExpressionNode
{
public:
    TildeExpression(ExpressionNode *e):
        expression (e) {}

    virtual ~TildeExpression() {}

    virtual void accept0(Visitor *visitor);

// attributes
    ExpressionNode *expression;
};

class NotExpression: public ExpressionNode
{
public:
    NotExpression(ExpressionNode *e):
        expression (e) {}

    virtual ~NotExpression() {}

    virtual void accept0(Visitor *visitor);

// attributes
    ExpressionNode *expression;
};

class BinaryExpression: public ExpressionNode
{
public:
    BinaryExpression(ExpressionNode *l, int o, ExpressionNode *r):
        left (l), op (o), right (r) {}

    virtual ~BinaryExpression() {}

    virtual void accept0(Visitor *visitor);

// attributes
    ExpressionNode *left;
    int op;
    ExpressionNode *right;
};

class ConditionalExpression: public ExpressionNode
{
public:
    ConditionalExpression(ExpressionNode *e, ExpressionNode *t, ExpressionNode *f):
        expression (e), ok (t), ko (f) {}

    virtual ~ConditionalExpression() {}

    virtual void accept0(Visitor *visitor);

// attributes
    ExpressionNode *expression;
    ExpressionNode *ok;
    ExpressionNode *ko;
};

class Expression: public ExpressionNode // ### rename
{
public:
    Expression(ExpressionNode *l, ExpressionNode *r):
        left (l), right (r) {}

    virtual ~Expression() {}

    virtual void accept0(Visitor *visitor);

// attributes
    ExpressionNode *left;
    ExpressionNode *right;
};

class Block: public Statement
{
public:
    Block(StatementList *slist):
        statements (slist) {}

    virtual ~Block() {}

    virtual void accept0(Visitor *visitor);

// attributes
    StatementList *statements;
};

class StatementList: public Node
{
public:
    StatementList(Statement *stmt):
        statement (stmt), next (this) {}

    StatementList(StatementList *previous, Statement *stmt):
        statement (stmt)
    {
        next = previous->next;
        previous->next = this;
    }

    virtual ~StatementList() {}

    virtual void accept0(Visitor *visitor);

    inline StatementList *finish ()
    {
        StatementList *front = next;
        next = 0;
        return front;
    }

// attributes
    Statement *statement;
    StatementList *next;
};

class VariableStatement: public Statement
{
public:
    VariableStatement(VariableDeclarationList *vlist):
        declarations (vlist) {}

    virtual ~VariableStatement() {}

    virtual void accept0(Visitor *visitor);

// attributes
    VariableDeclarationList *declarations;
};

class VariableDeclaration: public Node
{
public:
    VariableDeclaration(QScriptNameIdImpl *n, ExpressionNode *e):
        name (n), expression (e), readOnly(false) {}

    virtual ~VariableDeclaration() {}

    virtual void accept0(Visitor *visitor);

// attributes
    QScriptNameIdImpl *name;
    ExpressionNode *expression;
    bool readOnly;
};

class VariableDeclarationList: public Node
{
public:
    VariableDeclarationList(VariableDeclaration *decl):
        declaration (decl), next (this) {}

    VariableDeclarationList(VariableDeclarationList *previous, VariableDeclaration *decl):
        declaration (decl)
    {
        next = previous->next;
        previous->next = this;
    }

    virtual ~VariableDeclarationList() {}

    virtual void accept0(Visitor *visitor);

    inline VariableDeclarationList *finish (bool readOnly)
    {
        VariableDeclarationList *front = next;
        next = 0;
        if (readOnly) {
            VariableDeclarationList *vdl;
            for (vdl = front; vdl != 0; vdl = vdl->next)
                vdl->declaration->readOnly = true;
        }
        return front;
    }

// attributes
    VariableDeclaration *declaration;
    VariableDeclarationList *next;
};

class EmptyStatement: public Statement
{
public:
    EmptyStatement() {}
    virtual ~EmptyStatement() {}

    virtual void accept0(Visitor *visitor);
};

class ExpressionStatement: public Statement
{
public:
    ExpressionStatement(ExpressionNode *e):
        expression (e) {}

    virtual ~ExpressionStatement() {}

    virtual void accept0(Visitor *visitor);

// attributes
    ExpressionNode *expression;
};

class IfStatement: public Statement
{
public:
    IfStatement(ExpressionNode *e, Statement *t, Statement *f = 0):
        expression (e), ok (t), ko (f) {}

    virtual ~IfStatement() {}

    virtual void accept0(Visitor *visitor);

// attributes
    ExpressionNode *expression;
    Statement *ok;
    Statement *ko;
};

class DoWhileStatement: public Statement
{
public:
    DoWhileStatement(Statement *stmt, ExpressionNode *e):
        statement (stmt), expression (e) {}

    virtual ~DoWhileStatement() {}

    virtual void accept0(Visitor *visitor);

// attributes
    Statement *statement;
    ExpressionNode *expression;
};

class WhileStatement: public Statement
{
public:
    WhileStatement(ExpressionNode *e, Statement *stmt):
        expression (e), statement (stmt) {}

    virtual ~WhileStatement() {}

    virtual void accept0(Visitor *visitor);

// attributes
    ExpressionNode *expression;
    Statement *statement;
};

class ForStatement: public Statement
{
public:
    ForStatement(ExpressionNode *i, ExpressionNode *c, ExpressionNode *e, Statement *stmt):
        initialiser (i), condition (c), expression (e), statement (stmt) {}

    virtual ~ForStatement() {}

    virtual void accept0(Visitor *visitor);

// attributes
    ExpressionNode *initialiser;
    ExpressionNode *condition;
    ExpressionNode *expression;
    Statement *statement;
};

class LocalForStatement: public Statement
{
public:
    LocalForStatement(VariableDeclarationList *vlist, ExpressionNode *c, ExpressionNode *e, Statement *stmt):
        declarations (vlist), condition (c), expression (e), statement (stmt) {}

    virtual ~LocalForStatement() {}

    virtual void accept0(Visitor *visitor);

// attributes
    VariableDeclarationList *declarations;
    ExpressionNode *condition;
    ExpressionNode *expression;
    Statement *statement;
};

class ForEachStatement: public Statement
{
public:
    ForEachStatement(ExpressionNode *i, ExpressionNode *e, Statement *stmt):
        initialiser (i), expression (e), statement (stmt) {}

    virtual ~ForEachStatement() {}

    virtual void accept0(Visitor *visitor);

// attributes
    ExpressionNode *initialiser;
    ExpressionNode *expression;
    Statement *statement;
};

class LocalForEachStatement: public Statement
{
public:
    LocalForEachStatement(VariableDeclaration *v, ExpressionNode *e, Statement *stmt):
        declaration (v), expression (e), statement (stmt) {}

    virtual ~LocalForEachStatement() {}

    virtual void accept0(Visitor *visitor);

// attributes
    VariableDeclaration *declaration;
    ExpressionNode *expression;
    Statement *statement;
};

class ContinueStatement: public Statement
{
public:
    ContinueStatement(QScriptNameIdImpl *l = 0):
        label (l) {}

    virtual ~ContinueStatement() {}

    virtual void accept0(Visitor *visitor);

// attributes
    QScriptNameIdImpl *label;
};

class BreakStatement: public Statement
{
public:
    BreakStatement(QScriptNameIdImpl *l = 0):
        label (l) {}

    virtual ~BreakStatement() {}

    virtual void accept0(Visitor *visitor);

// attributes
    QScriptNameIdImpl *label;
};

class ReturnStatement: public Statement
{
public:
    ReturnStatement(ExpressionNode *e):
        expression (e) {}

    virtual ~ReturnStatement() {}

    virtual void accept0(Visitor *visitor);

// attributes
    ExpressionNode *expression;
};

class WithStatement: public Statement
{
public:
    WithStatement(ExpressionNode *e, Statement *stmt):
        expression (e), statement (stmt) {}

    virtual ~WithStatement() {}

    virtual void accept0(Visitor *visitor);

// attributes
    ExpressionNode *expression;
    Statement *statement;
};

class SwitchStatement: public Statement
{
public:
    SwitchStatement(ExpressionNode *e, CaseBlock *b):
        expression (e), block (b) {}

    virtual ~SwitchStatement() {}

    virtual void accept0(Visitor *visitor);

// attributes
    ExpressionNode *expression;
    CaseBlock *block;
};

class CaseBlock: public Node
{
public:
    CaseBlock(CaseClauses *c, DefaultClause *d = 0, CaseClauses *r = 0):
        clauses (c), defaultClause (d), moreClauses (r) {}

    virtual ~CaseBlock() {}

    virtual void accept0(Visitor *visitor);

// attributes
    CaseClauses *clauses;
    DefaultClause *defaultClause;
    CaseClauses *moreClauses;
};

class CaseClauses: public Node
{
public:
    CaseClauses(CaseClause *c):
        clause (c), next (this) {}

    CaseClauses(CaseClauses *previous, CaseClause *c):
        clause (c)
    {
        next = previous->next;
        previous->next = this;
    }

    virtual ~CaseClauses() {}

    virtual void accept0(Visitor *visitor);

    inline CaseClauses *finish ()
    {
        CaseClauses *front = next;
        next = 0;
        return front;
    }

//attributes
    CaseClause *clause;
    CaseClauses *next;
};

class CaseClause: public Node
{
public:
    CaseClause(ExpressionNode *e, StatementList *slist):
        expression (e), statements (slist) {}

    virtual ~CaseClause() {}

    virtual void accept0(Visitor *visitor);

// attributes
    ExpressionNode *expression;
    StatementList *statements;
};

class DefaultClause: public Node
{
public:
    DefaultClause(StatementList *slist):
        statements (slist) {}

    virtual ~DefaultClause() {}

    virtual void accept0(Visitor *visitor);

// attributes
    StatementList *statements;
};

class LabelledStatement: public Statement
{
public:
    LabelledStatement(QScriptNameIdImpl *l, Statement *stmt):
        label (l), statement (stmt) {}

    virtual ~LabelledStatement() {}

    virtual void accept0(Visitor *visitor);

// attributes
    QScriptNameIdImpl *label;
    Statement *statement;
};

class ThrowStatement: public Statement
{
public:
    ThrowStatement(ExpressionNode *e):
        expression (e) {}

    virtual ~ThrowStatement() {}

    virtual void accept0(Visitor *visitor);

// attributes
    ExpressionNode *expression;
};

class TryStatement: public Statement
{
public:
    TryStatement(Statement *stmt, Catch *c, Finally *f):
        statement (stmt), catchExpression (c), finallyExpression (f) {}

    TryStatement(Statement *stmt, Finally *f):
        statement (stmt), catchExpression (0), finallyExpression (f) {}

    TryStatement(Statement *stmt, Catch *c):
        statement (stmt), catchExpression (c), finallyExpression (0) {}

    virtual ~TryStatement() {}

    virtual void accept0(Visitor *visitor);

// attributes
    Statement *statement;
    Catch *catchExpression;
    Finally *finallyExpression;
};

class Catch: public Node
{
public:
    Catch(QScriptNameIdImpl *n, Statement *stmt):
        name (n), statement (stmt) {}

    virtual ~Catch() {}

    virtual void accept0(Visitor *visitor);

// attributes
    QScriptNameIdImpl *name;
    Statement *statement;
};

class Finally: public Node
{
public:
    Finally(Statement *stmt):
        statement (stmt) {}

    virtual ~Finally() {}

    virtual void accept0(Visitor *visitor);

// attributes
    Statement *statement;
};

class FunctionDeclaration: public Node
{
public:
    FunctionDeclaration(QScriptNameIdImpl *n, FormalParameterList *f, FunctionBody *b):
        name (n), formals (f), body (b) {}

    virtual ~FunctionDeclaration() {}

    virtual void accept0(Visitor *visitor);

// attributes
    QScriptNameIdImpl *name;
    FormalParameterList *formals;
    FunctionBody *body;
};

class FunctionExpression: public ExpressionNode
{
public:
    FunctionExpression(QScriptNameIdImpl *n, FormalParameterList *f, FunctionBody *b):
        name (n), formals (f), body (b) {}

    virtual ~FunctionExpression() {}

    virtual void accept0(Visitor *visitor);

// attributes
    QScriptNameIdImpl *name;
    FormalParameterList *formals;
    FunctionBody *body;
};

class FormalParameterList: public Node
{
public:
    FormalParameterList(QScriptNameIdImpl *n):
        name (n), next (this) {}

    FormalParameterList(FormalParameterList *previous, QScriptNameIdImpl *n):
        name (n)
    {
        next = previous->next;
        previous->next = this;
    }

    virtual ~FormalParameterList() {}

    virtual void accept0(Visitor *visitor);

    inline FormalParameterList *finish ()
    {
        FormalParameterList *front = next;
        next = 0;
        return front;
    }

// attributes
    QScriptNameIdImpl *name;
    FormalParameterList *next;
};

class FunctionBody: public Node
{
public:
    FunctionBody(SourceElements *elts):
        elements (elts) {}

    virtual ~FunctionBody() {}

    virtual void accept0(Visitor *visitor);

// attributes
    SourceElements *elements;
};

class Program: public Node
{
public:
    Program(SourceElements *elts):
        elements (elts) {}

    virtual ~Program() {}

    virtual void accept0(Visitor *visitor);

// attributes
    SourceElements *elements;
};

class SourceElements: public Node
{
public:
    SourceElements(SourceElement *elt):
        element (elt), next (this) {}

    SourceElements(SourceElements *previous, SourceElement *elt):
        element (elt)
    {
        next = previous->next;
        previous->next = this;
    }

    virtual ~SourceElements() {}

    virtual void accept0(Visitor *visitor);

    inline SourceElements *finish ()
    {
        SourceElements *front = next;
        next = 0;
        return front;
    }

// attributes
    SourceElement *element;
    SourceElements *next;
};

class SourceElement: public Node
{
public:
    inline SourceElement():
        startLine(0), startColumn(0) {}

    virtual ~SourceElement() {}

    int startLine;
    int startColumn;
};

class FunctionSourceElement: public SourceElement
{
public:
    FunctionSourceElement(FunctionDeclaration *f):
        declaration (f) {}

    virtual ~FunctionSourceElement() {}

    virtual void accept0(Visitor *visitor);

// attributes
    FunctionDeclaration *declaration;
};

class StatementSourceElement: public SourceElement
{
public:
    StatementSourceElement(Statement *stmt):
        statement (stmt) {}

    virtual ~StatementSourceElement() {}

    virtual void accept0(Visitor *visitor);

// attributes
    Statement *statement;
};

} } // namespace AST


#endif

