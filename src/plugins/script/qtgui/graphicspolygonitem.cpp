#include <QtScript/QScriptValue>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptContext>
#include <QtGui/QGraphicsPolygonItem>
#include "../global.h"

Q_DECLARE_METATYPE(QGraphicsPolygonItem*)
Q_DECLARE_METATYPE(QScript::Wrapper<QGraphicsPolygonItem*>::pointer_type)
Q_DECLARE_METATYPE(QAbstractGraphicsShapeItem*)
Q_DECLARE_METATYPE(QPolygonF)

/////////////////////////////////////////////////////////////

static QScriptValue ctor(QScriptContext *ctx, QScriptEngine *eng)
{
    if (ctx->argumentCount() > 1) {
        return QScript::construct<QGraphicsPolygonItem>(
            eng, new QGraphicsPolygonItem(qscriptvalue_cast<QPolygonF>(ctx->argument(0)),
                                          qscriptvalue_cast<QGraphicsItem*>(ctx->argument(1))));
    } else {
        if (QGraphicsItem *parent = qscriptvalue_cast<QGraphicsItem*>(ctx->argument(0))) {
            return QScript::construct<QGraphicsPolygonItem>(
                eng, new QGraphicsPolygonItem(parent));
        } else {
            return QScript::construct<QGraphicsPolygonItem>(
                eng, new QGraphicsPolygonItem(qscriptvalue_cast<QPolygonF>(ctx->argument(0))));
        }
    }
}

/////////////////////////////////////////////////////////////

static QScriptValue fillRule(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsPolygonItem, fillRule);
    return QScriptValue(eng, static_cast<int>(self->fillRule()));
}

/////////////////////////////////////////////////////////////

static QScriptValue polygon(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsPolygonItem, polygon);
    return eng->toScriptValue(self->polygon());
}

/////////////////////////////////////////////////////////////

static QScriptValue setFillRule(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsPolygonItem, setFillRule);
    self->setFillRule(static_cast<Qt::FillRule>(ctx->argument(0).toInt32()));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setPolygon(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsPolygonItem, setPolygon);
    self->setPolygon(qscriptvalue_cast<QPolygonF>(ctx->argument(0)));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue toString(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsPolygonItem, toString);
    return QScriptValue(eng, "QGraphicsPolygonItem");
}

/////////////////////////////////////////////////////////////

QScriptValue constructGraphicsPolygonItemClass(QScriptEngine *eng)
{
    QScriptValue proto = QScript::construct<QGraphicsPolygonItem>(eng, new QGraphicsPolygonItem());
    proto.setPrototype(eng->defaultPrototype(qMetaTypeId<QAbstractGraphicsShapeItem*>()));

    ADD_PROTO_FUNCTION(proto, fillRule);
    ADD_PROTO_FUNCTION(proto, polygon);
    ADD_PROTO_FUNCTION(proto, setFillRule);
    ADD_PROTO_FUNCTION(proto, setPolygon);
    ADD_PROTO_FUNCTION(proto, toString);

    QScript::registerMetaTypeWrapper<QScript::Wrapper<QGraphicsPolygonItem*> >(eng, proto);

    return eng->newFunction(ctor, proto);
}
