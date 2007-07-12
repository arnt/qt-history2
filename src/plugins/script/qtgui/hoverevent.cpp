#include <QtScript/QScriptValue>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptContext>
#include <QtScript/QScriptable>
#include <QHoverEvent>
#include "../global.h"

Q_DECLARE_METATYPE(QEvent*)
DECLARE_POINTER_METATYPE(QHoverEvent)

static QScriptValue newHoverEvent(QScriptEngine *eng, QHoverEvent *he)
{
    return QScript::wrapPointer(eng, he);
}

static QScriptValue ctor(QScriptContext *ctx, QScriptEngine *eng)
{
    QEvent::Type type = QEvent::Type(ctx->argument(0).toInt32());
    QPoint pos = qscriptvalue_cast<QPoint>(ctx->argument(1));
    QPoint oldPos = qscriptvalue_cast<QPoint>(ctx->argument(2));
    return newHoverEvent(eng, new QHoverEvent(type, pos, oldPos));
}

static QScriptValue oldPos(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QHoverEvent, oldPos);
    return qScriptValueFromValue(eng, self->oldPos());
}

static QScriptValue pos(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QHoverEvent, pos);
    return qScriptValueFromValue(eng, self->pos());
}

static QScriptValue toString(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QHoverEvent, toString);
    return QScriptValue(eng, QLatin1String("QHoverEvent"));
}

QScriptValue constructHoverEventClass(QScriptEngine *eng)
{
    QScriptValue proto = newHoverEvent(eng, new QHoverEvent(QEvent::HoverEnter, QPoint(), QPoint()));
    proto.setPrototype(eng->defaultPrototype(qMetaTypeId<QEvent*>()));
    ADD_METHOD(proto, oldPos);
    ADD_METHOD(proto, pos);
    ADD_METHOD(proto, toString);

    QScript::registerPointerMetaType<QHoverEvent>(eng, proto);

    QScriptValue ctorFun = eng->newFunction(ctor, proto);

    return ctorFun;
}
