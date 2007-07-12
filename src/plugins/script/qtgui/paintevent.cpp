#include <QtScript/QScriptValue>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptContext>
#include <QtScript/QScriptable>
#include <QPaintEvent>
#include "../global.h"

Q_DECLARE_METATYPE(QEvent*)
DECLARE_POINTER_METATYPE(QPaintEvent)

static QScriptValue newPaintEvent(QScriptEngine *eng, QPaintEvent *pe)
{
    return QScript::wrapPointer(eng, pe);
}

static QScriptValue ctor(QScriptContext *ctx, QScriptEngine *eng)
{
    QRect rect = qscriptvalue_cast<QRect>(ctx->argument(0));
    return newPaintEvent(eng, new QPaintEvent(rect));
}

static QScriptValue rect(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPaintEvent, rect);
    return qScriptValueFromValue(eng, self->rect());
}

static QScriptValue region(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPaintEvent, region);
    return qScriptValueFromValue(eng, self->region());
}

static QScriptValue toString(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPaintEvent, toString);
    return QScriptValue(eng, QLatin1String("QPaintEvent"));
}

QScriptValue constructPaintEventClass(QScriptEngine *eng)
{
    QScriptValue proto = newPaintEvent(eng, new QPaintEvent(QRect()));
    proto.setPrototype(eng->defaultPrototype(qMetaTypeId<QEvent*>()));
    ADD_METHOD(proto, rect);
    ADD_METHOD(proto, region);
    ADD_METHOD(proto, toString);

    QScript::registerPointerMetaType<QPaintEvent>(eng, proto);

    QScriptValue ctorFun = eng->newFunction(ctor, proto);

    return ctorFun;
}
