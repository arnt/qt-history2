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
#include "qscriptcontext_p.h"
#include "qscriptengine_p.h"
#include "qscriptecmastring_p.h"
#include "qscriptecmaobject_p.h"
#include "qscriptecmafunction_p.h"
#include "qscriptecmaerror_p.h"
#include <QtCore/QtDebug>

namespace QScript { namespace Ecma {

static QString getMessage(QScriptContextPrivate *context)
{
    if (context->argumentCount() > 0)
        return context->argument(0).toString();
    return QString();
}

static QScriptValueImpl method_EvalError(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *)
{
    QScriptValueImpl result;
    eng->errorConstructor->newError(&result, getMessage(context));
    return result;
}

static QScriptValueImpl method_RangeError(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *)
{
    QScriptValueImpl result;
    eng->errorConstructor->newRangeError(&result, getMessage(context));
    return result;
}

static QScriptValueImpl method_ReferenceError(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *)
{
    QScriptValueImpl result;
    eng->errorConstructor->newReferenceError(&result, getMessage(context));
    return result;
}

static QScriptValueImpl method_SyntaxError(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *)
{
    QScriptValueImpl result;
    eng->errorConstructor->newSyntaxError(&result, getMessage(context));
    return result;
}

static QScriptValueImpl method_TypeError(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *)
{
    QScriptValueImpl result;
    eng->errorConstructor->newTypeError(&result, getMessage(context));
    return result;
}

static QScriptValueImpl method_UriError(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *)
{
    QScriptValueImpl result;
    eng->errorConstructor->newURIError(&result, getMessage(context));
    return result;
}

Error::Error(QScriptEnginePrivate *eng):
    Core(eng)
{
    m_objectClass = eng->registerClass(QLatin1String("Error"));

    eng->newFunction(&ctor, this);
    newErrorPrototype(&publicPrototype, QScriptValueImpl(), ctor, QLatin1String("Error"));
    publicPrototype.setProperty(QLatin1String("toString"),
                                eng->createFunction(method_toString, 0, m_objectClass),
                                QScriptValue::SkipInEnumeration);

    // native errors

    evalErrorCtor = eng->createFunction(method_EvalError, 3, m_objectClass);
    rangeErrorCtor = eng->createFunction(method_RangeError, 3, m_objectClass);
    referenceErrorCtor = eng->createFunction(method_ReferenceError, 3, m_objectClass);
    syntaxErrorCtor = eng->createFunction(method_SyntaxError, 3, m_objectClass);
    typeErrorCtor = eng->createFunction(method_TypeError, 3, m_objectClass);
    uriErrorCtor = eng->createFunction(method_UriError, 3, m_objectClass);

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

void Error::execute(QScriptContextPrivate *context)
{
    QString message = QString();

    if (context->argumentCount() > 0)
        message = context->argument(0).toString();

    QScriptValueImpl result;
    newError(&result, publicPrototype, message);
    context->setReturnValue(result);
}

void Error::newError(QScriptValueImpl *result, const QString &message)
{
    newError(result, publicPrototype, message);
}

void Error::newEvalError(QScriptValueImpl *result, const QString &message)
{
    newError(result, evalErrorPrototype, message);
}

void Error::newRangeError(QScriptValueImpl *result, const QString &message)
{
    newError(result, rangeErrorPrototype, message);
}

void Error::newReferenceError(QScriptValueImpl *result, const QString &message)
{
    newError(result, referenceErrorPrototype, message);
}

void Error::newSyntaxError(QScriptValueImpl *result, const QString &message)
{
    newError(result, syntaxErrorPrototype, message);
}

void Error::newTypeError(QScriptValueImpl *result, const QString &message)
{
    newError(result, typeErrorPrototype, message);
}

void Error::newURIError(QScriptValueImpl *result, const QString &message)
{
    newError(result, uriErrorPrototype, message);
}

void Error::newError(QScriptValueImpl *result, const QScriptValueImpl &proto,
                     const QString &message)
{
    engine()->newObject(result, proto, classInfo());
    result->setProperty(QLatin1String("message"), QScriptValueImpl(engine(), message));
}

void Error::newErrorPrototype(QScriptValueImpl *result, const QScriptValueImpl &proto,
                              QScriptValueImpl &ztor, const QString &name)
{
    newError(result, proto);
    result->setProperty(QLatin1String("name"), QScriptValueImpl(engine(), name));
    result->setProperty(QLatin1String("constructor"), ztor,
                        QScriptValue::Undeletable
                        | QScriptValue::SkipInEnumeration);
    ztor.setProperty(QLatin1String("prototype"), *result,
                     QScriptValue::Undeletable
                     | QScriptValue::ReadOnly
                     | QScriptValue::SkipInEnumeration);
}

QScriptValueImpl Error::method_toString(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *)
{
    QScriptValueImpl name = context->thisObject().property(QLatin1String("name"),
                                                           QScriptValue::ResolvePrototype);
    QScriptValueImpl message = context->thisObject().property(QLatin1String("message"),
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
    return (QScriptValueImpl(eng, result));
}

} } // namespace QSA::Ecma
