#include <QtScript/QScriptValue>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptContext>
#include <QtGui/QMatrix>
#include <QtGui/QTransform>
#include <QtGui/QPainterPath>
#include <QtGui/QRegion>
#include "../global.h"

Q_DECLARE_METATYPE(QTransform)
Q_DECLARE_METATYPE(QTransform*)
Q_DECLARE_METATYPE(QMatrix)
Q_DECLARE_METATYPE(QMatrix*)
Q_DECLARE_METATYPE(QPainterPath)
Q_DECLARE_METATYPE(QPainterPath*)
Q_DECLARE_METATYPE(QPolygonF)
Q_DECLARE_METATYPE(QRegion)
Q_DECLARE_METATYPE(QRegion*)

static inline QScriptValue newTransform(QScriptEngine *eng, const QTransform &transform)
{
    return eng->newVariant(qVariantFromValue(transform));
}

/////////////////////////////////////////////////////////////

static QScriptValue ctor(QScriptContext *ctx, QScriptEngine *eng)
{
    if (ctx->argumentCount() == 0)
        return newTransform(eng, QTransform());
    if (QMatrix *matrix = qscriptvalue_cast<QMatrix*>(ctx->argument(0)))
        return newTransform(eng, QTransform(*matrix));
    if (ctx->argumentCount() > 6) {
        qreal h33 = (ctx->argumentCount() > 8) ? ctx->argument(8).toNumber() : 1.0;
        return newTransform(eng, QTransform(ctx->argument(0).toNumber(),
                                            ctx->argument(1).toNumber(),
                                            ctx->argument(2).toNumber(),
                                            ctx->argument(3).toNumber(),
                                            ctx->argument(4).toNumber(),
                                            ctx->argument(5).toNumber(),
                                            ctx->argument(6).toNumber(),
                                            ctx->argument(7).toNumber(),
                                            h33));
    } else {
        return newTransform(eng, QTransform(ctx->argument(0).toNumber(),
                                            ctx->argument(1).toNumber(),
                                            ctx->argument(2).toNumber(),
                                            ctx->argument(3).toNumber(),
                                            ctx->argument(4).toNumber(),
                                            ctx->argument(5).toNumber()));
    }
}

/////////////////////////////////////////////////////////////

static QScriptValue m11(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Transform, m11);
    return QScriptValue(eng, self->m11());
}

/////////////////////////////////////////////////////////////

static QScriptValue m12(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Transform, m12);
    return QScriptValue(eng, self->m12());
}

/////////////////////////////////////////////////////////////

static QScriptValue m13(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Transform, m13);
    return QScriptValue(eng, self->m13());
}

/////////////////////////////////////////////////////////////

static QScriptValue m21(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Transform, m21);
    return QScriptValue(eng, self->m21());
}

/////////////////////////////////////////////////////////////

static QScriptValue m22(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Transform, m22);
    return QScriptValue(eng, self->m22());
}

/////////////////////////////////////////////////////////////

static QScriptValue m23(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Transform, m23);
    return QScriptValue(eng, self->m23());
}

/////////////////////////////////////////////////////////////

static QScriptValue m31(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Transform, m31);
    return QScriptValue(eng, self->m31());
}

/////////////////////////////////////////////////////////////

static QScriptValue m32(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Transform, m32);
    return QScriptValue(eng, self->m32());
}

/////////////////////////////////////////////////////////////

static QScriptValue m33(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Transform, m33);
    return QScriptValue(eng, self->m33());
}

/////////////////////////////////////////////////////////////

static QScriptValue adjoint(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Transform, adjoint);
    return newTransform(eng, self->adjoint());
}

/////////////////////////////////////////////////////////////

static QScriptValue det(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Transform, det);
    return QScriptValue(eng, self->det());
}

/////////////////////////////////////////////////////////////

static QScriptValue determinant(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Transform, determinant);
    return QScriptValue(eng, self->determinant());
}

/////////////////////////////////////////////////////////////

static QScriptValue dx(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Transform, dx);
    return QScriptValue(eng, self->dx());
}

/////////////////////////////////////////////////////////////

static QScriptValue dy(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Transform, dy);
    return QScriptValue(eng, self->dy());
}

/////////////////////////////////////////////////////////////

static QScriptValue inverted(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Transform, inverted);
    return newTransform(eng, self->inverted());
}

/////////////////////////////////////////////////////////////

static QScriptValue isAffine(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Transform, isAffine);
    return QScriptValue(eng, self->isAffine());
}

/////////////////////////////////////////////////////////////

static QScriptValue isIdentity(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Transform, isIdentity);
    return QScriptValue(eng, self->isIdentity());
}

/////////////////////////////////////////////////////////////

static QScriptValue isInvertible(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Transform, isInvertible);
    return QScriptValue(eng, self->isInvertible());
}

/////////////////////////////////////////////////////////////

static QScriptValue isRotating(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Transform, isRotating);
    return QScriptValue(eng, self->isRotating());
}

/////////////////////////////////////////////////////////////

static QScriptValue isScaling(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Transform, isScaling);
    return QScriptValue(eng, self->isScaling());
}

/////////////////////////////////////////////////////////////

static QScriptValue isTranslating(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Transform, isTranslating);
    return QScriptValue(eng, self->isTranslating());
}

/////////////////////////////////////////////////////////////

static QScriptValue map(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Transform, map);
    QScriptValue arg = ctx->argument(0);
    if (QPainterPath *path = qscriptvalue_cast<QPainterPath*>(arg)) {
        return eng->toScriptValue(self->map(*path));
    } else if (QRegion *region = qscriptvalue_cast<QRegion*>(arg)) {
        return eng->toScriptValue(self->map(*region));
    }
    // ### Line(F), Point(F)
    return eng->toScriptValue(self->map(qscriptvalue_cast<QPolygonF>(arg)));
}

/////////////////////////////////////////////////////////////

static QScriptValue mapRect(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Transform, mapRect);
    return eng->toScriptValue(self->mapRect(qscriptvalue_cast<QRectF>(ctx->argument(0))));
}

/////////////////////////////////////////////////////////////

static QScriptValue mapToPolygon(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Transform, mapToPolygon);
    return eng->toScriptValue(self->mapToPolygon(qscriptvalue_cast<QRect>(ctx->argument(0))));
}

/////////////////////////////////////////////////////////////

static QScriptValue reset(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Transform, reset);
    self->reset();
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue rotate(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(Transform, rotate);
    qreal a = ctx->argument(0).toNumber();
    if (ctx->argumentCount() > 1) {
        Qt::Axis axis = static_cast<Qt::Axis>(ctx->argument(1).toInt32());
        self->rotate(a, axis);
    } else {
        self->rotate(a);
    }
    return ctx->thisObject();
}

/////////////////////////////////////////////////////////////

static QScriptValue rotateRadians(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(Transform, rotateRadians);
    qreal a = ctx->argument(0).toNumber();
    if (ctx->argumentCount() > 1) {
        Qt::Axis axis = static_cast<Qt::Axis>(ctx->argument(1).toInt32());
        self->rotateRadians(a, axis);
    } else {
        self->rotateRadians(a);
    }
    return ctx->thisObject();
}

/////////////////////////////////////////////////////////////

static QScriptValue scale(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(Transform, scale);
    self->scale(ctx->argument(0).toNumber(),
                ctx->argument(1).toNumber());
    return ctx->thisObject();
}

/////////////////////////////////////////////////////////////

static QScriptValue setMatrix(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Transform, setMatrix);
    self->setMatrix(ctx->argument(0).toNumber(),
                    ctx->argument(1).toNumber(),
                    ctx->argument(2).toNumber(),
                    ctx->argument(3).toNumber(),
                    ctx->argument(4).toNumber(),
                    ctx->argument(5).toNumber(),
                    ctx->argument(6).toNumber(),
                    ctx->argument(7).toNumber(),
                    ctx->argument(8).toNumber());
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue shear(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(Transform, shear);
    self->shear(ctx->argument(0).toNumber(),
                ctx->argument(1).toNumber());
    return ctx->thisObject();
}

/////////////////////////////////////////////////////////////

static QScriptValue toAffine(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Transform, toAffine);
    return eng->toScriptValue(self->toAffine());
}

/////////////////////////////////////////////////////////////

static QScriptValue translate(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(Transform, translate);
    self->translate(ctx->argument(0).toNumber(),
                    ctx->argument(1).toNumber());
    return ctx->thisObject();
}

/////////////////////////////////////////////////////////////

static QScriptValue transposed(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Transform, transposed);
    return newTransform(eng, self->transposed());
}

/////////////////////////////////////////////////////////////

static QScriptValue type(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Transform, type);
    return QScriptValue(eng, self->type());
}

/////////////////////////////////////////////////////////////

static QScriptValue toString(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Transform, toString);
    return QScriptValue(eng, "Transform");
}

/////////////////////////////////////////////////////////////

QScriptValue constructTransformClass(QScriptEngine *eng)
{
    QScriptValue proto = newTransform(eng, QTransform());
    ADD_PROTO_FUNCTION(proto, m11);
    ADD_PROTO_FUNCTION(proto, m12);
    ADD_PROTO_FUNCTION(proto, m13);
    ADD_PROTO_FUNCTION(proto, m21);
    ADD_PROTO_FUNCTION(proto, m22);
    ADD_PROTO_FUNCTION(proto, m23);
    ADD_PROTO_FUNCTION(proto, m31);
    ADD_PROTO_FUNCTION(proto, m32);
    ADD_PROTO_FUNCTION(proto, m33);
    ADD_PROTO_FUNCTION(proto, adjoint);
    ADD_PROTO_FUNCTION(proto, det);
    ADD_PROTO_FUNCTION(proto, determinant);
    ADD_PROTO_FUNCTION(proto, dx);
    ADD_PROTO_FUNCTION(proto, dy);
    ADD_PROTO_FUNCTION(proto, inverted);
    ADD_PROTO_FUNCTION(proto, isAffine);
    ADD_PROTO_FUNCTION(proto, isIdentity);
    ADD_PROTO_FUNCTION(proto, isInvertible);
    ADD_PROTO_FUNCTION(proto, isRotating);
    ADD_PROTO_FUNCTION(proto, isScaling);
    ADD_PROTO_FUNCTION(proto, isTranslating);
    ADD_PROTO_FUNCTION(proto, map);
    ADD_PROTO_FUNCTION(proto, mapRect);
    ADD_PROTO_FUNCTION(proto, mapToPolygon);
    ADD_PROTO_FUNCTION(proto, reset);
    ADD_PROTO_FUNCTION(proto, rotate);
    ADD_PROTO_FUNCTION(proto, rotateRadians);
    ADD_PROTO_FUNCTION(proto, scale);
    ADD_PROTO_FUNCTION(proto, setMatrix);
    ADD_PROTO_FUNCTION(proto, shear);
    ADD_PROTO_FUNCTION(proto, toAffine);
    ADD_PROTO_FUNCTION(proto, translate);
    ADD_PROTO_FUNCTION(proto, transposed);
    ADD_PROTO_FUNCTION(proto, type);
    ADD_PROTO_FUNCTION(proto, toString);

    eng->setDefaultPrototype(qMetaTypeId<QTransform>(), proto);
    eng->setDefaultPrototype(qMetaTypeId<QTransform*>(), proto);

    return eng->newFunction(ctor, proto);
}
