#include <QtScript/QScriptValue>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptContext>
#include <QtGui/QGraphicsItemGroup>
#include "../global.h"

DECLARE_POINTER_METATYPE(QGraphicsItemGroup)

static QScriptValue ctor(QScriptContext *ctx, QScriptEngine *eng)
{
    return QScript::wrapGVPointer(
        eng, new QGraphicsItemGroup(
            qscriptvalue_cast<QGraphicsItem*>(ctx->argument(0))));
}

static QScriptValue addToGroup(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsItemGroup, addToGroup);
    self->addToGroup(qscriptvalue_cast<QGraphicsItem*>(ctx->argument(0)));
    return eng->undefinedValue();
}

static QScriptValue removeFromGroup(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsItemGroup, removeFromGroup);
    self->removeFromGroup(qscriptvalue_cast<QGraphicsItem*>(ctx->argument(0)));
    return eng->undefinedValue();
}

static QScriptValue toString(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsItemGroup, toString);
    return QScriptValue(eng, "QGraphicsItemGroup");
}

QScriptValue constructGraphicsItemGroupClass(QScriptEngine *eng)
{
    QScriptValue proto = QScript::wrapGVPointer(eng, new QGraphicsItemGroup());
    proto.setPrototype(eng->defaultPrototype(qMetaTypeId<QGraphicsItem*>()));

    ADD_METHOD(proto, addToGroup);
    ADD_METHOD(proto, removeFromGroup);
    ADD_METHOD(proto, toString);

    QScript::registerPointerMetaType<QGraphicsItemGroup>(eng, proto);

    return eng->newFunction(ctor, proto);
}
