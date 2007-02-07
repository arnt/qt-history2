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
                        QScriptValue(eng, ::exp(1.0)), flags);
    object->setProperty(QLatin1String("LN2"),
                        QScriptValue(eng, ::log(2.0)), flags);
    object->setProperty(QLatin1String("LN10"),
                        QScriptValue(eng, ::log(10.0)), flags);
    object->setProperty(QLatin1String("LOG2E"),
                        QScriptValue(eng, 1.0/::log(2.0)), flags);
    object->setProperty(QLatin1String("LOG10E"),
                        QScriptValue(eng, 1.0/::log(10.0)), flags);
    object->setProperty(QLatin1String("PI"),
                        QScriptValue(eng, 2.0 * ::asin(1.0)), flags);
    object->setProperty(QLatin1String("SQRT1_2"),
                        QScriptValue(eng, ::sqrt(0.5)), flags);
    object->setProperty(QLatin1String("SQRT2"),
                        QScriptValue(eng, ::sqrt(2.0)), flags);

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
    qsreal v = context->argument(0).toNumber();
    if (v == 0) // 0 | -0
        return (QScriptValue(eng, 0));
    else
        return (QScriptValue(eng, v < 0 ? -v : v));
}

QScriptValue Math::method_acos(QScriptContext *context,
                               QScriptEngine *eng)
{
    qsreal v = context->argument(0).toNumber();
    return (QScriptValue(eng, ::acos(v)));
}

QScriptValue Math::method_asin(QScriptContext *context,
                               QScriptEngine *eng)
{
    qsreal v = context->argument(0).toNumber();
    return (QScriptValue(eng, ::asin(v)));
}

QScriptValue Math::method_atan(QScriptContext *context,
                               QScriptEngine *eng)
{
    qsreal v = context->argument(0).toNumber();
    return (QScriptValue(eng, ::atan(v)));
}

QScriptValue Math::method_atan2(QScriptContext *context,
                                QScriptEngine *eng)
{
    qsreal v1 = context->argument(0).toNumber();
    qsreal v2 = context->argument(1).toNumber();
    return (QScriptValue(eng, ::atan2(v1, v2)));
}

QScriptValue Math::method_ceil(QScriptContext *context,
                               QScriptEngine *eng)
{
    qsreal v = context->argument(0).toNumber();
    return (QScriptValue(eng, ::ceil(v)));
}

QScriptValue Math::method_cos(QScriptContext *context,
                              QScriptEngine *eng)
{
    qsreal v = context->argument(0).toNumber();
    return (QScriptValue(eng, ::cos(v)));
}

QScriptValue Math::method_exp(QScriptContext *context,
                              QScriptEngine *eng)
{
    qsreal v = context->argument(0).toNumber();
    return (QScriptValue(eng, ::exp(v)));
}

QScriptValue Math::method_floor(QScriptContext *context,
                                QScriptEngine *eng)
{
    qsreal v = context->argument(0).toNumber();
    return (QScriptValue(eng, ::floor(v)));
}

QScriptValue Math::method_log(QScriptContext *context,
                              QScriptEngine *eng)
{
    qsreal v = context->argument(0).toNumber();
    return (QScriptValue(eng, ::log(v)));
}

QScriptValue Math::method_max(QScriptContext *context,
                              QScriptEngine *eng)
{
    qsreal mx = -qInf();
    for (int i = 0; i < context->argumentCount(); ++i) {
        qsreal x = context->argument(i).toNumber();
        if (x > mx || qIsNan(x))
            mx = x;
    }
    return (QScriptValue(eng, mx));
}

/* copies the sign from y to x and returns the result */
static qsreal copySign(qsreal x, qsreal y)
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
    qsreal mx = qInf();
    for (int i = 0; i < context->argumentCount(); ++i) {
        qsreal x = context->argument(i).toNumber();
        if ((x == 0 && mx == x && copySign(1.0, x) == -1.0)
            || (x < mx) || qIsNan(x)) {
            mx = x;
        }
    }
    return (QScriptValue(eng, mx));
}

QScriptValue Math::method_pow(QScriptContext *context,
                              QScriptEngine *eng)
{
    qsreal x = context->argument(0).toNumber();
    qsreal y = context->argument(1).toNumber();
    if (qIsNan(y))
        return QScriptValue(eng, qSNan());
    if ((x == 1) && qIsInf(y))
        return QScriptValue(eng, qSNan());
    return (QScriptValue(eng, ::pow(x, y)));
}

QScriptValue Math::method_random(QScriptContext *,
                                 QScriptEngine *eng)
{
    return (QScriptValue(eng, qrand() / (qsreal) RAND_MAX));
}

QScriptValue Math::method_round(QScriptContext *context,
                                QScriptEngine *eng)
{
    qsreal v = context->argument(0).toNumber();
    v = copySign(::floor(v + 0.5), v);
    return (QScriptValue(eng, v));
}

QScriptValue Math::method_sin(QScriptContext *context,
                              QScriptEngine *eng)
{
    qsreal v = context->argument(0).toNumber();
    return (QScriptValue(eng, ::sin(v)));
}

QScriptValue Math::method_sqrt(QScriptContext *context,
                               QScriptEngine *eng)
{
    qsreal v = context->argument(0).toNumber();
    return (QScriptValue(eng, ::sqrt(v)));
}

QScriptValue Math::method_tan(QScriptContext *context,
                              QScriptEngine *eng)
{
    qsreal v = context->argument(0).toNumber();
    return (QScriptValue(eng, ::tan(v)));
}

} } // namespace QScript::Ecma
