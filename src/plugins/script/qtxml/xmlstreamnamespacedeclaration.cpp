#include <QtScript/QScriptValue>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptContext>
#include <QtXml/QXmlStreamNamespaceDeclaration>
#include "../global.h"

Q_DECLARE_METATYPE(QXmlStreamNamespaceDeclaration)
Q_DECLARE_METATYPE(QXmlStreamNamespaceDeclaration*)
Q_DECLARE_METATYPE(QStringRef)

static inline QScriptValue newXmlStreamNamespaceDeclaration(QScriptEngine *eng, const QXmlStreamNamespaceDeclaration &decl)
{
    return eng->newVariant(qVariantFromValue(decl));
}

static QScriptValue ctor(QScriptContext *ctx, QScriptEngine *eng)
{
    if (ctx->argumentCount() == 0)
        return newXmlStreamNamespaceDeclaration(eng, QXmlStreamNamespaceDeclaration());
    QScriptValue arg0 = ctx->argument(0);
    if (QXmlStreamNamespaceDeclaration *other = qscriptvalue_cast<QXmlStreamNamespaceDeclaration*>(arg0))
        return newXmlStreamNamespaceDeclaration(eng, QXmlStreamNamespaceDeclaration(*other));
    return ctx->throwError(QLatin1String("QXmlStreamNamespaceDeclaration: invalid argument"));
}

static QScriptValue equals(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QXmlStreamNamespaceDeclaration, equals);
    if (QXmlStreamNamespaceDeclaration *other = qscriptvalue_cast<QXmlStreamNamespaceDeclaration*>(ctx->argument(0)))
        return QScriptValue(eng, *self == *other);
    return QScriptValue(eng, false);
}

static QScriptValue namespaceUri(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QXmlStreamNamespaceDeclaration, namespaceUri);
    return eng->toScriptValue(self->namespaceUri());
}

static QScriptValue prefix(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QXmlStreamNamespaceDeclaration, prefix);
    return eng->toScriptValue(self->prefix());
}

static QScriptValue toString(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QXmlStreamNamespaceDeclaration, toString);
    return QScriptValue(eng, QLatin1String("QXmlStreamNamespaceDeclaration"));
}

QScriptValue constructXmlStreamNamespaceDeclarationClass(QScriptEngine *eng)
{
    QScriptValue proto = newXmlStreamNamespaceDeclaration(eng, QXmlStreamNamespaceDeclaration());
    ADD_METHOD(proto, equals);
    ADD_METHOD(proto, namespaceUri);
    ADD_METHOD(proto, prefix);
    ADD_METHOD(proto, toString);

    eng->setDefaultPrototype(qMetaTypeId<QXmlStreamNamespaceDeclaration>(), proto);
    eng->setDefaultPrototype(qMetaTypeId<QXmlStreamNamespaceDeclaration*>(), proto);

    QScriptValue ctorFun = eng->newFunction(ctor, proto);
    return ctorFun;
}
