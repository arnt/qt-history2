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

Variant::Variant(QScriptEnginePrivate *eng, QScriptClassInfo *classInfo):
    Ecma::Core(eng), m_classInfo(classInfo)
{
    publicPrototype.invalidate();
    newVariant(&publicPrototype, QVariant());

    eng->newConstructor(&ctor, this, publicPrototype);

    const QScriptValue::PropertyFlags flags = QScriptValue::SkipInEnumeration;
    publicPrototype.setProperty(QLatin1String("toString"),
                eng->createFunction(method_toString, 0, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("valueOf"),
                eng->createFunction(method_valueOf, 0, m_classInfo), flags);
}

Variant::~Variant()
{
}

Variant::Instance *Variant::Instance::get(const QScriptValueImpl &object, QScriptClassInfo *klass)
{
    if (! klass || klass == object.classInfo())
        return static_cast<Instance*> (object.objectData().data());

    return 0;
}

void Variant::execute(QScriptContextPrivate *context)
{
    QScriptValueImpl tmp;
    newVariant(&tmp, QVariant());
    context->setReturnValue(tmp);
}

void Variant::newVariant(QScriptValueImpl *result, const QVariant &value)
{
    Instance *instance = new Instance();
    instance->value = value;

    engine()->newObject(result, publicPrototype, classInfo());
    result->setObjectData(QExplicitlySharedDataPointer<QScriptObjectData>(instance));
}

QScriptValueImpl Variant::method_toString(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *classInfo)
{
    if (Instance *instance = Instance::get(context->thisObject(), classInfo)) {
        QScriptValueImpl value = method_valueOf(context, eng, classInfo);
        QString valueStr = value.isObject() ? QString::fromUtf8("...") : value.toString();
        QString str = QString::fromUtf8("variant(%0, %1)")
                      .arg(QLatin1String(instance->value.typeName()))
                      .arg(valueStr);
        return QScriptValueImpl(eng, str);
    }
    return context->throwError(QScriptContext::TypeError,
                               QLatin1String("Variant.prototype.toString"));
}

QScriptValueImpl Variant::method_valueOf(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *classInfo)
{
    if (Instance *instance = Instance::get(context->thisObject(), classInfo)) {
        QVariant v = instance->value;
        switch (v.type ()) {
        case QVariant::String:
            return (QScriptValueImpl(eng, v.toString()));

        case QVariant::Int:
            return (QScriptValueImpl(eng, v.toInt()));

        case QVariant::Bool:
            return (QScriptValueImpl(eng, v.toBool()));

        case QVariant::Double:
            return (QScriptValueImpl(eng, v.toDouble())); // ### hmmm

        case QVariant::Char:
            return (QScriptValueImpl(eng, v.toChar().unicode()));

        case QVariant::UInt:
            return (QScriptValueImpl(eng, v.toUInt()));

        case QVariant::LongLong:
            return QScriptValueImpl(eng, qsreal(v.toLongLong())); // ###


        case QVariant::ULongLong:
#if defined(Q_OS_WIN) && _MSC_FULL_VER <= 12008804
#pragma message("** NOTE: You need the Visual Studio Processor Pack to compile support for 64bit unsigned integers.")
            return QScriptValueImpl(eng, qsreal(v.toLongLong()));
#else
            return QScriptValueImpl(eng, qsreal(v.toULongLong()));
#endif

        default:
            return context->thisObject();
        } // switch
    }

    return context->throwError(QScriptContext::TypeError,
                               QLatin1String("Variant.prototype.valueOf"));
}

} } // namespace QScript::Ecma
