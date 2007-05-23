#include <QtScript/QScriptValue>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptContext>
#include <QtXml/QXmlStreamWriter>
#include "../global.h"

DECLARE_POINTER_METATYPE(QXmlStreamWriter)

static inline QScriptValue newXmlStreamWriter(QScriptEngine *eng, QXmlStreamWriter *writer)
{
    return QScript::wrapPointer(eng, writer);
}

static QScriptValue ctor(QScriptContext *ctx, QScriptEngine *eng)
{
    return eng->undefinedValue();
}

static QScriptValue toString(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(XmlStreamWriter, toString);
    return QScriptValue(eng, QLatin1String("QXmlStreamWriter"));
}

QScriptValue constructXmlStreamWriterClass(QScriptEngine *eng)
{
    QScriptValue proto = newXmlStreamWriter(eng, new QXmlStreamWriter());
    ADD_PROTO_FUNCTION(proto, toString);

    QScript::registerPointerMetaType<QXmlStreamWriter>(eng, proto);

    QScriptValue ctorFun = eng->newFunction(ctor, proto);
    return ctorFun;
}
