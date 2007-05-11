#include <QtScript/QScriptValue>
#include <QtScript/QScriptEngine>

#include <QtCore/QMetaType>

#include <QtGui/QPolygon>
#include <QtGui/QPolygonF>
#include <qdebug.h>

Q_DECLARE_METATYPE(QPolygon)
Q_DECLARE_METATYPE(QPolygonF)

static QScriptValue ctor(QScriptContext *context, QScriptEngine *engine)
{
    return context->thisObject()
        .property("constructor", QScriptValue::ResolvePrototype)
        .property("prototype")
        .property("constructor")
        .call(context->thisObject(), context->argumentsObject());
}

static QScriptValue fromQPolygon(QScriptEngine *engine, const QPolygon &polygon)
{
    const int len = polygon.size();
    const QPoint *data = polygon.constData();
    QScriptValue array = engine->newArray(len);
    for (int i = 0; i < len; ++i)
        array.setProperty(i, qScriptValueFromValue<QPoint>(engine, data[i]));
    return array;
}

static void toQPolygon(const QScriptValue &value, QPolygon &polygon)
{
    const int len = value.property("length").toInt32();
    polygon.resize(len);
    QPoint *data = polygon.data();
    for (int i = 0; i < len; ++i)
        data[i] = qscriptvalue_cast<QPoint>(value.property(i));
}

static QScriptValue fromQPolygonF(QScriptEngine *engine, const QPolygonF &polygon)
{
    const int len = polygon.size();
    const QPointF *data = polygon.constData();
    QScriptValue array = engine->newArray(len);
    for (int i = 0; i < len; ++i)
        array.setProperty(i, qScriptValueFromValue<QPointF>(engine, data[i]));
    return array;
}

static void toQPolygonF(const QScriptValue &value, QPolygonF &polygon)
{
    const int len = value.property("length").toInt32();
    polygon.resize(len);
    QPointF *data = polygon.data();
    for (int i = 0; i < len; ++i)
        data[i] = qscriptvalue_cast<QPointF>(value.property(i));
}

QScriptValue constructPolygonClass(QScriptEngine *eng)
{
    QScriptValue proto = eng->newArray();
    qScriptRegisterMetaType<QPolygon>(eng, fromQPolygon, toQPolygon, proto);
    qScriptRegisterMetaType<QPolygonF>(eng, fromQPolygonF, toQPolygonF, proto);
    QScriptValue fun = eng->newFunction(ctor);
    fun.setProperty("prototype", proto);
    return fun;
}
