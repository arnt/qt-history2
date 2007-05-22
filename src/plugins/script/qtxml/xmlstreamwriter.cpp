#include <QtScript/QScriptValue>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptContext>
#include <QtXml/QXmlStreamWriter>
#include "../global.h"

Q_DECLARE_METATYPE(QXmlStreamWriter*)
Q_DECLARE_METATYPE(QScript::Wrapper<QXmlStreamWriter*>::pointer_type)

static inline QScriptValue newXmlStreamWriter(QScriptEngine *eng, QXmlStreamWriter *writer)
{
    return eng->newVariant(qVariantFromValue(QScript::Wrapper<QXmlStreamWriter*>::wrap(writer)));
}

/////////////////////////////////////////////////////////////

static QScriptValue ctor(QScriptContext *ctx, QScriptEngine *eng)
{
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue toString(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(XmlStreamWriter, toString);
    return QScriptValue(eng, QLatin1String("QXmlStreamWriter"));
}

/////////////////////////////////////////////////////////////

QScriptValue constructXmlStreamWriterClass(QScriptEngine *eng)
{
    QScriptValue proto = newXmlStreamWriter(eng, new QXmlStreamWriter());
    ADD_PROTO_FUNCTION(proto, toString);

    QScriptValue ctorFun = eng->newFunction(ctor, proto);
    return ctorFun;
}
