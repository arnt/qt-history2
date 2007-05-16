#include <QtScript/QScriptValue>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptContext>
#include <QtCore/QStringRef>
#include "../global.h"

Q_DECLARE_METATYPE(QStringRef)
Q_DECLARE_METATYPE(QStringRef*)

static inline QScriptValue newStringRef(QScriptEngine *eng, const QStringRef &ref)
{
    return eng->newVariant(qVariantFromValue(ref));
}

static QScriptValue ctor(QScriptContext *ctx, QScriptEngine *eng)
{
    if (ctx->argumentCount() == 0)
        return newStringRef(eng, QStringRef());
    if (QStringRef *other = qscriptvalue_cast<QStringRef*>(ctx->argument(0)))
        return newStringRef(eng, *other);
    return ctx->throwError(QLatin1String("QStringRef"));
}

static QScriptValue appendTo(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(StringRef, appendTo);
    return ctx->throwError(QLatin1String("not implemented"));
}

static QScriptValue at(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(StringRef, at);
    return eng->toScriptValue(self->at(ctx->argument(0).toInt32()));
}

static QScriptValue clear(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(StringRef, clear);
    self->clear();
    return eng->undefinedValue();
}

static QScriptValue constData(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(StringRef, constData);
    return ctx->throwError(QLatin1String("not implemented"));
}

static QScriptValue count(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(StringRef, count);
    return QScriptValue(eng, self->count());
}

static QScriptValue data(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(StringRef, data);
    return ctx->throwError(QLatin1String("not implemented"));
}

static QScriptValue equals(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(StringRef, equals);
    if (QStringRef *other = qscriptvalue_cast<QStringRef*>(ctx->argument(0)))
        return QScriptValue(eng, *self == *other);
    return QScriptValue(eng, false);
}

static QScriptValue isEmpty(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(StringRef, isEmpty);
    return QScriptValue(eng, self->isEmpty());
}

static QScriptValue isNull(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(StringRef, isNull);
    return QScriptValue(eng, self->isNull());
}

static QScriptValue length(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(StringRef, length);
    return QScriptValue(eng, self->length());
}

static QScriptValue position(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(StringRef, position);
    return QScriptValue(eng, self->position());
}

static QScriptValue size(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(StringRef, size);
    return QScriptValue(eng, self->size());
}

static QScriptValue string(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(StringRef, string);
    return ctx->throwError(QLatin1String("not implemented"));
}

static QScriptValue unicode(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(StringRef, unicode);
    return ctx->throwError(QLatin1String("not implemented"));
}

static QScriptValue toString(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(StringRef, toString);
    return QScriptValue(eng, self->toString());
}

QScriptValue constructStringRefClass(QScriptEngine *eng)
{
    QScriptValue proto = newStringRef(eng, QStringRef());
    ADD_PROTO_FUNCTION(proto, appendTo);
    ADD_PROTO_FUNCTION(proto, at);
    ADD_PROTO_FUNCTION(proto, clear);
    ADD_PROTO_FUNCTION(proto, constData);
    ADD_PROTO_FUNCTION(proto, count);
    ADD_PROTO_FUNCTION(proto, data);
    ADD_PROTO_FUNCTION(proto, equals);
    ADD_PROTO_FUNCTION(proto, isEmpty);
    ADD_PROTO_FUNCTION(proto, isNull);
    ADD_PROTO_FUNCTION(proto, length);
    ADD_PROTO_FUNCTION(proto, position);
    ADD_PROTO_FUNCTION(proto, size);
    ADD_PROTO_FUNCTION(proto, string);
    ADD_PROTO_FUNCTION(proto, toString);
    ADD_PROTO_FUNCTION(proto, unicode);
    ADD_PROTO_FUNCTION(proto, toString);

    eng->setDefaultPrototype(qMetaTypeId<QStringRef>(), proto);
    eng->setDefaultPrototype(qMetaTypeId<QStringRef*>(), proto);

    return eng->newFunction(ctor, proto);
}
