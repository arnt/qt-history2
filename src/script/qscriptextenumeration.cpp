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

#include "qscriptarray_p.h"
#include "qscriptcontext_p.h"
#include "qscriptengine.h"
#include "qscriptengine_p.h"
#include "qscriptextenumeration_p.h"
#include "qscriptecmaobject_p.h"
#include "qscriptecmafunction_p.h"
#include "qscriptvalue_p.h"

#include <QtCore/QtDebug>

namespace QScript { namespace Ext {

EnumerationClassData::EnumerationClassData(QScriptClassInfo *classInfo):
    m_classInfo(classInfo)
{
}

EnumerationClassData::~EnumerationClassData()
{
}

void EnumerationClassData::mark(const QScriptValue &object, int generation)
{
    Q_ASSERT(object.isValid());

    QScriptEnginePrivate *eng_p = QScriptEnginePrivate::get(object.engine());

    if (Enumeration::Instance *instance = Enumeration::Instance::get(object, classInfo())) {
        eng_p->markObject(instance->object, generation);
        eng_p->markObject(instance->value, generation);
    }
}


Enumeration::Enumeration(QScriptEngine *eng):
    Ecma::Core(eng)
{
    QScriptEnginePrivate *eng_p = QScriptEnginePrivate::get(eng);
    m_classInfo = eng_p->registerClass(QLatin1String("Enumeration"));
    QExplicitlySharedDataPointer<QScriptClassData> data(new EnumerationClassData(m_classInfo));
    m_classInfo->setData(data);

    publicPrototype.invalidate();
    newEnumeration(&publicPrototype, eng->newArray());

    eng_p->newConstructor(&ctor, this, publicPrototype);

    const QScriptValue::PropertyFlags flags = QScriptValue::SkipInEnumeration;
    publicPrototype.setProperty(QLatin1String("toFirst"),
                                eng_p->createFunction(method_toFirst, 0, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("hasNext"),
                                eng_p->createFunction(method_hasNext, 0, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("next"),
                                eng_p->createFunction(method_next, 0, m_classInfo), flags);
}

Enumeration::~Enumeration()
{
}

Enumeration::Instance *Enumeration::Instance::get(const QScriptValue &object, QScriptClassInfo *klass)
{
    if (! klass || klass == QScriptValueImpl::get(object)->classInfo())
        return static_cast<Instance*> (QScriptValueImpl::get(object)->objectData().data());
    
    return 0;
}

void Enumeration::execute(QScriptContext *context)
{
    if (context->argumentCount() > 0) {
        newEnumeration(&QScriptContextPrivate::get(context)->result, context->argument(0));
    } else {
        context->throwError(QScriptContext::TypeError,
                            QLatin1String("Enumeration.execute"));
    }
}

void Enumeration::newEnumeration(QScriptValue *result, const QScriptValue &object)
{
    Instance *instance = new Instance();
    instance->object = object;
    instance->value = object;
    instance->index = -1;
    instance->toFirst();

    QScriptEnginePrivate::get(engine())->newObject(result, publicPrototype, classInfo());
    QScriptValueImpl::get(*result)->setObjectData(QExplicitlySharedDataPointer<QScriptObjectData>(instance));
}

QScriptValue Enumeration::method_toFirst(QScriptEngine *eng, QScriptClassInfo *classInfo)
{
    QScriptContext *context = eng->currentContext();
    if (Instance *instance = Instance::get(context->thisObject(), classInfo)) {
        instance->toFirst();
        return eng->undefinedValue();
    } else {
        return context->throwError(QScriptContext::TypeError,
                                   QLatin1String("Enumeration.toFirst"));
    }
}

QScriptValue Enumeration::method_hasNext(QScriptEngine *eng, QScriptClassInfo *classInfo)
{
    QScriptContext *context = eng->currentContext();
    Instance *instance = Instance::get(context->thisObject(), classInfo);
    if (! instance || ! instance->value.isObject())
        return context->throwError(QScriptContext::TypeError,
                                   QLatin1String("Enumeration.hasNext"));

    QScriptValue v;
    instance->hasNext(context, &v);
    return v;
}

QScriptValue Enumeration::method_next(QScriptEngine *eng, QScriptClassInfo *classInfo)
{
    QScriptContext *context = eng->currentContext();
    Instance *instance = Instance::get(context->thisObject(), classInfo);
    if (! instance || ! instance->value.isObject())
        return context->throwError(QScriptContext::TypeError,
                                   QLatin1String("Enumeration.next"));

    QScriptValue v;
    instance->next(context, &v);
    return v;
}

void Enumeration::Instance::toFirst()
{
    value = object;
    index = -1;
}

void Enumeration::Instance::hasNext(QScriptContext *context, QScriptValue *result)
{
    QScriptEngine *eng = context->engine();
Lagain:
    int count = QScriptValueImpl::get(value)->memberCount();
    bool found = false;
    while (! found && ++index < count) {
        QScript::Member member;
        QScriptValueImpl::get(value)->member(index, &member);
        found = member.isValid() && ! member.dontEnum();
        if (found) {
            QScriptValue current;
            QScriptValueImpl::get(value)->get(member, &current);
            found = current.isValid();

            if (found && member.nameId()) {
                Member m;
                QScriptValue b;
                if (QScriptValueImpl::get(object)->resolve(member.nameId(), &m, &b, QScriptValue::ResolvePrototype))
                    found = (QScriptValueImpl::get(b)->objectValue() == QScriptValueImpl::get(value)->objectValue());
            }
        }
    }

    if (! found && value.prototype().isValid() && value.prototype().isObject()) {
        value = value.prototype();
        index = -1;
        goto Lagain;
    }

    *result = eng->scriptValue(found);
}

void Enumeration::Instance::next(QScriptContext *context, QScriptValue *result)
{
    QScriptEngine *eng = context->engine();
    QScriptEnginePrivate *eng_p = QScriptEnginePrivate::get(eng);

    QScript::Member member;
    QScriptValueImpl::get(value)->member(index, &member);

    if (member.isObjectProperty() || member.nameId())
        eng_p->newNameId(result, member.nameId());

    else if (member.isNativeProperty() && ! member.nameId())
        eng_p->newNumber(result, member.id());

    else
        eng_p->newUndefined(result);
}

} } // namespace QScript::Ext
