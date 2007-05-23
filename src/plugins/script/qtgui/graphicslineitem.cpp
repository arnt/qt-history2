#include <QtScript/QScriptValue>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptContext>
#include <QtGui/QGraphicsLineItem>
#include <QtGui/QPen>
#include "../global.h"

DECLARE_POINTER_METATYPE(QGraphicsLineItem)

static QScriptValue ctor(QScriptContext *ctx, QScriptEngine *eng)
{
    if (ctx->argumentCount() >= 4) {
        return QScript::wrapGVPointer(
            eng, new QGraphicsLineItem(ctx->argument(0).toNumber(),
                                       ctx->argument(1).toNumber(),
                                       ctx->argument(2).toNumber(),
                                       ctx->argument(3).toNumber(),
                                       qscriptvalue_cast<QGraphicsItem*>(ctx->argument(4))));
    } else if (ctx->argumentCount() > 1) {
        return QScript::wrapGVPointer(
            eng, new QGraphicsLineItem(qscriptvalue_cast<QLineF>(ctx->argument(0)),
                                       qscriptvalue_cast<QGraphicsItem*>(ctx->argument(1))));
    } else {
        return QScript::wrapGVPointer(
            eng, new QGraphicsLineItem(qscriptvalue_cast<QGraphicsItem*>(ctx->argument(0))));
    }
}

static QScriptValue line(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsLineItem, line);
    return eng->toScriptValue(self->line());
}

static QScriptValue pen(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsLineItem, pen);
    return eng->toScriptValue(self->pen());
}

static QScriptValue setLine(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsLineItem, setLine);
    if (ctx->argumentCount() > 1) {
        self->setLine(ctx->argument(0).toNumber(),
                      ctx->argument(1).toNumber(),
                      ctx->argument(2).toNumber(),
                      ctx->argument(3).toNumber());
    } else {
        self->setLine(qscriptvalue_cast<QLineF>(ctx->argument(0)));
    }
    return eng->undefinedValue();
}

static QScriptValue setPen(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsLineItem, setPen);
    self->setPen(qscriptvalue_cast<QPen>(ctx->argument(0)));
    return eng->undefinedValue();
}

static QScriptValue toString(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsLineItem, toString);
    return QScriptValue(eng, "QGraphicsLineItem");
}

QScriptValue constructGraphicsLineItemClass(QScriptEngine *eng)
{
    QScriptValue proto = QScript::wrapGVPointer(eng, new QGraphicsLineItem());
    proto.setPrototype(eng->defaultPrototype(qMetaTypeId<QGraphicsItem*>()));

    ADD_PROTO_FUNCTION(proto, line);
    ADD_PROTO_FUNCTION(proto, pen);
    ADD_PROTO_FUNCTION(proto, setLine);
    ADD_PROTO_FUNCTION(proto, setPen);
    ADD_PROTO_FUNCTION(proto, toString);

    QScript::registerPointerMetaType<QGraphicsLineItem>(eng, proto);

    return eng->newFunction(ctor, proto);
}
