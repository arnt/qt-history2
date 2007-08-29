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

#include "qscriptecmaobject_p.h"

#ifndef QT_NO_SCRIPT

#include "qscriptengine_p.h"
#include "qscriptvalueimpl_p.h"
#include "qscriptcontext_p.h"
#include "qscriptmember_p.h"
#include "qscriptobject_p.h"

#include <QtCore/QtDebug>

namespace QScript { namespace Ecma {

Object::Object(QScriptEnginePrivate *eng, QScriptClassInfo *classInfo):
    Core(eng, classInfo)
{
    newObject(&publicPrototype, eng->nullValue());
}

Object::~Object()
{
}

void Object::initialize()
{
    QScriptEnginePrivate *eng = engine();

    eng->newConstructor(&ctor, this, publicPrototype);

    addPrototypeFunction(QLatin1String("toString"), method_toString, 1);
    addPrototypeFunction(QLatin1String("toLocaleString"), method_toLocaleString, 1);
    addPrototypeFunction(QLatin1String("valueOf"), method_valueOf, 0);
    addPrototypeFunction(QLatin1String("hasOwnProperty"), method_hasOwnProperty, 1);
    addPrototypeFunction(QLatin1String("isPrototypeOf"), method_isPrototypeOf, 1);
    addPrototypeFunction(QLatin1String("propertyIsEnumerable"), method_propertyIsEnumerable, 1);
    addPrototypeFunction(QLatin1String("__defineGetter__"), method_defineGetter, 2);
    addPrototypeFunction(QLatin1String("__defineSetter__"), method_defineSetter, 2);
}

void Object::execute(QScriptContextPrivate *context)
{
#ifndef Q_SCRIPT_NO_EVENT_NOTIFY
    engine()->notifyFunctionEntry(context);
#endif
    QScriptValueImpl value;

    if (context->argumentCount() > 0)
        value = context->argument(0).toObject();
    else
        value.invalidate();

    if (! value.isValid())
        newObject(&value);

    context->setReturnValue(value);
#ifndef Q_SCRIPT_NO_EVENT_NOTIFY
    engine()->notifyFunctionExit(context);
#endif
}

void Object::newObject(QScriptValueImpl *result, const QScriptValueImpl &proto)
{
    engine()->newObject(result, proto, classInfo());
}

QScriptValueImpl Object::method_toString(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *)
{
    QScriptValueImpl glo = eng->globalObject();
    QString s = QLatin1String("[object ");
    QScriptValueImpl self = context->thisObject();
    if (self.objectValue() == glo.objectValue())
        s += QLatin1String("global");
    else
        s += self.classInfo()->name();
    s += QLatin1String("]");
    return (QScriptValueImpl(eng, s));
}

QScriptValueImpl Object::method_toLocaleString(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *classInfo)
{
    return method_toString(context, eng, classInfo);
}

QScriptValueImpl Object::method_valueOf(QScriptContextPrivate *context, QScriptEnginePrivate *, QScriptClassInfo *)
{
    return (context->thisObject());
}

QScriptValueImpl Object::method_hasOwnProperty(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *)
{
    bool result = false;

    if (context->thisObject().isObject() && (context->argumentCount() > 0)) {
        QScriptValueImpl arg = context->argument(0);

        QScriptNameIdImpl *id = 0;
        if (arg.isString())
            id = arg.stringValue();

        if (! id || ! id->unique) {
            QString str = arg.toString();
            id = eng->nameId(str);
        }

        QScript::Member member;
        QScriptValueImpl base;
        QScriptValueImpl self = context->thisObject();
        if (self.resolve(id, &member, &base, QScriptValue::ResolveLocal))
            result = true;
    }

    return (QScriptValueImpl(eng, result));
}

QScriptValueImpl Object::method_isPrototypeOf(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *)
{
    bool result = false;

    if (context->thisObject().isObject() && (context->argumentCount() > 0)) {
        QScriptValueImpl arg = context->argument(0);

        if (arg.isObject()) {
            QScriptValueImpl proto = arg.prototype();

            if (proto.isObject()) {
                QScriptValueImpl self = context->thisObject();
                result = self.objectValue() == proto.objectValue();
            }
        }
    }

    return (QScriptValueImpl(eng, result));
}

QScriptValueImpl Object::method_propertyIsEnumerable(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *)
{
    bool result = false;

    if (context->thisObject().isObject() && (context->argumentCount() > 0)) {
        QScriptValueImpl arg = context->argument(0);

        QScriptNameIdImpl *id = 0;
        if (arg.isString())
            id = arg.stringValue();

        if (! id || ! id->unique) {
            QString str = arg.toString();
            id = eng->nameId(str);
        }

        QScript::Member member;
        QScriptValueImpl base;
        QScriptValueImpl self = context->thisObject();
        if (self.resolve(id, &member, &base, QScriptValue::ResolveLocal)) {
            result = ! member.dontEnum();
            if (result) {
                QScriptValueImpl tmp;
                base.get(member, &tmp);
                result = tmp.isValid();
            }
        }
    }

    return (QScriptValueImpl(eng, result));
}

QScriptValueImpl Object::method_defineGetter(QScriptContextPrivate *context, QScriptEnginePrivate *eng,
                                             QScriptClassInfo *)
{
    QString propertyName = context->argument(0).toString();
    QScriptValueImpl getter = context->argument(1);
    if (!getter.isFunction())
        return context->throwError(QLatin1String("getter must be a function"));
    context->thisObject().setProperty(propertyName, getter, QScriptValue::PropertyGetter);
    return eng->undefinedValue();
}

QScriptValueImpl Object::method_defineSetter(QScriptContextPrivate *context, QScriptEnginePrivate *eng,
                                             QScriptClassInfo *)
{
    QString propertyName = context->argument(0).toString();
    QScriptValueImpl setter = context->argument(1);
    if (!setter.isFunction())
        return context->throwError(QLatin1String("setter must be a function"));
    context->thisObject().setProperty(propertyName, setter, QScriptValue::PropertySetter);
    return eng->undefinedValue();
}

} } // namespace QScript::Ecma

#endif // QT_NO_SCRIPT
