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

#ifndef QSCRIPTASM_P_H
#define QSCRIPTASM_P_H

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
#include <QtCore/qvector.h>

#include "qscriptvalue.h"

class QTextStream;

class QScriptInstruction
{
public:
    enum Operator {
#define Q_SCRIPT_DEFINE_OPERATOR(op) OP_##op,
#include "instruction.table"
#undef Q_SCRIPT_DEFINE_OPERATOR
    };

public:
    Operator op;
    QScriptValue operand[2];
#if defined(Q_SCRIPT_DIRECT_CODE)
    void *code;
#endif

    void print(QTextStream &out) const;

    static const char *opcode[];
};

namespace QScript {

class ExceptionHandlerDescriptor
{
public:
    ExceptionHandlerDescriptor() {}

    ExceptionHandlerDescriptor(
        int startInstruction,
        int endInstruction,
        int handlerInstruction)
        : m_startInstruction(startInstruction),
          m_endInstruction(endInstruction),
          m_handlerInstruction(handlerInstruction) {}

    inline int startInstruction() const { return m_startInstruction; }
    inline int endInstruction() const { return m_endInstruction; }
    inline int handlerInstruction() const { return m_handlerInstruction; }

private:
    int m_startInstruction;
    int m_endInstruction;
    int m_handlerInstruction;
};

class CompilationUnit
{
public:
    CompilationUnit(): m_valid(true) {}

    bool isValid() const { return m_valid; }
    void setValid(bool v) { m_valid = v; }

    QString errorMessage() const { return m_errorMessage; }
    void setErrorMessage(const QString &errorMessage) { m_errorMessage = errorMessage; }

    QVector<QScriptInstruction> instructions() const { return m_instructions; }
    void setInstructions(const QVector<QScriptInstruction> &instructions) { m_instructions = instructions; }

    QVector<QScript::ExceptionHandlerDescriptor> exceptionHandlers() const { return m_exceptionHandlers; }
    void setExceptionHandlers(const QVector<QScript::ExceptionHandlerDescriptor> &exceptionHandlers) { m_exceptionHandlers = exceptionHandlers; }

private:
    bool m_valid;
    QString m_errorMessage;
    QVector<QScriptInstruction> m_instructions;
    QVector<QScript::ExceptionHandlerDescriptor> m_exceptionHandlers;
};

class Code
{
public:
    Code();
    ~Code();

    void init(const QScript::CompilationUnit &compilation);

public: // attributes
    bool optimized;
    QScriptInstruction *firstInstruction;
    QScriptInstruction *lastInstruction;
    QVector<ExceptionHandlerDescriptor> exceptionHandlers;
    QScriptValue value;

private:
    Q_DISABLE_COPY(Code)
};


} // namespace QScript

#endif
