#include <QtScript/QScriptValue>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptContext>
#include <QtGui/QConicalGradient>
#include "../global.h"

Q_DECLARE_METATYPE(QGradient)
Q_DECLARE_METATYPE(QConicalGradient)
Q_DECLARE_METATYPE(QConicalGradient*)

static inline QScriptValue newConicalGradient(QScriptEngine *eng, const QConicalGradient &gradient)
{
    return eng->newVariant(qVariantFromValue(gradient));
}

/////////////////////////////////////////////////////////////

static QScriptValue ctor(QScriptContext *ctx, QScriptEngine *eng)
{
    if (ctx->argumentCount() == 0) {
        return newConicalGradient(eng, QConicalGradient());
    } else if (ctx->argumentCount() == 2) {
        QPointF center = qscriptvalue_cast<QPointF>(ctx->argument(0));
        qreal angle = qscriptvalue_cast<qreal>(ctx->argument(1));
        return newConicalGradient(eng, QConicalGradient(center, angle));
    } else if (ctx->argumentCount() == 3) {
        qreal cx = qscriptvalue_cast<qreal>(ctx->argument(0));
        qreal cy = qscriptvalue_cast<qreal>(ctx->argument(1));
        qreal angle = qscriptvalue_cast<qreal>(ctx->argument(2));
        return newConicalGradient(eng, QConicalGradient(cx, cy, angle));
    }
    return ctx->throwError("QConicalGradient: invalid number of arguments");
}

static QScriptValue angle(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(ConicalGradient, angle);
    return QScriptValue(eng, self->angle());
}

static QScriptValue center(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(ConicalGradient, center);
    return eng->toScriptValue(self->center());
}

static QScriptValue setAngle(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(ConicalGradient, setAngle);
    self->setAngle(qscriptvalue_cast<qreal>(ctx->argument(0)));
    return eng->undefinedValue();
}

static QScriptValue setCenter(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(ConicalGradient, setAngle);
    if (ctx->argumentCount() == 2) {
        self->setCenter(qscriptvalue_cast<qreal>(ctx->argument(0)),
                        qscriptvalue_cast<qreal>(ctx->argument(1)));
    } else {
        self->setCenter(qscriptvalue_cast<QPointF>(ctx->argument(0)));
    }
    return eng->undefinedValue();
}

static QScriptValue toString(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(ConicalGradient, toString);
    return QScriptValue(eng, "QConicalGradient");
}

/////////////////////////////////////////////////////////////

QScriptValue constructConicalGradientClass(QScriptEngine *eng)
{
    QScriptValue proto = newConicalGradient(eng, QConicalGradient());
    proto.setPrototype(eng->defaultPrototype(qMetaTypeId<QGradient>()));
    ADD_PROTO_FUNCTION(proto, angle);
    ADD_PROTO_FUNCTION(proto, center);
    ADD_PROTO_FUNCTION(proto, setAngle);
    ADD_PROTO_FUNCTION(proto, setCenter);
    ADD_PROTO_FUNCTION(proto, toString);

    eng->setDefaultPrototype(qMetaTypeId<QConicalGradient>(), proto);
    eng->setDefaultPrototype(qMetaTypeId<QConicalGradient*>(), proto);

    return eng->newFunction(ctor, proto);
}
