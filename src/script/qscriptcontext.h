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

#ifndef QSCRIPTCONTEXT_H
#define QSCRIPTCONTEXT_H

#include <QtCore/qobjectdefs.h>

#include <QtScript/qscriptvalue.h>

QT_BEGIN_HEADER

QT_MODULE(Script)

class QScriptInstruction;

class QScriptContextPrivate;

class Q_SCRIPT_EXPORT QScriptContext
{
public:
    enum ExecutionState {
        NormalState,
        ExceptionState
    };

    enum Error {
        GenericError,
        ReferenceError,
        SyntaxError,
        TypeError,
        RangeError,
        URIError
    };

    ~QScriptContext();

    QScriptContext *parentContext() const;
    QScriptEngine *engine() const;

    ExecutionState state() const;
    QScriptValue callee() const;

    int argumentCount() const;
    QScriptValue argument(int index) const;

    QScriptValue returnValue() const;
    void setReturnValue(const QScriptValue &result);

    QScriptValue activationObject() const;
    void setActivationObject(const QScriptValue &activation);

    QScriptValue thisObject() const;
    void setThisObject(const QScriptValue &thisObject);

    bool calledAsConstructor() const;

    const QScriptInstruction *instructionPointer() const;
    void setInstructionPointer(const QScriptInstruction *instructionPointer);

    const QScriptInstruction *firstInstruction() const;
    const QScriptInstruction *lastInstruction() const;

    const QScriptValue *baseStackPointer() const;
    const QScriptValue *currentStackPointer() const;

    QScriptValue throwValue(const QScriptValue &value);
    QScriptValue throwError(Error error, const QString &text);
    QScriptValue throwError(const QString &text);

    int errorLineNumber() const;

private:
    QScriptContext();

    QScriptContextPrivate *d_ptr;

    Q_DECLARE_PRIVATE(QScriptContext)
    Q_DISABLE_COPY(QScriptContext)
};

QT_END_HEADER

#endif

