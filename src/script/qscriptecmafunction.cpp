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

#include "qscriptengine_p.h"
#include "qscriptcontext_p.h"
#include "qscriptvalue_p.h"
#include "qscriptecmafunction_p.h"
#include "qscriptecmastring_p.h"
#include "qscriptecmaobject_p.h"
#include "qscriptecmaarray_p.h"
#include "qscriptcompiler_p.h"

#include <QtCore/QtDebug>

#ifndef QT_NO_QOBJECT
#include "qscriptextqobject_p.h"
#include <QtCore/QMetaMethod>
#endif

namespace QScript { namespace Ecma {

Function::Function(QScriptEngine *eng, QScriptClassInfo *classInfo):
    Core(eng), m_classInfo(classInfo)
{
    newFunction(&publicPrototype, new QScript::C2Function(&method_void, 0, m_classInfo)); // public prototype
}

Function::~Function()
{
}

void Function::initialize()
{
    QScriptEnginePrivate *eng_p = QScriptEnginePrivate::get(engine());

    eng_p->newConstructor(&ctor, this, publicPrototype);

    const QScriptValue::PropertyFlags flags = QScriptValue::SkipInEnumeration;
    publicPrototype.setProperty(QLatin1String("toString"),
                                eng_p->createFunction(method_toString, 1, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("apply"),
                                eng_p->createFunction(method_apply, 1, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("call"),
                                eng_p->createFunction(method_call, 1, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("connect"),
                                eng_p->createFunction(method_connect, 1, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("disconnect"),
                                eng_p->createFunction(method_disconnect, 1, m_classInfo), flags);
}

void Function::execute(QScriptContext *context)
{
    QScriptEngine *eng = context->engine();
    QScriptEnginePrivate *driver = QScriptEnginePrivate::get(eng);

    QString contents = buildFunction(context);
    driver->evaluate(context, contents);
}

QString Function::buildFunction(QScriptContext *context)
{
    int argc = context->argumentCount();

    QString code;
    code += QLatin1String("return function(");

    // the formals
    for (int i = 0; i < argc - 1; ++i) {
        if (i != 0)
            code += QLatin1String(",");

        code += context->argument(i).toString();
    }

    code += QLatin1String("){");

    // the function body
    if (argc != 0)
        code += context->argument(argc - 1).toString();

    code += QLatin1String("\n}");

    return code;
}

void Function::newFunction(QScriptValue *result, QScriptFunction *foo)
{
    QScriptEnginePrivate::get(engine())->newFunction(result, foo);
}

QScriptValue Function::method_toString(QScriptEngine *eng, QScriptClassInfo *)
{
    QScriptContext *context = eng->currentContext();
    if (QScriptFunction *foo = context->thisObject().impl()->toFunction()) {
        QString code = foo->toString(context);
        return eng->scriptValue(code);
    }

    return context->throwError(QScriptContext::TypeError,
                               QLatin1String("Function.prototype.toString"));
}

QScriptValue Function::method_call(QScriptEngine *eng, QScriptClassInfo *)
{
    QScriptContext *context = eng->currentContext();
    if (! context->thisObject().isFunction()) {
        return context->throwError(QScriptContext::TypeError,
                                   QLatin1String("Function.prototype.call"));
    }

    QScriptValue thisObject = context->argument(0).toObject();
    if (! (thisObject.isValid () && thisObject.isObject()))
        thisObject = eng->globalObject();

    QScriptValueList args;
    for (int i = 1; i < context->argumentCount(); ++i)
        args << context->argument(i);

    return context->thisObject().call(thisObject, args);
}

QScriptValue Function::method_apply(QScriptEngine *eng, QScriptClassInfo *)
{
    QScriptContext *context = eng->currentContext();
    QScriptEnginePrivate *eng_p = QScriptEnginePrivate::get(eng);

    if (! context->thisObject().isFunction()) {
        return context->throwError(QScriptContext::TypeError,
                                   QLatin1String("Function.prototype.apply"));
    }

    QScriptValue thisObject = context->argument(0).toObject();
    if (! (thisObject.isValid () && thisObject.isObject()))
        thisObject = eng_p->globalObject;

    QScriptValueList args;
    QScriptValue undefined = eng->undefinedScriptValue();

    QScriptValue arg = context->argument(1);

    if (Ecma::Array::Instance *arr = eng_p->arrayConstructor->get(arg)) {
        QScript::Array actuals = arr->value;

        for (quint32 i = 0; i < actuals.count(); ++i) {
            QScriptValue a = actuals.at(i);
            if (! a.isValid())
                args << undefined;
            else
                args << a;
        }
    } else if (arg.impl()->classInfo() == eng_p->m_class_arguments) {
        QScript::ArgumentsObjectData *arguments = static_cast<QScript::ArgumentsObjectData*> (arg.impl()->objectData().data());
        QScriptObject *activation = arguments->activation.impl()->objectValue();
        for (uint i = 0; i < arguments->length; ++i)
            args << activation->m_objects[i];
    } else if (!(arg.isUndefined() || arg.isNull())) {
        return context->throwError(QScriptContext::TypeError,
                                   QLatin1String("Function.prototype.apply: second argument is not an array"));
    }

    return context->thisObject().call(thisObject, args);
}

QScriptValue Function::method_void(QScriptEngine *eng, QScriptClassInfo *)
{
    return eng->undefinedScriptValue();
}

QScriptValue Function::method_disconnect(QScriptEngine *eng, QScriptClassInfo *)
{
    QScriptContext *context = eng->currentContext();
#ifndef QT_NO_QOBJECT
    if (context->argumentCount() == 0)
        return context->engine()->scriptValue(false);

    QScriptFunction *fun = context->thisObject().impl()->toFunction();
    if ((fun == 0) || (fun->type() != QScriptFunction::Qt))
        return context->throwError(QScriptContext::TypeError,
                                   QLatin1String("Function.prototype.disconnect"));

    QScriptFunction *otherFun = context->argument(0).impl()->toFunction();
    if (otherFun == 0)
        return context->throwError(QScriptContext::TypeError,
                                   QLatin1String("Function.prototype.disconnect"));

    QtFunction *qtSignal = static_cast<QtFunction*>(fun);

    QObject *thisQObject = context->thisObject().toQObject();
    if (!thisQObject)
        thisQObject = qtSignal->object(); /// ### TypeError

    QMetaMethod sig = qtSignal->metaObject()->method(qtSignal->initialIndex());
    if (sig.methodType() != QMetaMethod::Signal) {
        return context->throwError(QScriptContext::TypeError, 
            QString::fromUtf8("Function.prototype.disconnect: %0::%1 is not a signal")
            .arg(QLatin1String(qtSignal->metaObject()->className()))
            .arg(QLatin1String(sig.signature())));
    }

    bool ok = false;
    if (otherFun->type() == QScriptFunction::Qt) {
        QtFunction *qtSlot = static_cast<QtFunction*>(otherFun);
        ok = QMetaObject::disconnect(thisQObject, qtSignal->initialIndex(),
                                     qtSlot->object(), qtSlot->initialIndex());
    } else {
        ok = qtSignal->destroyConnection(context->thisObject(), context->argument(0));
    }
    return context->engine()->scriptValue(ok);
#else
    return context->throwError(QScriptContext::TypeError,
                               QLatin1String("Function.prototype.disconnect"));
#endif // QT_NO_QOBJECT
}

QScriptValue Function::method_connect(QScriptEngine *eng, QScriptClassInfo *classInfo)
{
    QScriptContext *context = eng->currentContext();
    Q_UNUSED(classInfo);

#ifndef QT_NO_QOBJECT
    if (context->argumentCount() == 0)
        return context->engine()->scriptValue(false);

    QScriptFunction *fun = context->thisObject().impl()->toFunction();
    if ((fun == 0) || (fun->type() != QScriptFunction::Qt))
        return context->throwError(QScriptContext::TypeError,
                                   QLatin1String("Function.prototype.connect"));

    QScriptFunction *otherFun = context->argument(0).impl()->toFunction();
    if (otherFun == 0)
        return context->throwError(QScriptContext::TypeError,
                                   QLatin1String("Function.prototype.connect"));

    QtFunction *qtSignal = static_cast<QtFunction*>(fun);

    QObject *thisQObject = context->thisObject().toQObject();
    if (!thisQObject)
        thisQObject = qtSignal->object(); /// ### TypeError

    QMetaMethod sig = qtSignal->metaObject()->method(qtSignal->initialIndex());
    if (sig.methodType() != QMetaMethod::Signal) {
        return context->throwError(QScriptContext::TypeError, 
            QString::fromUtf8("Function.prototype.connect: %0::%1 is not a signal")
            .arg(QLatin1String(qtSignal->metaObject()->className()))
            .arg(QLatin1String(sig.signature())));
    }

    bool ok = false;
    if (otherFun->type() == QScriptFunction::Qt) {
        QtFunction *qtSlot = static_cast<QtFunction*>(otherFun);
        // ### find the best match using QMetaObject::checkConnectArgs
        // ### check if signal or slot overloaded
        ok = QMetaObject::connect(thisQObject, qtSignal->initialIndex(),
                                  qtSlot->object(), qtSlot->initialIndex());
    } else {
        ok = qtSignal->createConnection(context->thisObject(), context->argument(0));
    }
    return context->engine()->scriptValue(ok);
#else
    Q_UNUSED(eng);
    Q_UNUSED(classInfo);
    return context->throwError(QScriptContext::TypeError,
                               QLatin1String("Function.prototype.connect"));
#endif // QT_NO_QOBJECT
}

} } // namespace QScript::Ecma
