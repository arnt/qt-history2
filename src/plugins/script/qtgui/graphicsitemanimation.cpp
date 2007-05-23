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
    DECLARE_SELF(QGraphicsItemAnimation, clear);
    self->clear();
    return eng->undefinedValue();
}

static QScriptValue horizontalScaleAt(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsItemAnimation, horizontalScaleAt);
    return QScriptValue(eng, self->horizontalScaleAt(ctx->argument(0).toNumber()));
}

static QScriptValue horizontalShearAt(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsItemAnimation, horizontalShearAt);
    return QScriptValue(eng, self->horizontalShearAt(ctx->argument(0).toNumber()));
}

static QScriptValue item(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsItemAnimation, item);
    return eng->toScriptValue(self->item());
}

static QScriptValue matrixAt(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsItemAnimation, matrixAt);
    return eng->toScriptValue(self->matrixAt(ctx->argument(0).toNumber()));
}

static QScriptValue posAt(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsItemAnimation, posAt);
    return eng->toScriptValue(self->posAt(ctx->argument(0).toNumber()));
}

static QScriptValue posList(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(QGraphicsItemAnimation, posList);
    return ctx->throwError("QGraphicsItemAnimation.prototype.posList is not implemented");
}

static QScriptValue rotationAt(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsItemAnimation, rotationAt);
    return QScriptValue(eng, self->rotationAt(ctx->argument(0).toNumber()));
}

static QScriptValue rotationList(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(QGraphicsItemAnimation, rotationList);
    return ctx->throwError("QGraphicsItemAnimation.prototype.rotationList is not implemented");
}

static QScriptValue scaleList(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(QGraphicsItemAnimation, scaleList);
    return ctx->throwError("QGraphicsItemAnimation.prototype.scaleList is not implemented");
}

static QScriptValue setItem(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsItemAnimation, setItem);
    self->setItem(qscriptvalue_cast<QGraphicsItem*>(ctx->argument(0)));
    return eng->undefinedValue();
}

static QScriptValue setPosAt(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsItemAnimation, setPosAt);
    self->setPosAt(ctx->argument(0).toNumber(),
                   qscriptvalue_cast<QPointF>(ctx->argument(1)));
    return eng->undefinedValue();
}

static QScriptValue setRotationAt(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsItemAnimation, setRotationAt);
    self->setRotationAt(ctx->argument(0).toNumber(),
                        ctx->argument(1).toNumber());
    return eng->undefinedValue();
}

static QScriptValue setScaleAt(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsItemAnimation, setScaleAt);
    self->setScaleAt(ctx->argument(0).toNumber(),
                     ctx->argument(1).toNumber(),
                     ctx->argument(2).toNumber());
    return eng->undefinedValue();
}

static QScriptValue setShearAt(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsItemAnimation, setShearAt);
    self->setShearAt(ctx->argument(0).toNumber(),
                     ctx->argument(1).toNumber(),
                     ctx->argument(2).toNumber());
    return eng->undefinedValue();
}

static QScriptValue setTimeLine(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsItemAnimation, setTimeLine);
    self->setTimeLine(qscriptvalue_cast<QTimeLine*>(ctx->argument(0)));
    return eng->undefinedValue();
}

static QScriptValue setTranslationAt(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsItemAnimation, setTranslationAt);
    self->setTranslationAt(ctx->argument(0).toNumber(),
                           ctx->argument(1).toNumber(),
                           ctx->argument(2).toNumber());
    return eng->undefinedValue();
}

static QScriptValue shearList(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(QGraphicsItemAnimation, shearList);
    return ctx->throwError("QGraphicsItemAnimation.prototype.shearList is not implemented");
}

static QScriptValue timeLine(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsItemAnimation, timeLine);
    return eng->toScriptValue(self->timeLine());
}

static QScriptValue translationList(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(QGraphicsItemAnimation, translationList);
    return ctx->throwError("QGraphicsItemAnimation.prototype.translationList is not implemented");
}

static QScriptValue verticalScaleAt(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsItemAnimation, verticalScaleAt);
    return QScriptValue(eng, self->verticalScaleAt(ctx->argument(0).toNumber()));
}

static QScriptValue verticalShearAt(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsItemAnimation, verticalShearAt);
    return QScriptValue(eng, self->verticalShearAt(ctx->argument(0).toNumber()));
}

static QScriptValue xTranslationAt(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsItemAnimation, xTranslationAt);
    return QScriptValue(eng, self->xTranslationAt(ctx->argument(0).toNumber()));
}

static QScriptValue yTranslationAt(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsItemAnimation, yTranslationAt);
    return QScriptValue(eng, self->yTranslationAt(ctx->argument(0).toNumber()));
}

static QScriptValue toString(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsItemAnimation, toString);
    return QScriptValue(eng, QString::fromLatin1("QGraphicsItemAnimation"));
}

QScriptValue constructGraphicsItemAnimationClass(QScriptEngine *eng)
{
    QScriptValue proto = newGraphicsItemAnimation(eng, new QGraphicsItemAnimation());
    ADD_METHOD(proto, clear);
    ADD_METHOD(proto, horizontalScaleAt);
    ADD_METHOD(proto, horizontalShearAt);
    ADD_METHOD(proto, item);
    ADD_METHOD(proto, matrixAt);
    ADD_METHOD(proto, posAt);
    ADD_METHOD(proto, posList);
    ADD_METHOD(proto, rotationAt);
    ADD_METHOD(proto, rotationList);
    ADD_METHOD(proto, scaleList);
    ADD_METHOD(proto, setItem);
    ADD_METHOD(proto, setPosAt);
    ADD_METHOD(proto, setRotationAt);
    ADD_METHOD(proto, setScaleAt);
    ADD_METHOD(proto, setShearAt);
    ADD_METHOD(proto, setTimeLine);
    ADD_METHOD(proto, setTranslationAt);
    ADD_METHOD(proto, shearList);
    ADD_METHOD(proto, timeLine);
    ADD_METHOD(proto, translationList);
    ADD_METHOD(proto, verticalScaleAt);
    ADD_METHOD(proto, verticalShearAt);
    ADD_METHOD(proto, xTranslationAt);
    ADD_METHOD(proto, yTranslationAt);
    ADD_METHOD(proto, toString);
    eng->setDefaultPrototype(qMetaTypeId<QGraphicsItemAnimation*>(), proto);

    return eng->newFunction(ctor, proto);
}
