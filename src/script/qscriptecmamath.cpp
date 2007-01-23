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

#include "qscriptcontext.h"
#include "qscriptengine.h"
#include "qscriptecmamath_p.h"
#include "qscriptvalue_p.h"

#include <QtCore/QtDebug>
#include <QtCore/qnumeric.h>
#include <QtCore/QSysInfo>
#include <math.h>

namespace QScript { namespace Ecma {

Math::Math(QScriptEngine *engine, QScriptClassInfo *classInfo):
    m_engine(engine),
    m_classInfo(classInfo)
{
}

Math::~Math()
{
}

void Math::construct(QScriptValue *object, QScriptEngine *eng)
{
    QScriptEnginePrivate *eng_p = QScriptEnginePrivate::get(eng);
    QScriptClassInfo *classInfo = eng_p->registerClass(QLatin1String("Math"));

    Math *instance = new Math(eng, classInfo);
    *object = eng_p->createObject(classInfo);
    QScriptValueImpl::get(*object)->setObjectData(QExplicitlySharedDataPointer<QScriptObjectData>(instance));

    QScriptValue::PropertyFlags flags = QScriptValue::Undeletable
                                        | QScriptValue::ReadOnly
                                        | QScriptValue::SkipInEnumeration;

    object->setProperty(QLatin1String("E"),
                        eng->scriptValue(::exp(1.0)), flags);
    object->setProperty(QLatin1String("LN2"),
                        eng->scriptValue(::log(2.0)), flags);
    object->setProperty(QLatin1String("LN10"),
                        eng->scriptValue(::log(10.0)), flags);
    object->setProperty(QLatin1String("LOG2E"),
                        eng->scriptValue(1.0/::log(2.0)), flags);
    object->setProperty(QLatin1String("LOG10E"),
                        eng->scriptValue(1.0/::log(10.0)), flags);
    object->setProperty(QLatin1String("PI"),
                        eng->scriptValue(2.0 * ::asin(1.0)), flags);
    object->setProperty(QLatin1String("SQRT1_2"),
                        eng->scriptValue(::sqrt(0.5)), flags);
    object->setProperty(QLatin1String("SQRT2"),
                        eng->scriptValue(::sqrt(2.0)), flags);

    flags = QScriptValue::SkipInEnumeration;

    object->setProperty(QLatin1String("abs"),
                        eng->newFunction(method_abs, 1), flags);
    object->setProperty(QLatin1String("acos"),
                        eng->newFunction(method_acos, 1), flags);
    object->setProperty(QLatin1String("asin"),
                        eng->newFunction(method_asin, 0), flags);
    object->setProperty(QLatin1String("atan"),
                        eng->newFunction(method_atan, 1), flags);
    object->setProperty(QLatin1String("atan2"),
                        eng->newFunction(method_atan2, 2), flags);
    object->setProperty(QLatin1String("ceil"),
                        eng->newFunction(method_ceil, 1), flags);
    object->setProperty(QLatin1String("cos"),
                        eng->newFunction(method_cos, 1), flags);
    object->setProperty(QLatin1String("exp"),
                        eng->newFunction(method_exp, 1), flags);
    object->setProperty(QLatin1String("floor"),
                        eng->newFunction(method_floor, 1), flags);
    object->setProperty(QLatin1String("log"),
                        eng->newFunction(method_log, 1), flags);
    object->setProperty(QLatin1String("max"),
                        eng->newFunction(method_max, 2), flags);
    object->setProperty(QLatin1String("min"),
                        eng->newFunction(method_min, 2), flags);
    object->setProperty(QLatin1String("pow"),
                        eng->newFunction(method_pow, 2), flags);
    object->setProperty(QLatin1String("random"),
                        eng->newFunction(method_random, 0), flags);
    object->setProperty(QLatin1String("round"),
                        eng->newFunction(method_round, 1), flags);
    object->setProperty(QLatin1String("sin"),
                        eng->newFunction(method_sin, 1), flags);
    object->setProperty(QLatin1String("sqrt"),
                        eng->newFunction(method_sqrt, 1), flags);
    object->setProperty(QLatin1String("tan"),
                        eng->newFunction(method_tan, 1), flags);
}

QScriptValue Math::method_abs(QScriptContext *context,
                              QScriptEngine *eng)
{
    qnumber v = context->argument(0).toNumber();
    if (v == 0) // 0 | -0
        return (eng->scriptValue(0));
    else
        return (eng->scriptValue(v < 0 ? -v : v));
}

QScriptValue Math::method_acos(QScriptContext *context,
                               QScriptEngine *eng)
{
    qnumber v = context->argument(0).toNumber();
    return (eng->scriptValue(::acos(v)));
}

QScriptValue Math::method_asin(QScriptContext *context,
                               QScriptEngine *eng)
{
    qnumber v = context->argument(0).toNumber();
    return (eng->scriptValue(::asin(v)));
}

QScriptValue Math::method_atan(QScriptContext *context,
                               QScriptEngine *eng)
{
    qnumber v = context->argument(0).toNumber();
    return (eng->scriptValue(::atan(v)));
}

QScriptValue Math::method_atan2(QScriptContext *context,
                                QScriptEngine *eng)
{
    qnumber v1 = context->argument(0).toNumber();
    qnumber v2 = context->argument(1).toNumber();
    return (eng->scriptValue(::atan2(v1, v2)));
}

QScriptValue Math::method_ceil(QScriptContext *context,
                               QScriptEngine *eng)
{
    qnumber v = context->argument(0).toNumber();
    return (eng->scriptValue(::ceil(v)));
}

QScriptValue Math::method_cos(QScriptContext *context,
                              QScriptEngine *eng)
{
    qnumber v = context->argument(0).toNumber();
    return (eng->scriptValue(::cos(v)));
}

QScriptValue Math::method_exp(QScriptContext *context,
                              QScriptEngine *eng)
{
    qnumber v = context->argument(0).toNumber();
    return (eng->scriptValue(::exp(v)));
}

QScriptValue Math::method_floor(QScriptContext *context,
                                QScriptEngine *eng)
{
    qnumber v = context->argument(0).toNumber();
    return (eng->scriptValue(::floor(v)));
}

QScriptValue Math::method_log(QScriptContext *context,
                              QScriptEngine *eng)
{
    qnumber v = context->argument(0).toNumber();
    return (eng->scriptValue(::log(v)));
}

QScriptValue Math::method_max(QScriptContext *context,
                              QScriptEngine *eng)
{
    qnumber mx = -qInf();
    for (int i = 0; i < context->argumentCount(); ++i) {
        qnumber x = context->argument(i).toNumber();
        if (x > mx || qIsNan(x))
            mx = x;
    }
    return (eng->scriptValue(mx));
}

/* copies the sign from y to x and returns the result */
static qnumber copySign(qnumber x, qnumber y)
{
    uchar *xch = (uchar *)&x;
    uchar *ych = (uchar *)&y;
    if (QSysInfo::ByteOrder == QSysInfo::BigEndian)
        xch[0] = (xch[0] & 0x7f) | (ych[0] & 0x80);
    else
        xch[7] = (xch[7] & 0x7f) | (ych[7] & 0x80);
    return x;
}

QScriptValue Math::method_min(QScriptContext *context,
                              QScriptEngine *eng)
{
    qnumber mx = qInf();
    for (int i = 0; i < context->argumentCount(); ++i) {
        qnumber x = context->argument(i).toNumber();
        if ((x == 0 && mx == x && copySign(1.0, x) == -1.0)
            || (x < mx) || qIsNan(x)) {
            mx = x;
        }
    }
    return (eng->scriptValue(mx));
}

QScriptValue Math::method_pow(QScriptContext *context,
                              QScriptEngine *eng)
{
    qnumber arg0 = context->argument(0).toNumber();
    qnumber arg1 = context->argument(1).toNumber();
    return (eng->scriptValue(::pow(arg0, arg1)));
}

QScriptValue Math::method_random(QScriptContext *,
                                 QScriptEngine *eng)
{
    return (eng->scriptValue(::rand() / (qnumber) RAND_MAX));
}

QScriptValue Math::method_round(QScriptContext *context,
                                QScriptEngine *eng)
{
    qnumber v = context->argument(0).toNumber();
    v = copySign(::floor(v + 0.5), v);
    return (eng->scriptValue(v));
}

QScriptValue Math::method_sin(QScriptContext *context,
                              QScriptEngine *eng)
{
    qnumber v = context->argument(0).toNumber();
    return (eng->scriptValue(::sin(v)));
}

QScriptValue Math::method_sqrt(QScriptContext *context,
                               QScriptEngine *eng)
{
    qnumber v = context->argument(0).toNumber();
    return (eng->scriptValue(::sqrt(v)));
}

QScriptValue Math::method_tan(QScriptContext *context,
                              QScriptEngine *eng)
{
    qnumber v = context->argument(0).toNumber();
    return (eng->scriptValue(::tan(v)));
}

} } // namespace QScript::Ecma
