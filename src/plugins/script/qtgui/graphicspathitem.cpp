#include <QtScript/QScriptValue>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptContext>
#include <QtGui/QGraphicsPathItem>
#include "../global.h"

DECLARE_POINTER_METATYPE(QGraphicsPathItem)
Q_DECLARE_METATYPE(QAbstractGraphicsShapeItem*)
Q_DECLARE_METATYPE(QPainterPath)

static QScriptValue ctor(QScriptContext *ctx, QScriptEngine *eng)
{
    if (ctx->argumentCount() > 1) {
        return QScript::wrapGVPointer(
            eng, new QGraphicsPathItem(qscriptvalue_cast<QPainterPath>(ctx->argument(0)),
                                       qscriptvalue_cast<QGraphicsItem*>(ctx->argument(1))));
    } else {
        return QScript::wrapGVPointer(
            eng, new QGraphicsPathItem(qscriptvalue_cast<QGraphicsItem*>(ctx->argument(0))));
    }
}

static QScriptValue path(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsPathItem, path);
    return eng->toScriptValue(self->path());
}

static QScriptValue setPath(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsPathItem, setPath);
    self->setPath(qscriptvalue_cast<QPainterPath>(ctx->argument(0)));
    return eng->undefinedValue();
}

static QScriptValue toString(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsPathItem, toString);
    return QScriptValue(eng, "QGraphicsPathItem");
}

QScriptValue constructGraphicsPathItemClass(QScriptEngine *eng)
{
    QScriptValue proto = QScript::wrapGVPointer(eng, new QGraphicsPathItem());
    proto.setPrototype(eng->defaultPrototype(qMetaTypeId<QAbstractGraphicsShapeItem*>()));

    ADD_METHOD(proto, path);
    ADD_METHOD(proto, setPath);
    ADD_METHOD(proto, toString);

    QScript::registerPointerMetaType<QGraphicsPathItem>(eng, proto);

    return eng->newFunction(ctor, proto);
}
