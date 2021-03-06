#include <QtScript/QScriptValue>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptContext>
#include <QtGui/QLinearGradient>
#include "../global.h"

Q_DECLARE_METATYPE(QGradient)
Q_DECLARE_METATYPE(QLinearGradient)
Q_DECLARE_METATYPE(QLinearGradient*)

QT_BEGIN_NAMESPACE

static inline QScriptValue newLinearGradient(QScriptEngine *eng, const QLinearGradient &gradient)
{
    return eng->newVariant(qVariantFromValue(gradient));
}

/////////////////////////////////////////////////////////////

static QScriptValue ctor(QScriptContext *ctx, QScriptEngine *eng)
{
    if (ctx->argumentCount() == 0) {
        return newLinearGradient(eng, QLinearGradient());
    } else if (ctx->argumentCount() == 2) {
        return newLinearGradient(
            eng, QLinearGradient(qscriptvalue_cast<QPointF>(ctx->argument(0)),
                                 qscriptvalue_cast<QPointF>(ctx->argument(1))));
    } else if (ctx->argumentCount() == 4) {
        return newLinearGradient(
            eng, QLinearGradient(ctx->argument(0).toNumber(),
                                 ctx->argument(1).toNumber(),
                                 ctx->argument(2).toNumber(),
                                 ctx->argument(3).toNumber()));
    }
    return ctx->throwError("QLinearGradient: invalid number of arguments");
}

static QScriptValue finalStop(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QLinearGradient, finalStop);
    return qScriptValueFromValue(eng, self->finalStop());
}

static QScriptValue setFinalStop(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QLinearGradient, setFinalStop);
    if (ctx->argumentCount() > 1) {
        self->setFinalStop(ctx->argument(0).toNumber(),
                           ctx->argument(1).toNumber());
    } else {
        self->setFinalStop(qscriptvalue_cast<QPointF>(ctx->argument(0)));
    }
    return eng->undefinedValue();
}

static QScriptValue setStart(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QLinearGradient, setStart);
    if (ctx->argumentCount() > 1) {
        self->setStart(ctx->argument(0).toNumber(),
                       ctx->argument(1).toNumber());
    } else {
        self->setStart(qscriptvalue_cast<QPointF>(ctx->argument(0)));
    }
    return eng->undefinedValue();
}

static QScriptValue start(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QLinearGradient, start);
    return qScriptValueFromValue(eng, self->start());
}

static QScriptValue toString(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QLinearGradient, toString);
    return QScriptValue(eng, "QLinearGradient");
}

/////////////////////////////////////////////////////////////

QScriptValue constructLinearGradientClass(QScriptEngine *eng)
{
    QScriptValue proto = newLinearGradient(eng, QLinearGradient());
    proto.setPrototype(eng->defaultPrototype(qMetaTypeId<QGradient>()));
    ADD_METHOD(proto, finalStop);
    ADD_METHOD(proto, setFinalStop);
    ADD_METHOD(proto, setStart);
    ADD_METHOD(proto, start);
    ADD_METHOD(proto, toString);

    eng->setDefaultPrototype(qMetaTypeId<QLinearGradient>(), proto);
    eng->setDefaultPrototype(qMetaTypeId<QLinearGradient*>(), proto);

    return eng->newFunction(ctor, proto);
}

QT_END_NAMESPACE
