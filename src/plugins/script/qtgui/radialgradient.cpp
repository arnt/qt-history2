#include <QtScript/QScriptValue>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptContext>
#include <QtGui/QRadialGradient>
#include "../global.h"

Q_DECLARE_METATYPE(QGradient)
Q_DECLARE_METATYPE(QRadialGradient)
Q_DECLARE_METATYPE(QRadialGradient*)

static inline QScriptValue newRadialGradient(QScriptEngine *eng, const QRadialGradient &gradient)
{
    return eng->newVariant(qVariantFromValue(gradient));
}

/////////////////////////////////////////////////////////////

static QScriptValue ctor(QScriptContext *ctx, QScriptEngine *eng)
{
    if (ctx->argumentCount() == 0) {
        return newRadialGradient(eng, QRadialGradient());
    } else if (ctx->argumentCount() == 2) {
        return newRadialGradient(
            eng, QRadialGradient(qscriptvalue_cast<QPointF>(ctx->argument(0)),
                                 ctx->argument(1).toNumber()));
    } else if (ctx->argumentCount() == 5) {
        return newRadialGradient(
            eng, QRadialGradient(ctx->argument(0).toNumber(),
                                 ctx->argument(1).toNumber(),
                                 ctx->argument(2).toNumber(),
                                 ctx->argument(3).toNumber(),
                                 ctx->argument(4).toNumber()));
    } else if (ctx->argumentCount() == 3) {
        if (ctx->argument(0).isObject()) {
            return newRadialGradient(
                eng, QRadialGradient(qscriptvalue_cast<QPointF>(ctx->argument(0)),
                                     ctx->argument(1).toNumber(),
                                     qscriptvalue_cast<QPointF>(ctx->argument(2))));
        } else {
            return newRadialGradient(
                eng, QRadialGradient(ctx->argument(0).toNumber(),
                                     ctx->argument(1).toNumber(),
                                     ctx->argument(2).toNumber()));
        }
    }
    return ctx->throwError("QRadialGradient: invalid number of arguments");
}

static QScriptValue center(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(RadialGradient, center);
    return eng->toScriptValue(self->center());
}

static QScriptValue focalPoint(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(RadialGradient, focalPoint);
    return eng->toScriptValue(self->focalPoint());
}

static QScriptValue radius(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(RadialGradient, radius);
    return QScriptValue(eng, self->radius());
}

static QScriptValue setCenter(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(RadialGradient, setCenter);
    if (ctx->argumentCount() > 1) {
        self->setCenter(ctx->argument(0).toNumber(),
                        ctx->argument(1).toNumber());
    } else {
        self->setCenter(qscriptvalue_cast<QPointF>(ctx->argument(0)));
    }
    return eng->undefinedValue();
}

static QScriptValue setFocalPoint(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(RadialGradient, setFocalPoint);
    if (ctx->argumentCount() > 1) {
        self->setFocalPoint(ctx->argument(0).toNumber(),
                            ctx->argument(1).toNumber());
    } else {
        self->setFocalPoint(qscriptvalue_cast<QPointF>(ctx->argument(0)));
    }
    return eng->undefinedValue();
}

static QScriptValue setRadius(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(RadialGradient, setRadius);
    self->setRadius(ctx->argument(0).toNumber());
    return eng->undefinedValue();
}

static QScriptValue toString(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(RadialGradient, toString);
    return QScriptValue(eng, "QRadialGradient");
}

/////////////////////////////////////////////////////////////

QScriptValue constructRadialGradientClass(QScriptEngine *eng)
{
    QScriptValue proto = newRadialGradient(eng, QRadialGradient());
    proto.setPrototype(eng->defaultPrototype(qMetaTypeId<QGradient>()));
    ADD_PROTO_FUNCTION(proto, center);
    ADD_PROTO_FUNCTION(proto, focalPoint);
    ADD_PROTO_FUNCTION(proto, radius);
    ADD_PROTO_FUNCTION(proto, setCenter);
    ADD_PROTO_FUNCTION(proto, setFocalPoint);
    ADD_PROTO_FUNCTION(proto, setRadius);
    ADD_PROTO_FUNCTION(proto, toString);

    eng->setDefaultPrototype(qMetaTypeId<QRadialGradient>(), proto);
    eng->setDefaultPrototype(qMetaTypeId<QRadialGradient*>(), proto);

    return eng->newFunction(ctor, proto);
}
