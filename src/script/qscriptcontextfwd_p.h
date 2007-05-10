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

#ifndef QSCRIPTCONTEXTFWD_P_H
#define QSCRIPTCONTEXTFWD_P_H

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

#include "qscriptvalueimplfwd_p.h"
#include "qscriptcontext.h"

#include <QtCore/qobjectdefs.h>

namespace QScript {
    namespace AST {
    class Node;
    }
class Code;
}

class QScriptContextPrivate
{
    Q_DECLARE_PUBLIC(QScriptContext)
public:
    inline QScriptContextPrivate();

    static inline QScriptContextPrivate *get(QScriptContext *q);

    static inline QScriptContext *create();

    inline QScriptEngine *engine() const;
    inline QScriptEnginePrivate *enginePrivate() const;
    inline QScriptContext *parentContext() const;

    inline void init(QScriptContext *parent);
    inline QScriptValueImpl argument(int index) const;
    inline int argumentCount() const;
    inline void throwException();
    inline bool hasUncaughtException() const;
    inline void recover();

    inline bool isNumerical(const QScriptValueImpl &v) const;

    inline bool eq_cmp(const QScriptValueImpl &lhs, const QScriptValueImpl &rhs);

    bool eq_cmp_helper(QScriptValueImpl lhs, QScriptValueImpl rhs);

#if defined(Q_CC_GNU) && __GNUC__ <= 3
    bool lt_cmp(QScriptValueImpl lhs, QScriptValueImpl rhs);
#else
    bool lt_cmp(const QScriptValueImpl &lhs, const QScriptValueImpl &rhs)
    {
        if (lhs.type() == rhs.type()) {
            switch (lhs.type()) {
            case QScript::UndefinedType:
            case QScript::NullType:
                return false;

            case QScript::NumberType:
                return lhs.m_number_value < rhs.m_number_value;

            case QScript::IntegerType:
                return lhs.m_int_value < rhs.m_int_value;

            case QScript::BooleanType:
                return lhs.m_bool_value < rhs.m_bool_value;

            default:
                break;
            } // switch
        }

        return lt_cmp_helper(lhs, rhs);
    }

    bool lt_cmp_helper(QScriptValueImpl lhs, QScriptValueImpl rhs);
#endif

    bool le_cmp(const QScriptValueImpl &lhs, const QScriptValueImpl &rhs)
    {
        if (lhs.type() == rhs.type()) {
            switch (lhs.type()) {
            case QScript::UndefinedType:
            case QScript::NullType:
                return true;

            case QScript::NumberType:
                return lhs.m_number_value <= rhs.m_number_value;

            case QScript::IntegerType:
                return lhs.m_int_value <= rhs.m_int_value;

            case QScript::BooleanType:
                return lhs.m_bool_value <= rhs.m_bool_value;

            default:
                break;
            } // switch
        }

        return le_cmp_helper(lhs, rhs);
    }

    bool le_cmp_helper(QScriptValueImpl lhs, QScriptValueImpl rhs);

    static inline bool strict_eq_cmp(const QScriptValueImpl &lhs, const QScriptValueImpl &rhs);

    bool resolveField(QScriptEnginePrivate *eng, QScriptValueImpl *stackPtr,
                      QScriptValueImpl *value);

    void execute(QScript::Code *code);

    QScriptValueImpl throwError(QScriptContext::Error error, const QString &text);
    QScriptValueImpl throwError(const QString &text);

    void setDebugInformation(QScriptValueImpl *error) const;

    QScriptValueImpl throwNotImplemented(const QString &name);
    QScriptValueImpl throwNotDefined(const QString &name);
    QScriptValueImpl throwNotDefined(QScriptNameIdImpl *nameId);

    inline QScriptValueImpl throwTypeError(const QString &text);
    inline QScriptValueImpl throwSyntaxError(const QString &text);

    inline QScriptValueImpl thisObject() const;
    inline void setThisObject(const QScriptValueImpl &object);

    inline QScriptValueImpl callee() const;
    inline bool calledAsConstructor() const;

    inline QScriptValueImpl returnValue() const;
    inline void setReturnValue(const QScriptValueImpl &value);

    inline QScriptValueImpl activationObject() const;
    inline void setActivationObject(const QScriptValueImpl &activation);

    inline const QScriptInstruction *instructionPointer();
    inline void setInstructionPointer(const QScriptInstruction *instructionPointer);

    inline const QScriptValueImpl *baseStackPointer() const;
    inline const QScriptValueImpl *currentStackPointer() const;

    inline QScriptContext::ExecutionState state() const;

public:
    QScriptContext *previous;
    int argc;
    QScriptContext::ExecutionState m_state;

    QScriptValueImpl m_activation;
    QScriptValueImpl m_thisObject;
    QScriptValueImpl m_result;
    QScriptValueImpl m_scopeChain;
    QScriptValueImpl m_callee;
    QScriptValueImpl m_arguments;

    QScriptValueImpl *args;
    QScriptValueImpl *tempStack;
    QScriptValueImpl *stackPtr;

    const QScriptInstruction *iPtr;
    const QScriptInstruction *firstInstruction;
    const QScriptInstruction *lastInstruction;

    int currentLine;
    int currentColumn;

    int errorLineNumber;

    bool catching;
    bool m_calledAsConstructor;

    QScriptContext *q_ptr;
};

#endif
