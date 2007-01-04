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
#include "qscriptecmanumber_p.h"
#include "qscriptecmastring_p.h"
#include "qscriptecmaobject_p.h"
#include "qscriptecmafunction_p.h"

#include <QtCore/QtDebug>
#include <QtCore/qnumeric.h>

namespace QScript { namespace Ecma {

Number::Number(QScriptEngine *eng):
    Core(eng)
{
    QScriptEnginePrivate *eng_p = QScriptEnginePrivate::get(eng);

    m_classInfo = eng_p->registerClass(QLatin1String("Number"));

    publicPrototype.invalidate();
    newNumber(&publicPrototype, 0);

    eng_p->newConstructor(&ctor, this, publicPrototype);

    QScriptValue::PropertyFlags flags = QScriptValue::SkipInEnumeration;

    publicPrototype.setProperty(QLatin1String("toString"),
                                eng_p->createFunction(method_toString, 0, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("toLocaleString"),
                                eng_p->createFunction(method_toLocaleString, 0, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("valueOf"),
                                eng_p->createFunction(method_valueOf, 0, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("toFixed"),
                                eng_p->createFunction(method_toFixed, 0, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("toExponential"),
                                eng_p->createFunction(method_toExponential, 0, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("toPrecision"),
                                eng_p->createFunction(method_toPrecision, 0, m_classInfo), flags);

    flags = QScriptValue::Undeletable
            | QScriptValue::ReadOnly
            | QScriptValue::SkipInEnumeration;

    ctor.setProperty(QLatin1String("NaN"),
                     eng->scriptValue(qSNan()), flags);
    ctor.setProperty(QLatin1String("NEGATIVE_INFINITY"),
                     eng->scriptValue(-qInf()), flags);
    ctor.setProperty(QLatin1String("POSITIVE_INFINITY"),
                     eng->scriptValue(qInf()), flags);
    ctor.setProperty(QLatin1String("MAX_VALUE"),
                     eng->scriptValue(1.7976931348623158e+308), flags);
    ctor.setProperty(QLatin1String("MIN_VALUE"),
                     eng->scriptValue(2.2250738585072014e-308), flags);
}

Number::~Number()
{
}

void Number::execute(QScriptContext *context)
{
    qnumber value;
    if (context->argumentCount() > 0)
        value = context->argument(0).toNumber();
    else
        value = 0;

    QScriptValue num = engine()->scriptValue(value);
    if (!context->calledAsConstructor()) {
        context->setReturnValue(num);
    } else {
        QScriptValue &obj = QScriptContextPrivate::get(context)->thisObject;
        obj.impl()->setClassInfo(classInfo());
        obj.impl()->setInternalValue(num);
        obj.setPrototype(publicPrototype);
    }
}

void Number::newNumber(QScriptValue *result, qnumber value)
{
    QScriptEnginePrivate::get(engine())->newObject(result, publicPrototype, classInfo());
    result->impl()->setInternalValue(engine()->scriptValue(value));
}

QScriptValue Number::method_toString(QScriptEngine *eng, QScriptClassInfo *classInfo)
{
    QScriptContext *context = eng->currentContext();
    if (context->thisObject().impl()->classInfo() != classInfo)
        return context->throwError(QScriptContext::TypeError,
                                   QLatin1String("Number.prototype.toString"));

    QString str = context->thisObject().impl()->internalValue().toString();
    return (eng->scriptValue(str));
}

QScriptValue Number::method_toLocaleString(QScriptEngine *eng, QScriptClassInfo *classInfo)
{
    QScriptContext *context = eng->currentContext();
    if (context->thisObject().impl()->classInfo() != classInfo)
        return context->throwError(QScriptContext::TypeError,
                                   QLatin1String("Number.prototype.toLocaleString"));

    QString str = context->thisObject().impl()->internalValue().toString();
    return (eng->scriptValue(str));
}

QScriptValue Number::method_valueOf(QScriptEngine *eng, QScriptClassInfo *classInfo)
{
    QScriptContext *context = eng->currentContext();
    if (context->thisObject().impl()->classInfo() != classInfo)
        return context->throwError(QScriptContext::TypeError,
                                   QLatin1String("Number.prototype.valueOf"));

    return (context->thisObject().impl()->internalValue());
}

QScriptValue Number::method_toFixed(QScriptEngine *eng, QScriptClassInfo *classInfo)
{
    QScriptContext *context = eng->currentContext();
    if (context->thisObject().impl()->classInfo() != classInfo)
        return context->throwError(QScriptContext::TypeError,
                                   QLatin1String("Number.prototype.toFixed"));

    qnumber fdigits = 0;

    if (context->argumentCount() > 0)
        fdigits = context->argument(0).toInteger();

    if (qIsNan(fdigits))
        fdigits = 0;

    qnumber v = context->thisObject().impl()->internalValue().toNumber();
    return (eng->scriptValue(QString::number(v, 'f', int (fdigits))));
}

QScriptValue Number::method_toExponential(QScriptEngine *eng, QScriptClassInfo *classInfo)
{
    QScriptContext *context = eng->currentContext();
    if (context->thisObject().impl()->classInfo() != classInfo)
        return context->throwError(QScriptContext::TypeError,
                                   QLatin1String("Number.prototype.toExponential"));

    qnumber fdigits = 0;

    if (context->argumentCount() > 0)
        fdigits = context->argument(0).toInteger();

    qnumber v = context->thisObject().impl()->internalValue().toNumber();
    QString z = QString::number(v, 'e', int (fdigits));
    return (eng->scriptValue(z));
}

QScriptValue Number::method_toPrecision(QScriptEngine *eng, QScriptClassInfo *classInfo)
{
    QScriptContext *context = eng->currentContext();
    if (context->thisObject().impl()->classInfo() != classInfo)
        return context->throwError(QScriptContext::TypeError,
                                   QLatin1String("Number.prototype.toPrecision"));

    qnumber fdigits = 0;

    if (context->argumentCount() > 0)
        fdigits = context->argument(0).toInteger();

    qnumber v = context->thisObject().impl()->internalValue().toNumber();
    return (eng->scriptValue(QString::number(v, 'g', int (fdigits))));
}

} } // namespace QScript::Ecma
