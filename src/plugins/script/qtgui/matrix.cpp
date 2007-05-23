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
    DECLARE_SELF(QMatrix, m11);
    return QScriptValue(eng, self->m11());
}

/////////////////////////////////////////////////////////////

static QScriptValue m12(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QMatrix, m12);
    return QScriptValue(eng, self->m12());
}

/////////////////////////////////////////////////////////////

static QScriptValue m21(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QMatrix, m21);
    return QScriptValue(eng, self->m21());
}

/////////////////////////////////////////////////////////////

static QScriptValue m22(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QMatrix, m22);
    return QScriptValue(eng, self->m22());
}

/////////////////////////////////////////////////////////////

static QScriptValue det(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QMatrix, det);
    return QScriptValue(eng, self->det());
}

/////////////////////////////////////////////////////////////

static QScriptValue dx(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QMatrix, dx);
    return QScriptValue(eng, self->dx());
}

/////////////////////////////////////////////////////////////

static QScriptValue dy(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QMatrix, dy);
    return QScriptValue(eng, self->dy());
}

/////////////////////////////////////////////////////////////

static QScriptValue inverted(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QMatrix, inverted);
    return newMatrix(eng, self->inverted());
}

/////////////////////////////////////////////////////////////

static QScriptValue isIdentity(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QMatrix, isIdentity);
    return QScriptValue(eng, self->isIdentity());
}

/////////////////////////////////////////////////////////////

static QScriptValue isInvertible(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QMatrix, isInvertible);
    return QScriptValue(eng, self->isInvertible());
}

/////////////////////////////////////////////////////////////

static QScriptValue map(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QMatrix, map);
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
    DECLARE_SELF(QMatrix, mapRect);
    return eng->toScriptValue(self->mapRect(qscriptvalue_cast<QRectF>(ctx->argument(0))));
}

/////////////////////////////////////////////////////////////

static QScriptValue mapToPolygon(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QMatrix, mapToPolygon);
    return eng->toScriptValue(self->mapToPolygon(qscriptvalue_cast<QRect>(ctx->argument(0))));
}

/////////////////////////////////////////////////////////////

static QScriptValue reset(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QMatrix, reset);
    self->reset();
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue rotate(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(QMatrix, rotate);
    self->rotate(ctx->argument(0).toNumber());
    return ctx->thisObject();
}

/////////////////////////////////////////////////////////////

static QScriptValue scale(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(QMatrix, scale);
    self->scale(ctx->argument(0).toNumber(),
                ctx->argument(1).toNumber());
    return ctx->thisObject();
}

/////////////////////////////////////////////////////////////

static QScriptValue setMatrix(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QMatrix, setMatrix);
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
    DECLARE_SELF(QMatrix, shear);
    self->shear(ctx->argument(0).toNumber(),
                ctx->argument(1).toNumber());
    return ctx->thisObject();
}

/////////////////////////////////////////////////////////////

static QScriptValue translate(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(QMatrix, translate);
    self->translate(ctx->argument(0).toNumber(),
                    ctx->argument(1).toNumber());
    return ctx->thisObject();
}

/////////////////////////////////////////////////////////////

static QScriptValue toString(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QMatrix, toString);
    return QScriptValue(eng, "QMatrix");
}

/////////////////////////////////////////////////////////////

QScriptValue constructMatrixClass(QScriptEngine *eng)
{
    QScriptValue proto = newMatrix(eng, QMatrix());
    ADD_METHOD(proto, m11);
    ADD_METHOD(proto, m12);
    ADD_METHOD(proto, m21);
    ADD_METHOD(proto, m22);
    ADD_METHOD(proto, det);
    ADD_METHOD(proto, dx);
    ADD_METHOD(proto, dy);
    ADD_METHOD(proto, inverted);
    ADD_METHOD(proto, isIdentity);
    ADD_METHOD(proto, isInvertible);
    ADD_METHOD(proto, map);
    ADD_METHOD(proto, mapRect);
    ADD_METHOD(proto, mapToPolygon);
    ADD_METHOD(proto, reset);
    ADD_METHOD(proto, rotate);
    ADD_METHOD(proto, scale);
    ADD_METHOD(proto, setMatrix);
    ADD_METHOD(proto, shear);
    ADD_METHOD(proto, translate);
    ADD_METHOD(proto, toString);

    eng->setDefaultPrototype(qMetaTypeId<QMatrix>(), proto);
    eng->setDefaultPrototype(qMetaTypeId<QMatrix*>(), proto);

    return eng->newFunction(ctor, proto);
}
