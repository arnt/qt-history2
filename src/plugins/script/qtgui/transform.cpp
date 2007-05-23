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
    DECLARE_SELF(QTransform, m11);
    return QScriptValue(eng, self->m11());
}

/////////////////////////////////////////////////////////////

static QScriptValue m12(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QTransform, m12);
    return QScriptValue(eng, self->m12());
}

/////////////////////////////////////////////////////////////

static QScriptValue m13(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QTransform, m13);
    return QScriptValue(eng, self->m13());
}

/////////////////////////////////////////////////////////////

static QScriptValue m21(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QTransform, m21);
    return QScriptValue(eng, self->m21());
}

/////////////////////////////////////////////////////////////

static QScriptValue m22(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QTransform, m22);
    return QScriptValue(eng, self->m22());
}

/////////////////////////////////////////////////////////////

static QScriptValue m23(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QTransform, m23);
    return QScriptValue(eng, self->m23());
}

/////////////////////////////////////////////////////////////

static QScriptValue m31(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QTransform, m31);
    return QScriptValue(eng, self->m31());
}

/////////////////////////////////////////////////////////////

static QScriptValue m32(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QTransform, m32);
    return QScriptValue(eng, self->m32());
}

/////////////////////////////////////////////////////////////

static QScriptValue m33(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QTransform, m33);
    return QScriptValue(eng, self->m33());
}

/////////////////////////////////////////////////////////////

static QScriptValue adjoint(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QTransform, adjoint);
    return newTransform(eng, self->adjoint());
}

/////////////////////////////////////////////////////////////

static QScriptValue det(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QTransform, det);
    return QScriptValue(eng, self->det());
}

/////////////////////////////////////////////////////////////

static QScriptValue determinant(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QTransform, determinant);
    return QScriptValue(eng, self->determinant());
}

/////////////////////////////////////////////////////////////

static QScriptValue dx(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QTransform, dx);
    return QScriptValue(eng, self->dx());
}

/////////////////////////////////////////////////////////////

static QScriptValue dy(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QTransform, dy);
    return QScriptValue(eng, self->dy());
}

/////////////////////////////////////////////////////////////

static QScriptValue inverted(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QTransform, inverted);
    return newTransform(eng, self->inverted());
}

/////////////////////////////////////////////////////////////

static QScriptValue isAffine(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QTransform, isAffine);
    return QScriptValue(eng, self->isAffine());
}

/////////////////////////////////////////////////////////////

static QScriptValue isIdentity(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QTransform, isIdentity);
    return QScriptValue(eng, self->isIdentity());
}

/////////////////////////////////////////////////////////////

static QScriptValue isInvertible(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QTransform, isInvertible);
    return QScriptValue(eng, self->isInvertible());
}

/////////////////////////////////////////////////////////////

static QScriptValue isRotating(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QTransform, isRotating);
    return QScriptValue(eng, self->isRotating());
}

/////////////////////////////////////////////////////////////

static QScriptValue isScaling(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QTransform, isScaling);
    return QScriptValue(eng, self->isScaling());
}

/////////////////////////////////////////////////////////////

static QScriptValue isTranslating(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QTransform, isTranslating);
    return QScriptValue(eng, self->isTranslating());
}

/////////////////////////////////////////////////////////////

static QScriptValue map(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QTransform, map);
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
    DECLARE_SELF(QTransform, mapRect);
    return eng->toScriptValue(self->mapRect(qscriptvalue_cast<QRectF>(ctx->argument(0))));
}

/////////////////////////////////////////////////////////////

static QScriptValue mapToPolygon(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QTransform, mapToPolygon);
    return eng->toScriptValue(self->mapToPolygon(qscriptvalue_cast<QRect>(ctx->argument(0))));
}

/////////////////////////////////////////////////////////////

static QScriptValue reset(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QTransform, reset);
    self->reset();
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue rotate(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(QTransform, rotate);
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
    DECLARE_SELF(QTransform, rotateRadians);
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
    DECLARE_SELF(QTransform, scale);
    self->scale(ctx->argument(0).toNumber(),
                ctx->argument(1).toNumber());
    return ctx->thisObject();
}

/////////////////////////////////////////////////////////////

static QScriptValue setMatrix(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QTransform, setMatrix);
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
    DECLARE_SELF(QTransform, shear);
    self->shear(ctx->argument(0).toNumber(),
                ctx->argument(1).toNumber());
    return ctx->thisObject();
}

/////////////////////////////////////////////////////////////

static QScriptValue toAffine(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QTransform, toAffine);
    return eng->toScriptValue(self->toAffine());
}

/////////////////////////////////////////////////////////////

static QScriptValue translate(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(QTransform, translate);
    self->translate(ctx->argument(0).toNumber(),
                    ctx->argument(1).toNumber());
    return ctx->thisObject();
}

/////////////////////////////////////////////////////////////

static QScriptValue transposed(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QTransform, transposed);
    return newTransform(eng, self->transposed());
}

/////////////////////////////////////////////////////////////

static QScriptValue type(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QTransform, type);
    return QScriptValue(eng, self->type());
}

/////////////////////////////////////////////////////////////

static QScriptValue toString(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QTransform, toString);
    return QScriptValue(eng, "QTransform");
}

/////////////////////////////////////////////////////////////

QScriptValue constructTransformClass(QScriptEngine *eng)
{
    QScriptValue proto = newTransform(eng, QTransform());
    ADD_METHOD(proto, m11);
    ADD_METHOD(proto, m12);
    ADD_METHOD(proto, m13);
    ADD_METHOD(proto, m21);
    ADD_METHOD(proto, m22);
    ADD_METHOD(proto, m23);
    ADD_METHOD(proto, m31);
    ADD_METHOD(proto, m32);
    ADD_METHOD(proto, m33);
    ADD_METHOD(proto, adjoint);
    ADD_METHOD(proto, det);
    ADD_METHOD(proto, determinant);
    ADD_METHOD(proto, dx);
    ADD_METHOD(proto, dy);
    ADD_METHOD(proto, inverted);
    ADD_METHOD(proto, isAffine);
    ADD_METHOD(proto, isIdentity);
    ADD_METHOD(proto, isInvertible);
    ADD_METHOD(proto, isRotating);
    ADD_METHOD(proto, isScaling);
    ADD_METHOD(proto, isTranslating);
    ADD_METHOD(proto, map);
    ADD_METHOD(proto, mapRect);
    ADD_METHOD(proto, mapToPolygon);
    ADD_METHOD(proto, reset);
    ADD_METHOD(proto, rotate);
    ADD_METHOD(proto, rotateRadians);
    ADD_METHOD(proto, scale);
    ADD_METHOD(proto, setMatrix);
    ADD_METHOD(proto, shear);
    ADD_METHOD(proto, toAffine);
    ADD_METHOD(proto, translate);
    ADD_METHOD(proto, transposed);
    ADD_METHOD(proto, type);
    ADD_METHOD(proto, toString);

    eng->setDefaultPrototype(qMetaTypeId<QTransform>(), proto);
    eng->setDefaultPrototype(qMetaTypeId<QTransform*>(), proto);

    return eng->newFunction(ctor, proto);
}
