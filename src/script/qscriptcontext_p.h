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

#ifndef QSCRIPTCONTEXT_P_H
#define QSCRIPTCONTEXT_P_H

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

#include <QtCore/QVariant>
#include <QtCore/qnumeric.h>

#include "qscriptengine.h"
#include "qscriptengine_p.h"
#include "qscriptvalue_p.h"
#include "qscriptcontext.h"
#include "qscriptecmaboolean_p.h"
#include "qscriptecmanumber_p.h"
#include "qscriptecmafunction_p.h"
#include "qscriptecmastring_p.h"
#include "qscriptecmadate_p.h"

class QScriptContextPrivate
{
    Q_DECLARE_PUBLIC(QScriptContext)
public:
    inline QScriptContextPrivate() { }

    static inline QScriptContextPrivate *get(QScriptContext *q)
        { return q->d_func(); }

    static inline QScriptContext *create()
        { return new QScriptContext; }

    inline QScriptEngine *engine() const
    { return m_activation.engine(); }
    inline QScriptEnginePrivate *enginePrivate() const
    { return QScriptEnginePrivate::get(engine()); }
    inline QScriptContext *parentContext() const
    { return previous; }

    inline void init(QScriptContext *parent);
    inline QScriptValueImpl argument(int index) const;
    inline int argumentCount() const;
    inline void throwException();
    inline void recover();

    inline bool isNumerical(const QScriptValueImpl &v) const
    {
        switch (v.type()) {
        case QScript::BooleanType:
        case QScript::IntegerType:
        case QScript::NumberType:
            return true;

        default:
            return false;
        }
    }

    inline bool eq_cmp(const QScriptValueImpl &lhs, const QScriptValueImpl &rhs)
    {
        QScriptEnginePrivate *eng = enginePrivate();

        if (lhs.type() == rhs.type()) {
            switch (lhs.type()) {
            case QScript::UndefinedType:
            case QScript::NullType:
                return true;

            case QScript::NumberType:
                return lhs.m_number_value == rhs.m_number_value;

            case QScript::IntegerType:
                return lhs.m_int_value == rhs.m_int_value;

            case QScript::BooleanType:
                return lhs.m_bool_value == rhs.m_bool_value;

            case QScript::StringType:
                if (lhs.m_string_value->unique && rhs.m_string_value->unique)
                    return lhs.m_string_value == rhs.m_string_value;
                return lhs.m_string_value->s == rhs.m_string_value->s;

            case QScript::VariantType:
                return lhs.m_object_value == rhs.m_object_value || lhs.toVariant() == rhs.toVariant();

            default:
                if (lhs.isObject())
                    return lhs.m_object_value == rhs.m_object_value;
                break;
            }
        }

        if (lhs.isNull() && rhs.isUndefined())
            return true;

        else if (lhs.isUndefined() && rhs.isNull())
            return true;

        else if (isNumerical(lhs) && rhs.isString())
            return eng->convertToNativeDouble(lhs) == eng->convertToNativeDouble(rhs);

        else if (lhs.isString() && isNumerical(rhs))
            return eng->convertToNativeString(lhs) == eng->convertToNativeString(rhs);

        return eq_cmp_helper(lhs, rhs, eng);
    }

    bool eq_cmp_helper(QScriptValueImpl lhs, QScriptValueImpl rhs, QScriptEnginePrivate *eng)
    {
        if (lhs.isObject() && ! rhs.isNull()) {
            lhs = eng->toPrimitive(lhs);

            if (lhs.isValid() && ! lhs.isObject())
                return eq_cmp(lhs, rhs);
        }

        else if (rhs.isObject() && ! lhs.isNull()) {
            rhs = eng->toPrimitive(rhs);

            if (rhs.isValid() && ! rhs.isObject())
                return eq_cmp(lhs, rhs);
        }

        return false;
    }

    inline bool lt_cmp(const QScriptValueImpl &lhs, const QScriptValueImpl &rhs)
    {
        QScriptEnginePrivate *eng = enginePrivate();

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

            case QScript::StringType:
                return lhs.m_string_value->s < rhs.m_string_value->s;

            default:
                break;
            } // switch
        }

        return lt_cmp_helper(lhs, rhs, eng);
    }

    bool lt_cmp_helper(QScriptValueImpl lhs, QScriptValueImpl rhs, QScriptEnginePrivate *eng)
    {
        if (lhs.isObject())
            lhs = eng->toPrimitive(lhs, QScriptValue::NumberTypeHint);

        if (rhs.isObject())
            rhs = eng->toPrimitive(rhs, QScriptValue::NumberTypeHint);

        if (lhs.isString() && rhs.isString())
            return eng->convertToNativeString(lhs) < eng->convertToNativeString(rhs);

        qsreal n1 = eng->convertToNativeDouble(lhs);
        qsreal n2 = eng->convertToNativeDouble(rhs);
        return n1 < n2;
    }

    inline bool le_cmp(const QScriptValueImpl &lhs, const QScriptValueImpl &rhs)
    {
        QScriptEnginePrivate *eng = enginePrivate();

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

            case QScript::StringType:
                return lhs.m_string_value->s <= rhs.m_string_value->s;

            default:
                break;
            } // switch
        }

        return le_cmp_helper(lhs, rhs, eng);
    }

    bool le_cmp_helper(QScriptValueImpl lhs, QScriptValueImpl rhs, QScriptEnginePrivate *eng)
    {
        if (lhs.isObject())
            lhs = eng->toPrimitive(lhs, QScriptValue::NumberTypeHint);

        if (rhs.isObject())
            rhs = eng->toPrimitive(rhs, QScriptValue::NumberTypeHint);

        if (lhs.isString() && rhs.isString())
            return eng->convertToNativeString(lhs) <= eng->convertToNativeString(rhs);

        qsreal n1 = eng->convertToNativeDouble(lhs);
        qsreal n2 = eng->convertToNativeDouble(rhs);
        return n1 <= n2;
    }

    inline static bool strict_eq_cmp(const QScriptValueImpl &lhs, const QScriptValueImpl &rhs)
    {
        if (lhs.type() != rhs.type())
            return false;

        switch (lhs.type()) {
        case QScript::UndefinedType:
        case QScript::NullType:
            return true;

        case QScript::NumberType:
            if (qIsNan(lhs.m_number_value) || qIsNan(rhs.m_number_value))
                return false;
            return lhs.m_number_value == rhs.m_number_value;

        case QScript::IntegerType:
            return lhs.m_int_value == rhs.m_int_value;

        case QScript::BooleanType:
            return lhs.m_bool_value == rhs.m_bool_value;

        case QScript::StringType:
            if (lhs.m_string_value->unique && rhs.m_string_value->unique)
                return lhs.m_string_value == rhs.m_string_value;
            return lhs.m_string_value->s == rhs.m_string_value->s;

        case QScript::VariantType:
            return lhs.m_object_value == rhs.m_object_value || lhs.toVariant() == rhs.toVariant();

        default:
            if (lhs.isObject())
                return lhs.m_object_value == rhs.m_object_value;
            break;
        }

        return false;
    }

    bool resolveField(QScriptValueImpl *stackPtr);
    void execute(QScript::Code *code);

    QScriptValueImpl throwError(QScriptContext::Error error, const QString &text);
    QScriptValueImpl throwError(const QString &text);

    QScriptValueImpl throwNotImplemented(const QString &name);
    QScriptValueImpl throwNotDefined(const QString &name);
    QScriptValueImpl throwNotDefined(QScriptNameIdImpl *nameId);

    inline QScriptValueImpl throwTypeError(const QString &text)
    { return throwError(QScriptContext::TypeError, text); }

    inline QScriptValueImpl throwSyntaxError(const QString &text)
    { return throwError(QScriptContext::SyntaxError, text); }

    inline QScriptValueImpl thisObject() const
        { return m_thisObject; }

    inline void setThisObject(const QScriptValueImpl &object)
        { m_thisObject = object; }

    inline QScriptValueImpl callee() const
        { return m_callee; }

    inline bool calledAsConstructor() const
        { return m_calledAsConstructor; }

    inline QScriptValueImpl returnValue() const
        { return m_result; }

    inline void setReturnValue(const QScriptValueImpl &value)
        { m_result = value; }

    inline QScriptValueImpl activationObject() const
        { return m_activation; }

    inline void setActivationObject(const QScriptValueImpl &activation)
        { m_activation = activation; }

    inline const QScriptInstruction *instructionPointer()
        { return iPtr; }

    inline void setInstructionPointer(const QScriptInstruction *instructionPointer)
        {iPtr = instructionPointer; }

    inline const QScriptValueImpl *baseStackPointer() const
        { return tempStack; }

    inline const QScriptValueImpl *currentStackPointer() const
        { return stackPtr; }

    inline QScriptContext::ExecutionState state() const
        { return m_state; }

public:
    QScriptContext *previous;
    int argc;
    QScriptContext::ExecutionState m_state;

    QScriptValueImpl m_activation;
    QScriptValueImpl m_thisObject;
    QScriptValueImpl m_result;
    QScriptValueImpl m_scopeChain;
    QScriptValueImpl m_callee;

    QScript::AST::Node *abstractSyntaxTree;
    QScriptValueImpl *args;
    QScriptValueImpl *tempStack;
    QScriptValueImpl *stackPtr;

    const QScriptInstruction *iPtr;
    const QScriptInstruction *firstInstruction;
    const QScriptInstruction *lastInstruction;

    int currentLine;
    int currentColumn;

    int errorLineNumber;

    QScriptNameIdImpl *m_functionNameId;
    bool catching;
    bool m_calledAsConstructor;

    QScriptContext *q_ptr;
};

inline void QScriptContextPrivate::init(QScriptContext *parent)
{
    m_state = QScriptContext::NormalState;
    previous = parent;
    args = 0;
    argc = 0;
    abstractSyntaxTree = 0;
    iPtr = firstInstruction = lastInstruction = 0;
    stackPtr = tempStack = (parent != 0) ? parent->d_func()->stackPtr : 0;
    m_activation.invalidate();
    m_thisObject.invalidate();
    m_result.invalidate();
    m_scopeChain.invalidate();
    m_callee.invalidate();
    currentLine = 0;
    currentColumn = 0;
    errorLineNumber = 0;
    m_functionNameId = 0;
    m_calledAsConstructor = false;
}

inline QScriptValueImpl QScriptContextPrivate::argument(int index) const
{
    if (index >= argc)
        return QScriptEnginePrivate::get(engine())->undefinedValue();

    else if (args != 0)
        return args[index];

    return tempStack[index - argc + 1];
}

inline int QScriptContextPrivate::argumentCount() const
{
    return argc;
}

inline void QScriptContextPrivate::throwException()
{
    m_state = QScriptContext::ExceptionState;
}

inline void QScriptContextPrivate::recover()
{
    m_state = QScriptContext::NormalState;
    errorLineNumber = 0;
}

#endif

