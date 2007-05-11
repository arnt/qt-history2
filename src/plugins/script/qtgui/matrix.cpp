#include <QtScript/QScriptValue>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptContext>
#include <QtGui/QMatrix>
#include <QtGui/QPainterPath>
#include <QtGui/QRegion>
#include "../global.h"

Q_DECLARE_METATYPE(QMatrix)
Q_DECLARE_METATYPE(QMatrix*)
Q_DECLARE_METATYPE(QPainterPath)
Q_DECLARE_METATYPE(QPainterPath*)
Q_DECLARE_METATYPE(QPolygonF)
Q_DECLARE_METATYPE(QRegion)
Q_DECLARE_METATYPE(QRegion*)

static inline QScriptValue newMatrix(QScriptEngine *eng, const QMatrix &matrix)
{
    return eng->newVariant(qVariantFromValue(matrix));
}

/////////////////////////////////////////////////////////////

static QScriptValue ctor(QScriptContext *ctx, QScriptEngine *eng)
{
    if (ctx->argumentCount() == 0)
        return newMatrix(eng, QMatrix());
    if (QMatrix *other = qscriptvalue_cast<QMatrix*>(ctx->argument(0)))
        return newMatrix(eng, QMatrix(*other));
    return newMatrix(eng, QMatrix(ctx->argument(0).toNumber(),
                                  ctx->argument(1).toNumber(),
                                  ctx->argument(2).toNumber(),
                                  ctx->argument(3).toNumber(),
                                  ctx->argument(4).toNumber(),
                                  ctx->argument(5).toNumber()));
}

/////////////////////////////////////////////////////////////

static QScriptValue m11(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Matrix, m11);
    return QScriptValue(eng, self->m11());
}

/////////////////////////////////////////////////////////////

static QScriptValue m12(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Matrix, m12);
    return QScriptValue(eng, self->m12());
}

/////////////////////////////////////////////////////////////

static QScriptValue m21(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Matrix, m21);
    return QScriptValue(eng, self->m21());
}

/////////////////////////////////////////////////////////////

static QScriptValue m22(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Matrix, m22);
    return QScriptValue(eng, self->m22());
}

/////////////////////////////////////////////////////////////

static QScriptValue det(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Matrix, det);
    return QScriptValue(eng, self->det());
}

/////////////////////////////////////////////////////////////

static QScriptValue dx(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Matrix, dx);
    return QScriptValue(eng, self->dx());
}

/////////////////////////////////////////////////////////////

static QScriptValue dy(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Matrix, dy);
    return QScriptValue(eng, self->dy());
}

/////////////////////////////////////////////////////////////

static QScriptValue inverted(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Matrix, inverted);
    return newMatrix(eng, self->inverted());
}

/////////////////////////////////////////////////////////////

static QScriptValue isIdentity(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Matrix, isIdentity);
    return QScriptValue(eng, self->isIdentity());
}

/////////////////////////////////////////////////////////////

static QScriptValue isInvertible(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Matrix, isInvertible);
    return QScriptValue(eng, self->isInvertible());
}

/////////////////////////////////////////////////////////////

static QScriptValue map(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Matrix, map);
    QScriptValue arg = ctx->argument(0);
    if (QPainterPath *path = qscriptvalue_cast<QPainterPath*>(arg))
        return eng->toScriptValue(self->map(*path));
    else if (QRegion *region = qscriptvalue_cast<QRegion*>(arg))
        return eng->toScriptValue(self->map(*region));
    // ### Point(F), Line(F)
    return eng->toScriptValue(self->map(qscriptvalue_cast<QPolygonF>(arg)));
}

/////////////////////////////////////////////////////////////

static QScriptValue mapRect(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Matrix, mapRect);
    return eng->toScriptValue(self->mapRect(qscriptvalue_cast<QRectF>(ctx->argument(0))));
}

/////////////////////////////////////////////////////////////

static QScriptValue mapToPolygon(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Matrix, mapToPolygon);
    return eng->toScriptValue(self->mapToPolygon(qscriptvalue_cast<QRect>(ctx->argument(0))));
}

/////////////////////////////////////////////////////////////

static QScriptValue reset(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Matrix, reset);
    self->reset();
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue rotate(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(Matrix, rotate);
    self->rotate(ctx->argument(0).toNumber());
    return ctx->thisObject();
}

/////////////////////////////////////////////////////////////

static QScriptValue scale(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(Matrix, scale);
    self->scale(ctx->argument(0).toNumber(),
                ctx->argument(1).toNumber());
    return ctx->thisObject();
}

/////////////////////////////////////////////////////////////

static QScriptValue setMatrix(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Matrix, setMatrix);
    self->setMatrix(ctx->argument(0).toNumber(),
                    ctx->argument(1).toNumber(),
                    ctx->argument(2).toNumber(),
                    ctx->argument(3).toNumber(),
                    ctx->argument(4).toNumber(),
                    ctx->argument(5).toNumber());
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue shear(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(Matrix, shear);
    self->shear(ctx->argument(0).toNumber(),
                ctx->argument(1).toNumber());
    return ctx->thisObject();
}

/////////////////////////////////////////////////////////////

static QScriptValue translate(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(Matrix, translate);
    self->translate(ctx->argument(0).toNumber(),
                    ctx->argument(1).toNumber());
    return ctx->thisObject();
}

/////////////////////////////////////////////////////////////

static QScriptValue toString(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Matrix, toString);
    return QScriptValue(eng, "Matrix");
}

/////////////////////////////////////////////////////////////

QScriptValue constructMatrixClass(QScriptEngine *eng)
{
    QScriptValue proto = newMatrix(eng, QMatrix());
    ADD_PROTO_FUNCTION(proto, m11);
    ADD_PROTO_FUNCTION(proto, m12);
    ADD_PROTO_FUNCTION(proto, m21);
    ADD_PROTO_FUNCTION(proto, m22);
    ADD_PROTO_FUNCTION(proto, det);
    ADD_PROTO_FUNCTION(proto, dx);
    ADD_PROTO_FUNCTION(proto, dy);
    ADD_PROTO_FUNCTION(proto, inverted);
    ADD_PROTO_FUNCTION(proto, isIdentity);
    ADD_PROTO_FUNCTION(proto, isInvertible);
    ADD_PROTO_FUNCTION(proto, map);
    ADD_PROTO_FUNCTION(proto, mapRect);
    ADD_PROTO_FUNCTION(proto, mapToPolygon);
    ADD_PROTO_FUNCTION(proto, reset);
    ADD_PROTO_FUNCTION(proto, rotate);
    ADD_PROTO_FUNCTION(proto, scale);
    ADD_PROTO_FUNCTION(proto, setMatrix);
    ADD_PROTO_FUNCTION(proto, shear);
    ADD_PROTO_FUNCTION(proto, translate);
    ADD_PROTO_FUNCTION(proto, toString);

    eng->setDefaultPrototype(qMetaTypeId<QMatrix>(), proto);
    eng->setDefaultPrototype(qMetaTypeId<QMatrix*>(), proto);

    return eng->newFunction(ctor, proto);
}
