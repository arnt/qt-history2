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

    inline QScriptEngine *engine() const
    { return activation.impl()->engine(); }
    inline QScriptEnginePrivate *enginePrivate() const
    { return QScriptEnginePrivate::get(engine()); }

    inline void init(QScriptContext *parent);
    inline QScriptValue argument(int index) const;
    inline void throwException();
    inline void recover();

    static inline bool isValid(const QScriptValue &v)
        { return v.m_class && v.m_class->engine(); }
    static inline bool isUndefined(const QScriptValue &v)
        { return v.m_class && (v.m_class->type() == QScript::UndefinedType); }
    static inline bool isNull(const QScriptValue &v)
        { return v.m_class && (v.m_class->type() == QScript::NullType); }
    static inline bool isNumber(const QScriptValue &v)
        { return v.m_class && (v.m_class->type() == QScript::NumberType); }
    static inline bool isString(const QScriptValue &v)
        { return v.m_class && (v.m_class->type() == QScript::StringType); }
    static inline bool isObject(const QScriptValue &v)
        { return v.m_class && (v.m_class->type() & QScript::ObjectBased); }
    static inline QScript::Type type(const QScriptValue &v)
        { return v.m_class->type(); }
    static inline void invalidate(QScriptValue &v)
        { v.m_class = 0; }

    inline bool isNumerical(const QScriptValue &v) const
    {
        switch (type(v)) {
        case QScript::BooleanType:
        case QScript::IntegerType:
        case QScript::NumberType:
            return true;

        default:
            return false;
        }
    }

    inline bool eq_cmp(const QScriptValue &lhs, const QScriptValue &rhs)
    {
        QScriptEnginePrivate *eng = enginePrivate();

        if (type(lhs) == type(rhs)) {
            switch (type(lhs)) {
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
                if (isObject(lhs))
                    return lhs.m_object_value == rhs.m_object_value;
                break;
            }
        }

        if (isNull(lhs) && isUndefined(rhs))
            return true;

        else if (isUndefined(lhs) && isNull(rhs))
            return true;

        else if (isNumerical(lhs) && isString(rhs))
            return eng->convertToNativeDouble(lhs) == eng->convertToNativeDouble(rhs);

        else if (isString(lhs) && isNumerical(rhs))
            return eng->convertToNativeString(lhs) == eng->convertToNativeString(rhs);

        return eq_cmp_helper(lhs, rhs, eng);
    }

    bool eq_cmp_helper(QScriptValue lhs, QScriptValue rhs, QScriptEnginePrivate *eng)
    {
        if (isObject(lhs) && ! isNull(rhs)) {
            lhs = eng->toPrimitive(lhs);

            if (isValid(lhs) && ! isObject(lhs))
                return eq_cmp(lhs, rhs);
        }

        else if (isObject(rhs) && ! isNull(lhs)) {
            rhs = eng->toPrimitive(rhs);

            if (isValid(rhs) && ! isObject(rhs))
                return eq_cmp(lhs, rhs);
        }

        return false;
    }

    inline bool lt_cmp(const QScriptValue &lhs, const QScriptValue &rhs)
    {
        QScriptEnginePrivate *eng = enginePrivate();

        if (type(lhs) == type(rhs)) {
            switch (type(lhs)) {
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

    bool lt_cmp_helper(QScriptValue lhs, QScriptValue rhs, QScriptEnginePrivate *eng)
    {
        if (isObject(lhs))
            lhs = eng->toPrimitive(lhs, QScriptValue::NumberTypeHint);

        if (isObject(rhs))
            rhs = eng->toPrimitive(rhs, QScriptValue::NumberTypeHint);

        if (isString(lhs) && isString(rhs))
            return eng->convertToNativeString(lhs) < eng->convertToNativeString(rhs);

        qnumber n1 = eng->convertToNativeDouble(lhs);
        qnumber n2 = eng->convertToNativeDouble(rhs);
        return n1 < n2;
    }

    inline bool le_cmp(const QScriptValue &lhs, const QScriptValue &rhs)
    {
        QScriptEnginePrivate *eng = enginePrivate();

        if (type(lhs) == type(rhs)) {
            switch (type(lhs)) {
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

    bool le_cmp_helper(QScriptValue lhs, QScriptValue rhs, QScriptEnginePrivate *eng)
    {
        if (isObject(lhs))
            lhs = eng->toPrimitive(lhs, QScriptValue::NumberTypeHint);

        if (isObject(rhs))
            rhs = eng->toPrimitive(rhs, QScriptValue::NumberTypeHint);

        if (isString(lhs) && isString(rhs))
            return eng->convertToNativeString(lhs) <= eng->convertToNativeString(rhs);

        qnumber n1 = eng->convertToNativeDouble(lhs);
        qnumber n2 = eng->convertToNativeDouble(rhs);
        return n1 <= n2;
    }

    inline bool strict_eq_cmp(const QScriptValue &lhs, const QScriptValue &rhs)
    {
        if (type(lhs) != type(rhs))
            return false;

        switch (type(lhs)) {
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
            if (isObject(lhs))
                return lhs.m_object_value == rhs.m_object_value;
            break;
        }

        return false;
    }

    bool resolveField(QScriptValue *stackPtr);
    void execute(QScript::Code *code);

    QScriptValue throwNotImplemented(const QString &name);
    QScriptValue throwNotDefined(const QString &name);
    QScriptValue throwNotDefined(QScriptNameIdImpl *nameId);

    inline QScriptValue throwTypeError(const QString &text)
        { return q_ptr->throwError(QScriptContext::TypeError, text); }
    inline QScriptValue throwSyntaxError(const QString &text)
        { return q_ptr->throwError(QScriptContext::SyntaxError, text); }

public:
    QScriptContext *previous;
    int argc;
    QScriptContext::State state;

    QScriptValue activation;
    QScriptValue thisObject;
    QScriptValue result;
    QScriptValue scopeChain;
    QScriptValue callee;

    QScript::AST::Node *abstractSyntaxTree;
    QScriptValue *args;
    QScriptValue *tempStack;
    QScriptValue *stackPtr;

    const QScriptInstruction *iPtr;
    const QScriptInstruction *firstInstruction;
    const QScriptInstruction *lastInstruction;

    int currentLine;
    int currentColumn;

    int errorLineNumber;

    QScriptNameIdImpl *functionNameId;
    bool catching;
    bool calledAsConstructor;

    QScriptContext *q_ptr;
};

inline void QScriptContextPrivate::init(QScriptContext *parent)
{
    state = QScriptContext::Normal;
    previous = parent;
    args = 0;
    argc = 0;
    abstractSyntaxTree = 0;
    iPtr = firstInstruction = lastInstruction = 0;
    stackPtr = tempStack = (parent != 0) ? parent->d_func()->stackPtr : 0;
    invalidate(activation);
    invalidate(thisObject);
    invalidate(result);
    invalidate(scopeChain);
    invalidate(callee);
    currentLine = 0;
    currentColumn = 0;
    errorLineNumber = 0;
    functionNameId = 0;
    calledAsConstructor = false;
}

inline QScriptValue QScriptContextPrivate::argument(int index) const
{
    if (index >= argc)
        return engine()->undefinedScriptValue();

    else if (args != 0)
        return args[index];

    return tempStack[index - argc + 1];
}

inline void QScriptContextPrivate::throwException()
{
    state = QScriptContext::Exception;
}

inline void QScriptContextPrivate::recover()
{
    state = QScriptContext::Normal;
    errorLineNumber = 0;
}

#endif

