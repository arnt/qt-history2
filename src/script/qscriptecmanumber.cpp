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

#include "qscriptecmanumber_p.h"

#ifndef QT_NO_SCRIPT

#include "qscriptengine_p.h"
#include "qscriptvalueimpl_p.h"
#include "qscriptcontext_p.h"
#include "qscriptmember_p.h"
#include "qscriptobject_p.h"

#include <QtCore/QtDebug>
#include <QtCore/qnumeric.h>

QT_BEGIN_NAMESPACE

namespace QScript { namespace Ecma {

Number::Number(QScriptEnginePrivate *eng):
    Core(eng, QLatin1String("Number"))
{
    newNumber(&publicPrototype, 0);

    eng->newConstructor(&ctor, this, publicPrototype);

    addPrototypeFunction(QLatin1String("toString"), method_toString, 0);
    addPrototypeFunction(QLatin1String("toLocaleString"), method_toLocaleString, 0);
    addPrototypeFunction(QLatin1String("valueOf"), method_valueOf, 0);
    addPrototypeFunction(QLatin1String("toFixed"), method_toFixed, 0);
    addPrototypeFunction(QLatin1String("toExponential"), method_toExponential, 0);
    addPrototypeFunction(QLatin1String("toPrecision"), method_toPrecision, 0);

    QScriptValue::PropertyFlags flags = QScriptValue::Undeletable
                                        | QScriptValue::ReadOnly
                                        | QScriptValue::SkipInEnumeration;
    ctor.setProperty(QLatin1String("NaN"),
                     QScriptValueImpl(eng, qSNaN()), flags);
    ctor.setProperty(QLatin1String("NEGATIVE_INFINITY"),
                     QScriptValueImpl(eng, -qInf()), flags);
    ctor.setProperty(QLatin1String("POSITIVE_INFINITY"),
                     QScriptValueImpl(eng, qInf()), flags);
    ctor.setProperty(QLatin1String("MAX_VALUE"),
                     QScriptValueImpl(eng, 1.7976931348623158e+308), flags);
#ifdef __INTEL_COMPILER
# pragma warning( push )
# pragma warning(disable: 239)
#endif
    ctor.setProperty(QLatin1String("MIN_VALUE"),
                     QScriptValueImpl(eng, 5e-324), flags);
#ifdef __INTEL_COMPILER
# pragma warning( pop )
#endif
}

Number::~Number()
{
}

void Number::execute(QScriptContextPrivate *context)
{
#ifndef Q_SCRIPT_NO_EVENT_NOTIFY
    engine()->notifyFunctionEntry(context);
#endif
    qsreal value;
    if (context->argumentCount() > 0)
        value = context->argument(0).toNumber();
    else
        value = 0;

    QScriptValueImpl num(engine(), value);
    if (!context->isCalledAsConstructor()) {
        context->setReturnValue(num);
    } else {
        QScriptValueImpl &obj = context->m_thisObject;
        obj.setClassInfo(classInfo());
        obj.setInternalValue(num);
        obj.setPrototype(publicPrototype);
        context->setReturnValue(obj);
    }
#ifndef Q_SCRIPT_NO_EVENT_NOTIFY
    engine()->notifyFunctionExit(context);
#endif
}

void Number::newNumber(QScriptValueImpl *result, qsreal value)
{
    engine()->newObject(result, publicPrototype, classInfo());
    result->setInternalValue(QScriptValueImpl(engine(), value));
}

QScriptValueImpl Number::method_toString(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *classInfo)
{
    QScriptValueImpl self = context->thisObject();
    if (self.classInfo() != classInfo)
        return context->throwError(QScriptContext::TypeError,
                                   QLatin1String("Number.prototype.toString"));

    QString str = self.internalValue().toString();
    return (QScriptValueImpl(eng, str));
}

QScriptValueImpl Number::method_toLocaleString(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *classInfo)
{
    QScriptValueImpl self = context->thisObject();
    if (self.classInfo() != classInfo)
        return context->throwError(QScriptContext::TypeError,
                                   QLatin1String("Number.prototype.toLocaleString"));

    QString str = self.internalValue().toString();
    return (QScriptValueImpl(eng, str));
}

QScriptValueImpl Number::method_valueOf(QScriptContextPrivate *context, QScriptEnginePrivate *, QScriptClassInfo *classInfo)
{
    QScriptValueImpl self = context->thisObject();
    if (self.classInfo() != classInfo)
        return context->throwError(QScriptContext::TypeError,
                                   QLatin1String("Number.prototype.valueOf"));

    return (self.internalValue());
}

QScriptValueImpl Number::method_toFixed(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *classInfo)
{
    QScriptValueImpl self = context->thisObject();
    if (self.classInfo() != classInfo)
        return context->throwError(QScriptContext::TypeError,
                                   QLatin1String("Number.prototype.toFixed"));

    qsreal fdigits = 0;

    if (context->argumentCount() > 0)
        fdigits = context->argument(0).toInteger();

    if (qIsNaN(fdigits))
        fdigits = 0;

    qsreal v = self.internalValue().toNumber();
    return (QScriptValueImpl(eng, QString::number(v, 'f', int (fdigits))));
}

QScriptValueImpl Number::method_toExponential(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *classInfo)
{
    QScriptValueImpl self = context->thisObject();
    if (self.classInfo() != classInfo)
        return context->throwError(QScriptContext::TypeError,
                                   QLatin1String("Number.prototype.toExponential"));

    qsreal fdigits = 0;

    if (context->argumentCount() > 0)
        fdigits = context->argument(0).toInteger();

    qsreal v = self.internalValue().toNumber();
    QString z = QString::number(v, 'e', int (fdigits));
    return (QScriptValueImpl(eng, z));
}

QScriptValueImpl Number::method_toPrecision(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *classInfo)
{
    QScriptValueImpl self = context->thisObject();
    if (self.classInfo() != classInfo)
        return context->throwError(QScriptContext::TypeError,
                                   QLatin1String("Number.prototype.toPrecision"));

    qsreal fdigits = 0;

    if (context->argumentCount() > 0)
        fdigits = context->argument(0).toInteger();

    qsreal v = self.internalValue().toNumber();
    return (QScriptValueImpl(eng, QString::number(v, 'g', int (fdigits))));
}

} } // namespace QScript::Ecma

QT_END_NAMESPACE

#endif // QT_NO_SCRIPT
