#include <QtScript/QScriptValue>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptContext>
#include <QtGui/QGraphicsEllipseItem>
#include "../global.h"

DECLARE_POINTER_METATYPE(QGraphicsEllipseItem)
Q_DECLARE_METATYPE(QAbstractGraphicsShapeItem*)

static QScriptValue ctor(QScriptContext *ctx, QScriptEngine *eng)
{
    if (ctx->argumentCount() >= 4) {
        return QScript::wrapGVPointer(eng,
            new QGraphicsEllipseItem(ctx->argument(0).toNumber(),
                                     ctx->argument(1).toNumber(),
                                     ctx->argument(2).toNumber(),
                                     ctx->argument(3).toNumber(),
                                     qscriptvalue_cast<QGraphicsItem*>(ctx->argument(4))));
    } else if (ctx->argumentCount() > 1) {
        return QScript::wrapGVPointer(eng,
            new QGraphicsEllipseItem(qscriptvalue_cast<QRectF>(ctx->argument(0)),
                                     qscriptvalue_cast<QGraphicsItem*>(ctx->argument(1))));
    } else {
        return QScript::wrapGVPointer(eng,
            new QGraphicsEllipseItem(qscriptvalue_cast<QGraphicsItem*>(ctx->argument(0))));
    }
}

static QScriptValue rect(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsEllipseItem, rect);
    return eng->toScriptValue(self->rect());
}

static QScriptValue setRect(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsEllipseItem, setRect);
    self->setRect(qscriptvalue_cast<QRectF>(ctx->argument(0)));
    return eng->undefinedValue();
}

static QScriptValue setSpanAngle(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsEllipseItem, setSpanAngle);
    self->setSpanAngle(ctx->argument(0).toInt32());
    return eng->undefinedValue();
}

static QScriptValue setStartAngle(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsEllipseItem, setStartAngle);
    self->setStartAngle(ctx->argument(0).toInt32());
    return eng->undefinedValue();
}

static QScriptValue spanAngle(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsEllipseItem, spanAngle);
    return QScriptValue(eng, self->spanAngle());
}

static QScriptValue startAngle(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsEllipseItem, startAngle);
    return QScriptValue(eng, self->startAngle());
}

static QScriptValue toString(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsEllipseItem, toString);
    return QScriptValue(eng, "QGraphicsEllipseItem");
}

QScriptValue constructGraphicsEllipseItemClass(QScriptEngine *eng)
{
    QScriptValue proto = QScript::wrapGVPointer(eng, new QGraphicsEllipseItem());
    proto.setPrototype(eng->defaultPrototype(qMetaTypeId<QAbstractGraphicsShapeItem*>()));

    ADD_PROTO_FUNCTION(proto, rect);
    ADD_PROTO_FUNCTION(proto, setRect);
    ADD_PROTO_FUNCTION(proto, setSpanAngle);
    ADD_PROTO_FUNCTION(proto, setStartAngle);
    ADD_PROTO_FUNCTION(proto, spanAngle);
    ADD_PROTO_FUNCTION(proto, startAngle);
    ADD_PROTO_FUNCTION(proto, toString);

    QScript::registerPointerMetaType<QGraphicsEllipseItem>(eng, proto);

    return eng->newFunction(ctor, proto);
}
