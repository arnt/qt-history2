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
    DECLARE_SELF(QStringRef, appendTo);
    return ctx->throwError(QLatin1String("not implemented"));
}

static QScriptValue at(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QStringRef, at);
    return eng->toScriptValue(self->at(ctx->argument(0).toInt32()));
}

static QScriptValue clear(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QStringRef, clear);
    self->clear();
    return eng->undefinedValue();
}

static QScriptValue constData(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(QStringRef, constData);
    return ctx->throwError(QLatin1String("not implemented"));
}

static QScriptValue count(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QStringRef, count);
    return QScriptValue(eng, self->count());
}

static QScriptValue data(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(QStringRef, data);
    return ctx->throwError(QLatin1String("not implemented"));
}

static QScriptValue equals(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QStringRef, equals);
    if (QStringRef *other = qscriptvalue_cast<QStringRef*>(ctx->argument(0)))
        return QScriptValue(eng, *self == *other);
    return QScriptValue(eng, false);
}

static QScriptValue isEmpty(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QStringRef, isEmpty);
    return QScriptValue(eng, self->isEmpty());
}

static QScriptValue isNull(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QStringRef, isNull);
    return QScriptValue(eng, self->isNull());
}

static QScriptValue length(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QStringRef, length);
    return QScriptValue(eng, self->length());
}

static QScriptValue position(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QStringRef, position);
    return QScriptValue(eng, self->position());
}

static QScriptValue size(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QStringRef, size);
    return QScriptValue(eng, self->size());
}

static QScriptValue string(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(QStringRef, string);
    return ctx->throwError(QLatin1String("not implemented"));
}

static QScriptValue unicode(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(QStringRef, unicode);
    return ctx->throwError(QLatin1String("not implemented"));
}

static QScriptValue toString(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QStringRef, toString);
    return QScriptValue(eng, self->toString());
}

QScriptValue constructStringRefClass(QScriptEngine *eng)
{
    QScriptValue proto = newStringRef(eng, QStringRef());
    ADD_METHOD(proto, appendTo);
    ADD_METHOD(proto, at);
    ADD_METHOD(proto, clear);
    ADD_METHOD(proto, constData);
    ADD_METHOD(proto, count);
    ADD_METHOD(proto, data);
    ADD_METHOD(proto, equals);
    ADD_METHOD(proto, isEmpty);
    ADD_METHOD(proto, isNull);
    ADD_METHOD(proto, length);
    ADD_METHOD(proto, position);
    ADD_METHOD(proto, size);
    ADD_METHOD(proto, string);
    ADD_METHOD(proto, toString);
    ADD_METHOD(proto, unicode);
    ADD_METHOD(proto, toString);

    eng->setDefaultPrototype(qMetaTypeId<QStringRef>(), proto);
    eng->setDefaultPrototype(qMetaTypeId<QStringRef*>(), proto);

    return eng->newFunction(ctor, proto);
}
