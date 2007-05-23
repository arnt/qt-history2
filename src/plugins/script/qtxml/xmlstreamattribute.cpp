#include <QtScript/QScriptValue>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptContext>
#include <QtXml/QXmlStreamAttribute>
#include "../global.h"

Q_DECLARE_METATYPE(QXmlStreamAttribute)
Q_DECLARE_METATYPE(QXmlStreamAttribute*)
Q_DECLARE_METATYPE(QStringRef)

static inline QScriptValue newXmlStreamAttribute(QScriptEngine *eng, const QXmlStreamAttribute &attr)
{
    return eng->newVariant(qVariantFromValue(attr));
}

static QScriptValue ctor(QScriptContext *ctx, QScriptEngine *eng)
{
    if (ctx->argumentCount() == 0)
        return newXmlStreamAttribute(eng, QXmlStreamAttribute());
    else if (ctx->argumentCount() >= 2) {
        QString arg0 = ctx->argument(0).toString();
        QString arg1 = ctx->argument(1).toString();
        if (ctx->argumentCount() == 2)
            return newXmlStreamAttribute(eng, QXmlStreamAttribute(arg0, arg1));
        QString arg2 = ctx->argument(2).toString();
        return newXmlStreamAttribute(eng, QXmlStreamAttribute(arg0, arg1, arg2));
    }
    QScriptValue arg0 = ctx->argument(0);
    if (QXmlStreamAttribute *other = qscriptvalue_cast<QXmlStreamAttribute*>(arg0))
        return newXmlStreamAttribute(eng, QXmlStreamAttribute(*other));
    return ctx->throwError(QLatin1String("QXmlStreamAttribute: invalid argument"));
}

static QScriptValue equals(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QXmlStreamAttribute, equals);
    if (QXmlStreamAttribute *other = qscriptvalue_cast<QXmlStreamAttribute*>(ctx->argument(0)))
        return QScriptValue(eng, *self == *other);
    return QScriptValue(eng, false);
}

static QScriptValue isDefault(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QXmlStreamAttribute, isDefault);
    return QScriptValue(eng, self->isDefault());
}

static QScriptValue name(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QXmlStreamAttribute, name);
    return eng->toScriptValue(self->name());
}

static QScriptValue namespaceUri(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QXmlStreamAttribute, namespaceUri);
    return eng->toScriptValue(self->namespaceUri());
}

static QScriptValue qualifiedName(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QXmlStreamAttribute, qualifiedName);
    return eng->toScriptValue(self->qualifiedName());
}

static QScriptValue value(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QXmlStreamAttribute, value);
    return eng->toScriptValue(self->value());
}

static QScriptValue toString(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QXmlStreamAttribute, toString);
    return QScriptValue(eng, QLatin1String("QXmlStreamAttribute"));
}

QScriptValue constructXmlStreamAttributeClass(QScriptEngine *eng)
{
    QScriptValue proto = newXmlStreamAttribute(eng, QXmlStreamAttribute());
    ADD_METHOD(proto, equals);
    ADD_METHOD(proto, isDefault);
    ADD_METHOD(proto, name);
    ADD_METHOD(proto, namespaceUri);
    ADD_METHOD(proto, qualifiedName);
    ADD_METHOD(proto, value);
    ADD_METHOD(proto, toString);

    eng->setDefaultPrototype(qMetaTypeId<QXmlStreamAttribute>(), proto);
    eng->setDefaultPrototype(qMetaTypeId<QXmlStreamAttribute*>(), proto);

    QScriptValue ctorFun = eng->newFunction(ctor, proto);
    return ctorFun;
}
