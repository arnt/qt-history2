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
    DECLARE_SELF(QGradient, setColorAt);
    self->setColorAt(qscriptvalue_cast<qreal>(ctx->argument(0)),
                     qscriptvalue_cast<QColor>(ctx->argument(1)));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setSpread(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGradient, setSpread);
    self->setSpread(static_cast<QGradient::Spread>(ctx->argument(0).toInt32()));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setStops(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGradient, setStops);
    self->setStops(qscriptvalue_cast<QGradientStops>(ctx->argument(0)));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue spread(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGradient, spread);
    return QScriptValue(eng, static_cast<int>(self->spread()));
}

/////////////////////////////////////////////////////////////

static QScriptValue stops(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGradient, stops);
    return eng->toScriptValue(self->stops());
}

/////////////////////////////////////////////////////////////

static QScriptValue type(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGradient, type);
    return QScriptValue(eng, static_cast<int>(self->type()));
}

/////////////////////////////////////////////////////////////

static QScriptValue toString(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGradient, toString);
    return QScriptValue(eng, "QGradient");
}

/////////////////////////////////////////////////////////////

QScriptValue constructGradientClass(QScriptEngine *eng)
{
    QScriptValue proto = eng->newVariant(qVariantFromValue(QGradient()));
    ADD_METHOD(proto, setColorAt);
    ADD_METHOD(proto, setSpread);
    ADD_METHOD(proto, setStops);
    ADD_METHOD(proto, spread);
    ADD_METHOD(proto, stops);
    ADD_METHOD(proto, toString);
    ADD_METHOD(proto, type);

    eng->setDefaultPrototype(qMetaTypeId<QGradient>(), proto);
    eng->setDefaultPrototype(qMetaTypeId<QGradient*>(), proto);

    return eng->newFunction(ctor, proto);
}
