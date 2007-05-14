#include <QtScript/QScriptValue>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptContext>
#include <QtGui/QGraphicsRectItem>
#include "../global.h"

Q_DECLARE_METATYPE(QGraphicsRectItem*)
Q_DECLARE_METATYPE(QScript::Wrapper<QGraphicsRectItem*>::pointer_type)
Q_DECLARE_METATYPE(QAbstractGraphicsShapeItem*)

/////////////////////////////////////////////////////////////

static QScriptValue ctor(QScriptContext *ctx, QScriptEngine *eng)
{
    if (ctx->argumentCount() >= 4) {
        return QScript::construct<QGraphicsRectItem>(
            eng, new QGraphicsRectItem(ctx->argument(0).toNumber(),
                                       ctx->argument(1).toNumber(),
                                       ctx->argument(2).toNumber(),
                                       ctx->argument(3).toNumber(),
                                       qscriptvalue_cast<QGraphicsItem*>(ctx->argument(4))));
    } else if (ctx->argumentCount() > 1) {
        return QScript::construct<QGraphicsRectItem>(
            eng, new QGraphicsRectItem(qscriptvalue_cast<QRectF>(ctx->argument(0)),
                                       qscriptvalue_cast<QGraphicsItem*>(ctx->argument(1))));
    } else {
        if (QGraphicsItem *parent = qscriptvalue_cast<QGraphicsItem*>(ctx->argument(0))) {
            return QScript::construct<QGraphicsRectItem>(
                eng, new QGraphicsRectItem(parent));
        } else {
            return QScript::construct<QGraphicsRectItem>(
                eng, new QGraphicsRectItem(qscriptvalue_cast<QRectF>(ctx->argument(0))));
        }
    }
}

/////////////////////////////////////////////////////////////

static QScriptValue rect(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsRectItem, rect);
    return eng->toScriptValue(self->rect());
}

/////////////////////////////////////////////////////////////

static QScriptValue setRect(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsRectItem, setRect);
    if (ctx->argumentCount() > 1) {
        self->setRect(ctx->argument(0).toNumber(),
                      ctx->argument(1).toNumber(),
                      ctx->argument(2).toNumber(),
                      ctx->argument(3).toNumber());
    } else {
        self->setRect(qscriptvalue_cast<QRectF>(ctx->argument(0)));
    }
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue toString(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsRectItem, toString);
    return QScriptValue(eng, "QGraphicsRectItem");
}

/////////////////////////////////////////////////////////////

QScriptValue constructGraphicsRectItemClass(QScriptEngine *eng)
{
    QScriptValue proto = QScript::construct<QGraphicsRectItem>(eng, new QGraphicsRectItem());
    proto.setPrototype(eng->defaultPrototype(qMetaTypeId<QAbstractGraphicsShapeItem*>()));

    ADD_PROTO_FUNCTION(proto, rect);
    ADD_PROTO_FUNCTION(proto, setRect);
    ADD_PROTO_FUNCTION(proto, toString);

    QScript::registerMetaTypeWrapper<QScript::Wrapper<QGraphicsRectItem*> >(eng, proto);

    return eng->newFunction(ctor, proto);
}
