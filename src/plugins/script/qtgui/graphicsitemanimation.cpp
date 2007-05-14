#include <QtScript/QScriptValue>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptContext>
#include <QtScript/QScriptable>
#include <QtGui/QGraphicsItemAnimation>
#include <QtGui/QMatrix>
#include <QtCore/QPointF>
#include "../global.h"

Q_DECLARE_METATYPE(QGraphicsItemAnimation*)
Q_DECLARE_METATYPE(QGraphicsItem*)
Q_DECLARE_METATYPE(QTimeLine*)

static QScriptValue newGraphicsItemAnimation(QScriptEngine *eng, QGraphicsItemAnimation *anim)
{
    return eng->newQObject(anim, QScriptEngine::AutoOwnership);
}

static QScriptValue ctor(QScriptContext *ctx, QScriptEngine *eng)
{
    return newGraphicsItemAnimation(eng, new QGraphicsItemAnimation(
                                        qscriptvalue_cast<QObject*>(ctx->argument(0))));
}

static QScriptValue clear(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsItemAnimation, clear);
    self->clear();
    return eng->undefinedValue();
}

static QScriptValue horizontalScaleAt(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsItemAnimation, horizontalScaleAt);
    return QScriptValue(eng, self->horizontalScaleAt(ctx->argument(0).toNumber()));
}

static QScriptValue horizontalShearAt(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsItemAnimation, horizontalShearAt);
    return QScriptValue(eng, self->horizontalShearAt(ctx->argument(0).toNumber()));
}

static QScriptValue item(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsItemAnimation, item);
    return eng->toScriptValue(self->item());
}

static QScriptValue matrixAt(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsItemAnimation, matrixAt);
    return eng->toScriptValue(self->matrixAt(ctx->argument(0).toNumber()));
}

static QScriptValue posAt(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsItemAnimation, posAt);
    return eng->toScriptValue(self->posAt(ctx->argument(0).toNumber()));
}

static QScriptValue posList(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(GraphicsItemAnimation, posList);
    return ctx->throwError("QGraphicsItemAnimation.prototype.posList is not implemented");
}

static QScriptValue rotationAt(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsItemAnimation, rotationAt);
    return QScriptValue(eng, self->rotationAt(ctx->argument(0).toNumber()));
}

static QScriptValue rotationList(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(GraphicsItemAnimation, rotationList);
    return ctx->throwError("QGraphicsItemAnimation.prototype.rotationList is not implemented");
}

static QScriptValue scaleList(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(GraphicsItemAnimation, scaleList);
    return ctx->throwError("QGraphicsItemAnimation.prototype.scaleList is not implemented");
}

static QScriptValue setItem(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsItemAnimation, setItem);
    self->setItem(qscriptvalue_cast<QGraphicsItem*>(ctx->argument(0)));
    return eng->undefinedValue();
}

static QScriptValue setPosAt(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsItemAnimation, setPosAt);
    self->setPosAt(ctx->argument(0).toNumber(),
                   qscriptvalue_cast<QPointF>(ctx->argument(1)));
    return eng->undefinedValue();
}

static QScriptValue setRotationAt(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsItemAnimation, setRotationAt);
    self->setRotationAt(ctx->argument(0).toNumber(),
                        ctx->argument(1).toNumber());
    return eng->undefinedValue();
}

static QScriptValue setScaleAt(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsItemAnimation, setScaleAt);
    self->setScaleAt(ctx->argument(0).toNumber(),
                     ctx->argument(1).toNumber(),
                     ctx->argument(2).toNumber());
    return eng->undefinedValue();
}

static QScriptValue setShearAt(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsItemAnimation, setShearAt);
    self->setShearAt(ctx->argument(0).toNumber(),
                     ctx->argument(1).toNumber(),
                     ctx->argument(2).toNumber());
    return eng->undefinedValue();
}

static QScriptValue setTimeLine(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsItemAnimation, setTimeLine);
    self->setTimeLine(qscriptvalue_cast<QTimeLine*>(ctx->argument(0)));
    return eng->undefinedValue();
}

static QScriptValue setTranslationAt(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsItemAnimation, setTranslationAt);
    self->setTranslationAt(ctx->argument(0).toNumber(),
                           ctx->argument(1).toNumber(),
                           ctx->argument(2).toNumber());
    return eng->undefinedValue();
}

static QScriptValue shearList(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(GraphicsItemAnimation, shearList);
    return ctx->throwError("QGraphicsItemAnimation.prototype.shearList is not implemented");
}

static QScriptValue timeLine(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsItemAnimation, timeLine);
    return eng->toScriptValue(self->timeLine());
}

static QScriptValue translationList(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(GraphicsItemAnimation, translationList);
    return ctx->throwError("QGraphicsItemAnimation.prototype.translationList is not implemented");
}

static QScriptValue verticalScaleAt(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsItemAnimation, verticalScaleAt);
    return QScriptValue(eng, self->verticalScaleAt(ctx->argument(0).toNumber()));
}

static QScriptValue verticalShearAt(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsItemAnimation, verticalShearAt);
    return QScriptValue(eng, self->verticalShearAt(ctx->argument(0).toNumber()));
}

static QScriptValue xTranslationAt(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsItemAnimation, xTranslationAt);
    return QScriptValue(eng, self->xTranslationAt(ctx->argument(0).toNumber()));
}

static QScriptValue yTranslationAt(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsItemAnimation, yTranslationAt);
    return QScriptValue(eng, self->yTranslationAt(ctx->argument(0).toNumber()));
}

static QScriptValue toString(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsItemAnimation, toString);
    return QScriptValue(eng, QString::fromLatin1("QGraphicsItemAnimation"));
}

QScriptValue constructGraphicsItemAnimationClass(QScriptEngine *eng)
{
    QScriptValue proto = newGraphicsItemAnimation(eng, new QGraphicsItemAnimation());
    ADD_PROTO_FUNCTION(proto, clear);
    ADD_PROTO_FUNCTION(proto, horizontalScaleAt);
    ADD_PROTO_FUNCTION(proto, horizontalShearAt);
    ADD_PROTO_FUNCTION(proto, item);
    ADD_PROTO_FUNCTION(proto, matrixAt);
    ADD_PROTO_FUNCTION(proto, posAt);
    ADD_PROTO_FUNCTION(proto, posList);
    ADD_PROTO_FUNCTION(proto, rotationAt);
    ADD_PROTO_FUNCTION(proto, rotationList);
    ADD_PROTO_FUNCTION(proto, scaleList);
    ADD_PROTO_FUNCTION(proto, setItem);
    ADD_PROTO_FUNCTION(proto, setPosAt);
    ADD_PROTO_FUNCTION(proto, setRotationAt);
    ADD_PROTO_FUNCTION(proto, setScaleAt);
    ADD_PROTO_FUNCTION(proto, setShearAt);
    ADD_PROTO_FUNCTION(proto, setTimeLine);
    ADD_PROTO_FUNCTION(proto, setTranslationAt);
    ADD_PROTO_FUNCTION(proto, shearList);
    ADD_PROTO_FUNCTION(proto, timeLine);
    ADD_PROTO_FUNCTION(proto, translationList);
    ADD_PROTO_FUNCTION(proto, verticalScaleAt);
    ADD_PROTO_FUNCTION(proto, verticalShearAt);
    ADD_PROTO_FUNCTION(proto, xTranslationAt);
    ADD_PROTO_FUNCTION(proto, yTranslationAt);
    ADD_PROTO_FUNCTION(proto, toString);
    eng->setDefaultPrototype(qMetaTypeId<QGraphicsItemAnimation*>(), proto);

    return eng->newFunction(ctor, proto);
}
