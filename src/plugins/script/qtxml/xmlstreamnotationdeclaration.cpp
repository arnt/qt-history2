#include <QtScript/QScriptValue>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptContext>
#include <QtXml/QXmlStreamNotationDeclaration>
#include "../global.h"

Q_DECLARE_METATYPE(QXmlStreamNotationDeclaration)
Q_DECLARE_METATYPE(QXmlStreamNotationDeclaration*)
Q_DECLARE_METATYPE(QStringRef)

static inline QScriptValue newXmlStreamNotationDeclaration(QScriptEngine *eng, const QXmlStreamNotationDeclaration &decl)
{
    return eng->newVariant(qVariantFromValue(decl));
}

static QScriptValue ctor(QScriptContext *ctx, QScriptEngine *eng)
{
    if (ctx->argumentCount() == 0)
        return newXmlStreamNotationDeclaration(eng, QXmlStreamNotationDeclaration());
    QScriptValue arg0 = ctx->argument(0);
    if (QXmlStreamNotationDeclaration *other = qscriptvalue_cast<QXmlStreamNotationDeclaration*>(arg0))
        return newXmlStreamNotationDeclaration(eng, QXmlStreamNotationDeclaration(*other));
    return ctx->throwError(QLatin1String("QXmlStreamNotationDeclaration: invalid argument"));
}

static QScriptValue equals(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(XmlStreamNotationDeclaration, equals);
    if (QXmlStreamNotationDeclaration *other = qscriptvalue_cast<QXmlStreamNotationDeclaration*>(ctx->argument(0)))
        return QScriptValue(eng, *self == *other);
    return QScriptValue(eng, false);
}

static QScriptValue name(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(XmlStreamNotationDeclaration, name);
    return eng->toScriptValue(self->name());
}

static QScriptValue publicId(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(XmlStreamNotationDeclaration, publicId);
    return eng->toScriptValue(self->publicId());
}

static QScriptValue systemId(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(XmlStreamNotationDeclaration, systemId);
    return eng->toScriptValue(self->systemId());
}

static QScriptValue toString(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(XmlStreamNotationDeclaration, toString);
    return QScriptValue(eng, QLatin1String("QXmlStreamNotationDeclaration"));
}

QScriptValue constructXmlStreamNotationDeclarationClass(QScriptEngine *eng)
{
    QScriptValue proto = newXmlStreamNotationDeclaration(eng, QXmlStreamNotationDeclaration());
    ADD_PROTO_FUNCTION(proto, equals);
    ADD_PROTO_FUNCTION(proto, name);
    ADD_PROTO_FUNCTION(proto, publicId);
    ADD_PROTO_FUNCTION(proto, systemId);
    ADD_PROTO_FUNCTION(proto, toString);

    eng->setDefaultPrototype(qMetaTypeId<QXmlStreamNotationDeclaration>(), proto);
    eng->setDefaultPrototype(qMetaTypeId<QXmlStreamNotationDeclaration*>(), proto);

    QScriptValue ctorFun = eng->newFunction(ctor, proto);
    return ctorFun;
}
