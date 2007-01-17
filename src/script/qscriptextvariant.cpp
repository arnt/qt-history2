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
#include "qscriptextvariant_p.h"
#include "qscriptecmaobject_p.h"
#include "qscriptecmafunction_p.h"
#include "qscriptecmaregexp_p.h"
#include "qscriptvalue_p.h"

#include <QtCore/QtDebug>

#ifndef QT_NO_GEOM_VARIANT
#include <QtCore/QPoint>
#include <QtCore/QRect>
#include <QtCore/QSize>
#endif
#include <QtCore/QStringList>

#include <limits.h>

namespace QScript { namespace Ext {

Variant::Variant(QScriptEngine *eng, QScriptClassInfo *classInfo):
    Ecma::Core(eng), m_classInfo(classInfo)
{
    QScriptEnginePrivate *eng_p = QScriptEnginePrivate::get(eng);

    publicPrototype.invalidate();
    newVariant(&publicPrototype, QVariant());

    eng_p->newConstructor(&ctor, this, publicPrototype);

    const QScriptValue::PropertyFlags flags = QScriptValue::SkipInEnumeration;
    publicPrototype.setProperty(QLatin1String("toString"),
                eng_p->createFunction(method_toString, 0, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("valueOf"),
                eng_p->createFunction(method_valueOf, 0, m_classInfo), flags);
}

Variant::~Variant()
{
}

Variant::Instance *Variant::Instance::get(const QScriptValue &object, QScriptClassInfo *klass)
{
    if (! klass || klass == QScriptValueImpl::get(object)->classInfo())
        return static_cast<Instance*> (QScriptValueImpl::get(object)->objectData().data());

    return 0;
}

void Variant::execute(QScriptContext *context)
{
    QScriptValue tmp;
    newVariant(&tmp, QVariant());
    context->setReturnValue(tmp);
}

void Variant::newVariant(QScriptValue *result, const QVariant &value)
{
    Instance *instance = new Instance();
    instance->value = value;

    QScriptEnginePrivate::get(engine())->newObject(result, publicPrototype, classInfo());
    QScriptValueImpl::get(*result)->setObjectData(QExplicitlySharedDataPointer<QScriptObjectData>(instance));
}

QScriptValue Variant::method_toString(QScriptEngine *eng, QScriptClassInfo *classInfo)
{
    QScriptContext *context = eng->currentContext();
    if (Instance *instance = Instance::get(context->thisObject(), classInfo)) {
        QScriptValue value = method_valueOf(eng, classInfo);
        QString valueStr = value.isObject() ? QString::fromUtf8("...") : value.toString();
        QString str = QString::fromUtf8("variant(%0, %1)")
                      .arg(QLatin1String(instance->value.typeName()))
                      .arg(valueStr);
        return eng->scriptValue(str);
    }
    return context->throwError(QScriptContext::TypeError,
                               QLatin1String("Variant.prototype.toString"));
}

QScriptValue Variant::method_valueOf(QScriptEngine *eng, QScriptClassInfo *classInfo)
{
    QScriptContext *context = eng->currentContext();
    if (Instance *instance = Instance::get(context->thisObject(), classInfo)) {
        QVariant v = instance->value;
        switch (v.type ()) {
        case QVariant::String:
            return (eng->scriptValue(v.toString()));

        case QVariant::Int:
            return (eng->scriptValue(v.toInt()));

        case QVariant::Bool:
            return (eng->scriptValue(v.toBool()));

        case QVariant::Double:
            return (eng->scriptValue(v.toDouble())); // ### hmmm

        case QVariant::Char:
            return (eng->scriptValue(v.toChar().unicode()));

        case QVariant::UInt:
            return (eng->scriptValue(v.toUInt()));

        case QVariant::LongLong:
            return eng->scriptValue(qnumber(v.toLongLong())); // ###


        case QVariant::ULongLong:
#if defined(Q_OS_WIN) && _MSC_FULL_VER <= 12008804
#pragma message("** NOTE: You need the Visual Studio Processor Pack to compile support for 64bit unsigned integers.")
            return eng->scriptValue(qnumber(v.toLongLong()));
#else
            return eng->scriptValue(qnumber(v.toULongLong()));
#endif

        default:
            return context->thisObject();
        } // switch
    }

    return context->throwError(QScriptContext::TypeError,
                               QLatin1String("Variant.prototype.valueOf"));
}

} } // namespace QScript::Ecma
