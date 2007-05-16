#include <QtScript/QScriptValue>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptContext>
#include <QtXml/QXmlStreamEntityDeclaration>
#include "../global.h"

Q_DECLARE_METATYPE(QXmlStreamEntityDeclaration)
Q_DECLARE_METATYPE(QXmlStreamEntityDeclaration*)
Q_DECLARE_METATYPE(QStringRef)

static inline QScriptValue newXmlStreamEntityDeclaration(QScriptEngine *eng, const QXmlStreamEntityDeclaration &decl)
{
    return eng->newVariant(qVariantFromValue(decl));
}

static QScriptValue ctor(QScriptContext *ctx, QScriptEngine *eng)
{
    if (ctx->argumentCount() == 0)
        return newXmlStreamEntityDeclaration(eng, QXmlStreamEntityDeclaration());
    QScriptValue arg0 = ctx->argument(0);
    if (QXmlStreamEntityDeclaration *other = qscriptvalue_cast<QXmlStreamEntityDeclaration*>(arg0))
        return newXmlStreamEntityDeclaration(eng, QXmlStreamEntityDeclaration(*other));
    return ctx->throwError(QLatin1String("QXmlStreamEntityDeclaration: invalid argument"));
}

static QScriptValue equals(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(XmlStreamEntityDeclaration, equals);
    if (QXmlStreamEntityDeclaration *other = qscriptvalue_cast<QXmlStreamEntityDeclaration*>(ctx->argument(0)))
        return QScriptValue(eng, *self == *other);
    return QScriptValue(eng, false);
}

static QScriptValue name(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(XmlStreamEntityDeclaration, name);
    return eng->toScriptValue(self->name());
}

static QScriptValue notationName(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(XmlStreamEntityDeclaration, notationName);
    return eng->toScriptValue(self->notationName());
}

static QScriptValue publicId(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(XmlStreamEntityDeclaration, publicId);
    return eng->toScriptValue(self->publicId());
}

static QScriptValue systemId(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(XmlStreamEntityDeclaration, systemId);
    return eng->toScriptValue(self->systemId());
}

static QScriptValue value(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(XmlStreamEntityDeclaration, value);
    return eng->toScriptValue(self->value());
}

static QScriptValue toString(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(XmlStreamEntityDeclaration, toString);
    return QScriptValue(eng, QLatin1String("QXmlStreamEntityDeclaration"));
}

QScriptValue constructXmlStreamEntityDeclarationClass(QScriptEngine *eng)
{
    QScriptValue proto = newXmlStreamEntityDeclaration(eng, QXmlStreamEntityDeclaration());
    ADD_PROTO_FUNCTION(proto, equals);
    ADD_PROTO_FUNCTION(proto, name);
    ADD_PROTO_FUNCTION(proto, notationName);
    ADD_PROTO_FUNCTION(proto, publicId);
    ADD_PROTO_FUNCTION(proto, systemId);
    ADD_PROTO_FUNCTION(proto, value);
    ADD_PROTO_FUNCTION(proto, toString);

    eng->setDefaultPrototype(qMetaTypeId<QXmlStreamEntityDeclaration>(), proto);
    eng->setDefaultPrototype(qMetaTypeId<QXmlStreamEntityDeclaration*>(), proto);

    QScriptValue ctorFun = eng->newFunction(ctor, proto);
    return ctorFun;
}
