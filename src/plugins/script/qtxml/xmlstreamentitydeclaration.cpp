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
    DECLARE_SELF(QXmlStreamEntityDeclaration, equals);
    if (QXmlStreamEntityDeclaration *other = qscriptvalue_cast<QXmlStreamEntityDeclaration*>(ctx->argument(0)))
        return QScriptValue(eng, *self == *other);
    return QScriptValue(eng, false);
}

static QScriptValue name(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QXmlStreamEntityDeclaration, name);
    return eng->toScriptValue(self->name());
}

static QScriptValue notationName(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QXmlStreamEntityDeclaration, notationName);
    return eng->toScriptValue(self->notationName());
}

static QScriptValue publicId(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QXmlStreamEntityDeclaration, publicId);
    return eng->toScriptValue(self->publicId());
}

static QScriptValue systemId(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QXmlStreamEntityDeclaration, systemId);
    return eng->toScriptValue(self->systemId());
}

static QScriptValue value(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QXmlStreamEntityDeclaration, value);
    return eng->toScriptValue(self->value());
}

static QScriptValue toString(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QXmlStreamEntityDeclaration, toString);
    return QScriptValue(eng, QLatin1String("QXmlStreamEntityDeclaration"));
}

QScriptValue constructXmlStreamEntityDeclarationClass(QScriptEngine *eng)
{
    QScriptValue proto = newXmlStreamEntityDeclaration(eng, QXmlStreamEntityDeclaration());
    ADD_METHOD(proto, equals);
    ADD_METHOD(proto, name);
    ADD_METHOD(proto, notationName);
    ADD_METHOD(proto, publicId);
    ADD_METHOD(proto, systemId);
    ADD_METHOD(proto, value);
    ADD_METHOD(proto, toString);

    eng->setDefaultPrototype(qMetaTypeId<QXmlStreamEntityDeclaration>(), proto);
    eng->setDefaultPrototype(qMetaTypeId<QXmlStreamEntityDeclaration*>(), proto);

    QScriptValue ctorFun = eng->newFunction(ctor, proto);
    return ctorFun;
}
