#include <QtScript/QScriptValue>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptContext>
#include <QtGui/QGraphicsPixmapItem>
#include "../global.h"

DECLARE_POINTER_METATYPE(QGraphicsPixmapItem)
Q_DECLARE_METATYPE(QPixmap*)

static QScriptValue ctor(QScriptContext *ctx, QScriptEngine *eng)
{
    QScriptValue arg = ctx->argument(0);
    if (QPixmap *pixmap = qscriptvalue_cast<QPixmap*>(arg)) {
        return QScript::wrapGVPointer(
            eng, new QGraphicsPixmapItem(*pixmap,
                                         qscriptvalue_cast<QGraphicsItem*>(ctx->argument(1))));
    } else {
        return QScript::wrapGVPointer(
            eng, new QGraphicsPixmapItem(qscriptvalue_cast<QGraphicsItem*>(arg)));
    }
}

static QScriptValue offset(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsPixmapItem, offset);
    return eng->toScriptValue(self->offset());
}

static QScriptValue pixmap(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsPixmapItem, pixmap);
    return eng->toScriptValue(self->pixmap());
}

static QScriptValue setOffset(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsPixmapItem, setOffset);
    if (ctx->argumentCount() > 1) {
        self->setOffset(ctx->argument(0).toNumber(),
                        ctx->argument(1).toNumber());
    } else {
        self->setOffset(qscriptvalue_cast<QPointF>(ctx->argument(0)));
    }
    return eng->undefinedValue();
}

static QScriptValue setPixmap(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsPixmapItem, setPixmap);
    self->setPixmap(qscriptvalue_cast<QPixmap>(ctx->argument(0)));
    return eng->undefinedValue();
}

static QScriptValue setShapeMode(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsPixmapItem, setShapeMode);
    self->setShapeMode(static_cast<QGraphicsPixmapItem::ShapeMode>(ctx->argument(0).toInt32()));
    return eng->undefinedValue();
}

static QScriptValue setTransformationMode(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsPixmapItem, setTransformationMode);
    self->setTransformationMode(static_cast<Qt::TransformationMode>(ctx->argument(0).toInt32()));
    return eng->undefinedValue();
}

static QScriptValue shapeMode(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsPixmapItem, shapeMode);
    return QScriptValue(eng, static_cast<int>(self->shapeMode()));
}

static QScriptValue transformationMode(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsPixmapItem, transformationMode);
    return QScriptValue(eng, static_cast<int>(self->transformationMode()));
}

static QScriptValue toString(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsPixmapItem, toString);
    return QScriptValue(eng, "QGraphicsPixmapItem");
}

QScriptValue constructGraphicsPixmapItemClass(QScriptEngine *eng)
{
    QScriptValue proto = QScript::wrapGVPointer(eng, new QGraphicsPixmapItem());
    proto.setPrototype(eng->defaultPrototype(qMetaTypeId<QGraphicsItem*>()));

    ADD_METHOD(proto, offset);
    ADD_METHOD(proto, pixmap);
    ADD_METHOD(proto, setOffset);
    ADD_METHOD(proto, setPixmap);
    ADD_METHOD(proto, setShapeMode);
    ADD_METHOD(proto, setTransformationMode);
    ADD_METHOD(proto, shapeMode);
    ADD_METHOD(proto, toString);
    ADD_METHOD(proto, transformationMode);

    QScript::registerPointerMetaType<QGraphicsPixmapItem>(eng, proto);

    QScriptValue ctorFun = eng->newFunction(ctor, proto);
    ADD_ENUM_VALUE(ctorFun, QGraphicsPixmapItem, MaskShape);
    ADD_ENUM_VALUE(ctorFun, QGraphicsPixmapItem, BoundingRectShape);
    ADD_ENUM_VALUE(ctorFun, QGraphicsPixmapItem, HeuristicMaskShape);

    return ctorFun;
}
