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

#include "qscriptengine.h"
#include "qscriptcontext.h"

#include "qscriptengine_p.h"
#include "qscriptecmastring_p.h"
#include "qscriptecmaobject_p.h"
#include "qscriptecmafunction_p.h"
#include "qscriptecmaerror_p.h"
#include <QtCore/QtDebug>

namespace QScript { namespace Ecma {

static QString getMessage(QScriptContext *context)
{
    if (context->argumentCount() > 0)
        return context->argument(0).toString();
    return QString();
}

static QScriptValue method_EvalError(QScriptEngine *eng, QScriptClassInfo *)
{
    QScriptContext *context = eng->currentContext();
    QScriptValue result;
    QScriptEnginePrivate::get(eng)->errorConstructor->newError(&result, getMessage(context));
    return result;
}

static QScriptValue method_RangeError(QScriptEngine *eng, QScriptClassInfo *)
{
    QScriptContext *context = eng->currentContext();
    QScriptValue result;
    QScriptEnginePrivate::get(eng)->errorConstructor->newRangeError(&result, getMessage(context));
    return result;
}

static QScriptValue method_ReferenceError(QScriptEngine *eng, QScriptClassInfo *)
{
    QScriptContext *context = eng->currentContext();
    QScriptValue result;
    QScriptEnginePrivate::get(eng)->errorConstructor->newReferenceError(&result, getMessage(context));
    return result;
}

static QScriptValue method_SyntaxError(QScriptEngine *eng, QScriptClassInfo *)
{
    QScriptContext *context = eng->currentContext();
    QScriptValue result;
    QScriptEnginePrivate::get(eng)->errorConstructor->newSyntaxError(&result, getMessage(context));
    return result;
}

static QScriptValue method_TypeError(QScriptEngine *eng, QScriptClassInfo *)
{
    QScriptContext *context = eng->currentContext();
    QScriptValue result;
    QScriptEnginePrivate::get(eng)->errorConstructor->newTypeError(&result, getMessage(context));
    return result;
}

static QScriptValue method_UriError(QScriptEngine *eng, QScriptClassInfo *)
{
    QScriptContext *context = eng->currentContext();
    QScriptValue result;
    QScriptEnginePrivate::get(eng)->errorConstructor->newURIError(&result, getMessage(context));
    return result;
}

Error::Error(QScriptEngine *eng):
    Core(eng)
{
    QScriptEnginePrivate *eng_p = QScriptEnginePrivate::get(eng);
    m_objectClass = eng_p->registerClass(QLatin1String("Error"));

    eng_p->newFunction(&ctor, this);
    newErrorPrototype(&publicPrototype, QScriptValue(), ctor, QLatin1String("Error"));
    publicPrototype.setProperty(QLatin1String("toString"),
                                eng_p->createFunction(method_toString, 0, m_objectClass),
                                QScriptValue::SkipInEnumeration);

    // native errors

    evalErrorCtor = eng_p->createFunction(method_EvalError, 3, m_objectClass);
    rangeErrorCtor = eng_p->createFunction(method_RangeError, 3, m_objectClass);
    referenceErrorCtor = eng_p->createFunction(method_ReferenceError, 3, m_objectClass);
    syntaxErrorCtor = eng_p->createFunction(method_SyntaxError, 3, m_objectClass);
    typeErrorCtor = eng_p->createFunction(method_TypeError, 3, m_objectClass);
    uriErrorCtor = eng_p->createFunction(method_UriError, 3, m_objectClass);

    newErrorPrototype(&evalErrorPrototype, publicPrototype,
                      evalErrorCtor, QLatin1String("EvalError"));
    newErrorPrototype(&rangeErrorPrototype, publicPrototype,
                      rangeErrorCtor, QLatin1String("RangeError"));
    newErrorPrototype(&referenceErrorPrototype, publicPrototype,
                      referenceErrorCtor, QLatin1String("ReferenceError"));
    newErrorPrototype(&syntaxErrorPrototype, publicPrototype,
                      syntaxErrorCtor, QLatin1String("SyntaxError"));
    newErrorPrototype(&typeErrorPrototype, publicPrototype,
                      typeErrorCtor, QLatin1String("TypeError"));
    newErrorPrototype(&uriErrorPrototype, publicPrototype,
                      uriErrorCtor, QLatin1String("URIError"));
}

Error::~Error()
{
}

void Error::execute(QScriptContext *context)
{
    QString message = QString();

    if (context->argumentCount() > 0)
        message = context->argument(0).toString();

    QScriptValue result;
    newError(&result, publicPrototype, message);
    context->setReturnValue(result);
}

void Error::newError(QScriptValue *result, const QString &message)
{
    newError(result, publicPrototype, message);
}

void Error::newEvalError(QScriptValue *result, const QString &message)
{
    newError(result, evalErrorPrototype, message);
}

void Error::newRangeError(QScriptValue *result, const QString &message)
{
    newError(result, rangeErrorPrototype, message);
}

void Error::newReferenceError(QScriptValue *result, const QString &message)
{
    newError(result, referenceErrorPrototype, message);
}

void Error::newSyntaxError(QScriptValue *result, const QString &message)
{
    newError(result, syntaxErrorPrototype, message);
}

void Error::newTypeError(QScriptValue *result, const QString &message)
{
    newError(result, typeErrorPrototype, message);
}

void Error::newURIError(QScriptValue *result, const QString &message)
{
    newError(result, uriErrorPrototype, message);
}

void Error::newError(QScriptValue *result, const QScriptValue &proto,
                     const QString &message)
{
    QScriptEnginePrivate::get(engine())->newObject(result, proto, classInfo());
    result->setProperty(QLatin1String("message"), QScriptValue(engine(), message));
}

void Error::newErrorPrototype(QScriptValue *result, const QScriptValue &proto,
                              QScriptValue &ztor, const QString &name)
{
    newError(result, proto);
    result->setProperty(QLatin1String("name"), QScriptValue(engine(), name));
    result->setProperty(QLatin1String("constructor"), ztor,
                        QScriptValue::Undeletable
                        | QScriptValue::SkipInEnumeration);
    ztor.setProperty(QLatin1String("prototype"), *result,
                     QScriptValue::Undeletable
                     | QScriptValue::ReadOnly
                     | QScriptValue::SkipInEnumeration);
}

QScriptValue Error::method_toString(QScriptEngine *eng, QScriptClassInfo *)
{
    QScriptContext *context = eng->currentContext();
    QScriptValue name = context->thisObject().property(QLatin1String("name"),
                                                       QScriptValue::ResolvePrototype);
    QScriptValue message = context->thisObject().property(QLatin1String("message"),
                                                          QScriptValue::ResolvePrototype);
    QString result = QLatin1String("");
    if (name.isValid())
        result = name.toString();
    if (message.isValid()) {
        QString str = message.toString();
        if (!str.isEmpty()) {
            if (!result.isEmpty())
                result += QLatin1String(": ");
            result += str;
        }
    }
    return (QScriptValue(eng, result));
}

} } // namespace QSA::Ecma
