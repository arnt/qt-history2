#include <QtScript/QScriptValue>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptContext>
#include <QtGui/QGraphicsPathItem>
#include "../global.h"

Q_DECLARE_METATYPE(QGraphicsPathItem*)
Q_DECLARE_METATYPE(QScript::Wrapper<QGraphicsPathItem*>::pointer_type)
Q_DECLARE_METATYPE(QAbstractGraphicsShapeItem*)
Q_DECLARE_METATYPE(QPainterPath)

/////////////////////////////////////////////////////////////

static QScriptValue ctor(QScriptContext *ctx, QScriptEngine *eng)
{
    if (ctx->argumentCount() > 1) {
        return QScript::construct<QGraphicsPathItem>(
            eng, new QGraphicsPathItem(qscriptvalue_cast<QPainterPath>(ctx->argument(0)),
                                       qscriptvalue_cast<QGraphicsItem*>(ctx->argument(1))));
    } else {
        return QScript::construct<QGraphicsPathItem>(
            eng, new QGraphicsPathItem(qscriptvalue_cast<QGraphicsItem*>(ctx->argument(0))));
    }
}

/////////////////////////////////////////////////////////////

static QScriptValue path(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsPathItem, path);
    return eng->toScriptValue(self->path());
}

/////////////////////////////////////////////////////////////

static QScriptValue setPath(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsPathItem, setPath);
    self->setPath(qscriptvalue_cast<QPainterPath>(ctx->argument(0)));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue toString(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsPathItem, toString);
    return QScriptValue(eng, "GraphicsPathItem");
}

/////////////////////////////////////////////////////////////

QScriptValue constructGraphicsPathItemClass(QScriptEngine *eng)
{
    QScriptValue proto = QScript::construct<QGraphicsPathItem>(eng, new QGraphicsPathItem());
    proto.setPrototype(eng->defaultPrototype(qMetaTypeId<QAbstractGraphicsShapeItem*>()));

    ADD_PROTO_FUNCTION(proto, path);
    ADD_PROTO_FUNCTION(proto, setPath);
    ADD_PROTO_FUNCTION(proto, toString);

    QScript::registerMetaTypeWrapper<QScript::Wrapper<QGraphicsPathItem*> >(eng, proto);

    return eng->newFunction(ctor, proto);
}
