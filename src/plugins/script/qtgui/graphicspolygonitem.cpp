#include <QtScript/QScriptValue>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptContext>
#include <QtGui/QGraphicsPolygonItem>
#include "../global.h"

DECLARE_POINTER_METATYPE(QGraphicsPolygonItem)
Q_DECLARE_METATYPE(QAbstractGraphicsShapeItem*)
Q_DECLARE_METATYPE(QPolygonF)

static QScriptValue ctor(QScriptContext *ctx, QScriptEngine *eng)
{
    if (ctx->argumentCount() > 1) {
        return QScript::wrapGVPointer(
            eng, new QGraphicsPolygonItem(qscriptvalue_cast<QPolygonF>(ctx->argument(0)),
                                          qscriptvalue_cast<QGraphicsItem*>(ctx->argument(1))));
    } else {
        if (QGraphicsItem *parent = qscriptvalue_cast<QGraphicsItem*>(ctx->argument(0))) {
            return QScript::wrapGVPointer(
                eng, new QGraphicsPolygonItem(parent));
        } else {
            return QScript::wrapGVPointer(
                eng, new QGraphicsPolygonItem(qscriptvalue_cast<QPolygonF>(ctx->argument(0))));
        }
    }
}

static QScriptValue fillRule(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsPolygonItem, fillRule);
    return QScriptValue(eng, static_cast<int>(self->fillRule()));
}

static QScriptValue polygon(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsPolygonItem, polygon);
    return eng->toScriptValue(self->polygon());
}

static QScriptValue setFillRule(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsPolygonItem, setFillRule);
    self->setFillRule(static_cast<Qt::FillRule>(ctx->argument(0).toInt32()));
    return eng->undefinedValue();
}

static QScriptValue setPolygon(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsPolygonItem, setPolygon);
    self->setPolygon(qscriptvalue_cast<QPolygonF>(ctx->argument(0)));
    return eng->undefinedValue();
}

static QScriptValue toString(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsPolygonItem, toString);
    return QScriptValue(eng, "QGraphicsPolygonItem");
}

QScriptValue constructGraphicsPolygonItemClass(QScriptEngine *eng)
{
    QScriptValue proto = QScript::wrapGVPointer(eng, new QGraphicsPolygonItem());
    proto.setPrototype(eng->defaultPrototype(qMetaTypeId<QAbstractGraphicsShapeItem*>()));

    ADD_METHOD(proto, fillRule);
    ADD_METHOD(proto, polygon);
    ADD_METHOD(proto, setFillRule);
    ADD_METHOD(proto, setPolygon);
    ADD_METHOD(proto, toString);

    QScript::registerPointerMetaType<QGraphicsPolygonItem>(eng, proto);

    return eng->newFunction(ctor, proto);
}
