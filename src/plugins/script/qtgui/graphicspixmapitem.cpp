#include <QtScript/QScriptValue>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptContext>
#include <QtGui/QGraphicsPixmapItem>
#include "../global.h"

Q_DECLARE_METATYPE(QGraphicsPixmapItem*)
Q_DECLARE_METATYPE(QScript::Wrapper<QGraphicsPixmapItem*>::pointer_type)
Q_DECLARE_METATYPE(QPixmap*)

/////////////////////////////////////////////////////////////

static QScriptValue ctor(QScriptContext *ctx, QScriptEngine *eng)
{
    QScriptValue arg = ctx->argument(0);
    if (QPixmap *pixmap = qscriptvalue_cast<QPixmap*>(arg)) {
        return QScript::construct<QGraphicsPixmapItem>(
            eng, new QGraphicsPixmapItem(*pixmap,
                                         qscriptvalue_cast<QGraphicsItem*>(ctx->argument(1))));
    } else {
        return QScript::construct<QGraphicsPixmapItem>(
            eng, new QGraphicsPixmapItem(qscriptvalue_cast<QGraphicsItem*>(arg)));
    }
}

/////////////////////////////////////////////////////////////

static QScriptValue offset(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsPixmapItem, offset);
    return eng->toScriptValue(self->offset());
}

/////////////////////////////////////////////////////////////

static QScriptValue pixmap(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsPixmapItem, pixmap);
    return eng->toScriptValue(self->pixmap());
}

/////////////////////////////////////////////////////////////

static QScriptValue setOffset(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsPixmapItem, setOffset);
    if (ctx->argumentCount() > 1) {
        self->setOffset(ctx->argument(0).toNumber(),
                        ctx->argument(1).toNumber());
    } else {
        self->setOffset(qscriptvalue_cast<QPointF>(ctx->argument(0)));
    }
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setPixmap(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsPixmapItem, setPixmap);
    self->setPixmap(qscriptvalue_cast<QPixmap>(ctx->argument(0)));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setShapeMode(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsPixmapItem, setShapeMode);
    self->setShapeMode(static_cast<QGraphicsPixmapItem::ShapeMode>(ctx->argument(0).toInt32()));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setTransformationMode(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsPixmapItem, setTransformationMode);
    self->setTransformationMode(static_cast<Qt::TransformationMode>(ctx->argument(0).toInt32()));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue shapeMode(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsPixmapItem, shapeMode);
    return QScriptValue(eng, static_cast<int>(self->shapeMode()));
}

/////////////////////////////////////////////////////////////

static QScriptValue transformationMode(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsPixmapItem, transformationMode);
    return QScriptValue(eng, static_cast<int>(self->transformationMode()));
}

/////////////////////////////////////////////////////////////

static QScriptValue toString(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsPixmapItem, toString);
    return QScriptValue(eng, "QGraphicsPixmapItem");
}

/////////////////////////////////////////////////////////////

QScriptValue constructGraphicsPixmapItemClass(QScriptEngine *eng)
{
    QScriptValue proto = QScript::construct<QGraphicsPixmapItem>(eng, new QGraphicsPixmapItem());
    proto.setPrototype(eng->defaultPrototype(qMetaTypeId<QGraphicsItem*>()));

    ADD_PROTO_FUNCTION(proto, offset);
    ADD_PROTO_FUNCTION(proto, pixmap);
    ADD_PROTO_FUNCTION(proto, setOffset);
    ADD_PROTO_FUNCTION(proto, setPixmap);
    ADD_PROTO_FUNCTION(proto, setShapeMode);
    ADD_PROTO_FUNCTION(proto, setTransformationMode);
    ADD_PROTO_FUNCTION(proto, shapeMode);
    ADD_PROTO_FUNCTION(proto, toString);
    ADD_PROTO_FUNCTION(proto, transformationMode);

    QScript::registerMetaTypeWrapper<QScript::Wrapper<QGraphicsPixmapItem*> >(eng, proto);

    QScriptValue ctorFun = eng->newFunction(ctor, proto);
    ADD_ENUM_VALUE(ctorFun, QGraphicsPixmapItem, MaskShape);
    ADD_ENUM_VALUE(ctorFun, QGraphicsPixmapItem, BoundingRectShape);
    ADD_ENUM_VALUE(ctorFun, QGraphicsPixmapItem, HeuristicMaskShape);

    return ctorFun;
}
