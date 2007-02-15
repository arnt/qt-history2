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

#include "qscriptcontextfwd_p.h"
#include "qscriptenginefwd_p.h"
#include "qscriptnameid_p.h"

#include <QtCore/qnumeric.h>

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

inline QScriptContextPrivate::QScriptContextPrivate()
{
}

inline QScriptContextPrivate *QScriptContextPrivate::get(QScriptContext *q)
{
    return q->d_func();
}

inline QScriptContext *QScriptContextPrivate::create()
{
    return new QScriptContext;
}

inline QScriptEngine *QScriptContextPrivate::engine() const
{
    return m_activation.engine();
}

inline QScriptEnginePrivate *QScriptContextPrivate::enginePrivate() const
{
    return QScriptEnginePrivate::get(engine());
}

inline QScriptContext *QScriptContextPrivate::parentContext() const
{
    return previous;
}

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

inline bool QScriptContextPrivate::isNumerical(const QScriptValueImpl &v) const
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

inline bool QScriptContextPrivate::eq_cmp(const QScriptValueImpl &lhs, const QScriptValueImpl &rhs)
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

inline bool QScriptContextPrivate::lt_cmp(const QScriptValueImpl &lhs, const QScriptValueImpl &rhs)
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

inline bool QScriptContextPrivate::le_cmp(const QScriptValueImpl &lhs, const QScriptValueImpl &rhs)
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

inline bool QScriptContextPrivate::strict_eq_cmp(const QScriptValueImpl &lhs, const QScriptValueImpl &rhs)
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

inline QScriptValueImpl QScriptContextPrivate::throwTypeError(const QString &text)
{
    return throwError(QScriptContext::TypeError, text);
}

inline QScriptValueImpl QScriptContextPrivate::throwSyntaxError(const QString &text)
{
    return throwError(QScriptContext::SyntaxError, text);
}

inline QScriptValueImpl QScriptContextPrivate::thisObject() const
{
    return m_thisObject;
}

inline void QScriptContextPrivate::setThisObject(const QScriptValueImpl &object)
{
    m_thisObject = object;
}

inline QScriptValueImpl QScriptContextPrivate::callee() const
{
    return m_callee;
}

inline bool QScriptContextPrivate::calledAsConstructor() const
{
    return m_calledAsConstructor;
}

inline QScriptValueImpl QScriptContextPrivate::returnValue() const
{
    return m_result;
}

inline void QScriptContextPrivate::setReturnValue(const QScriptValueImpl &value)
{
    m_result = value;
}

inline QScriptValueImpl QScriptContextPrivate::activationObject() const
{
    return m_activation;
}

inline void QScriptContextPrivate::setActivationObject(const QScriptValueImpl &activation)
{
    m_activation = activation;
}

inline const QScriptInstruction *QScriptContextPrivate::instructionPointer()
{
    return iPtr;
}

inline void QScriptContextPrivate::setInstructionPointer(const QScriptInstruction *instructionPointer)
{
    iPtr = instructionPointer;
}

inline const QScriptValueImpl *QScriptContextPrivate::baseStackPointer() const
{
    return tempStack;
}

inline const QScriptValueImpl *QScriptContextPrivate::currentStackPointer() const
{
    return stackPtr;
}

inline QScriptContext::ExecutionState QScriptContextPrivate::state() const
{
    return m_state;
}

#endif
