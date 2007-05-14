#include <QtScript/QScriptValue>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptContext>
#include <QtGui/QGradient>
#include "../global.h"

Q_DECLARE_METATYPE(QGradient)
Q_DECLARE_METATYPE(QGradient*)
Q_DECLARE_METATYPE(QGradientStops)

/////////////////////////////////////////////////////////////

static QScriptValue ctor(QScriptContext *, QScriptEngine *eng)
{
    return eng->newVariant(qVariantFromValue(QGradient()));
}

/////////////////////////////////////////////////////////////

static QScriptValue setColorAt(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Gradient, setColorAt);
    self->setColorAt(qscriptvalue_cast<qreal>(ctx->argument(0)),
                     qscriptvalue_cast<QColor>(ctx->argument(1)));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setSpread(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Gradient, setSpread);
    self->setSpread(static_cast<QGradient::Spread>(ctx->argument(0).toInt32()));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setStops(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Gradient, setStops);
    self->setStops(qscriptvalue_cast<QGradientStops>(ctx->argument(0)));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue spread(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Gradient, spread);
    return QScriptValue(eng, static_cast<int>(self->spread()));
}

/////////////////////////////////////////////////////////////

static QScriptValue stops(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Gradient, stops);
    return eng->toScriptValue(self->stops());
}

/////////////////////////////////////////////////////////////

static QScriptValue type(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Gradient, type);
    return QScriptValue(eng, static_cast<int>(self->type()));
}

/////////////////////////////////////////////////////////////

static QScriptValue toString(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Gradient, toString);
    return QScriptValue(eng, "QGradient");
}

/////////////////////////////////////////////////////////////

QScriptValue constructGradientClass(QScriptEngine *eng)
{
    QScriptValue proto = eng->newVariant(qVariantFromValue(QGradient()));
    ADD_PROTO_FUNCTION(proto, setColorAt);
    ADD_PROTO_FUNCTION(proto, setSpread);
    ADD_PROTO_FUNCTION(proto, setStops);
    ADD_PROTO_FUNCTION(proto, spread);
    ADD_PROTO_FUNCTION(proto, stops);
    ADD_PROTO_FUNCTION(proto, toString);
    ADD_PROTO_FUNCTION(proto, type);

    eng->setDefaultPrototype(qMetaTypeId<QGradient>(), proto);
    eng->setDefaultPrototype(qMetaTypeId<QGradient*>(), proto);

    return eng->newFunction(ctor, proto);
}
