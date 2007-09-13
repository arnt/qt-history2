#include <QtScript/QScriptValue>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptContext>
#include <QtGui/QGraphicsRectItem>
#include "../global.h"

Q_DECLARE_METATYPE(QAbstractGraphicsShapeItem*)
DECLARE_POINTER_METATYPE(QGraphicsRectItem)

QT_BEGIN_NAMESPACE

static QScriptValue ctor(QScriptContext *ctx, QScriptEngine *eng)
{
    if (ctx->argumentCount() >= 4) {
        return QScript::wrapGVPointer(
            eng, new QGraphicsRectItem(ctx->argument(0).toNumber(),
                                       ctx->argument(1).toNumber(),
                                       ctx->argument(2).toNumber(),
                                       ctx->argument(3).toNumber(),
                                       qscriptvalue_cast<QGraphicsItem*>(ctx->argument(4))));
    } else if (ctx->argumentCount() > 1) {
        return QScript::wrapGVPointer(
            eng, new QGraphicsRectItem(qscriptvalue_cast<QRectF>(ctx->argument(0)),
                                       qscriptvalue_cast<QGraphicsItem*>(ctx->argument(1))));
    } else {
        if (QGraphicsItem *parent = qscriptvalue_cast<QGraphicsItem*>(ctx->argument(0))) {
            return QScript::wrapGVPointer(
                eng, new QGraphicsRectItem(parent));
        } else {
            return QScript::wrapGVPointer(
                eng, new QGraphicsRectItem(qscriptvalue_cast<QRectF>(ctx->argument(0))));
        }
    }
}

static QScriptValue rect(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsRectItem, rect);
    return qScriptValueFromValue(eng, self->rect());
}

static QScriptValue setRect(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsRectItem, setRect);
    if (ctx->argumentCount() > 1) {
        self->setRect(ctx->argument(0).toNumber(),
                      ctx->argument(1).toNumber(),
                      ctx->argument(2).toNumber(),
                      ctx->argument(3).toNumber());
    } else {
        self->setRect(qscriptvalue_cast<QRectF>(ctx->argument(0)));
    }
    return eng->undefinedValue();
}

static QScriptValue toString(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsRectItem, toString);
    return QScriptValue(eng, "QGraphicsRectItem");
}

QScriptValue constructGraphicsRectItemClass(QScriptEngine *eng)
{
    QScriptValue proto = QScript::wrapGVPointer(eng, new QGraphicsRectItem());
    proto.setPrototype(eng->defaultPrototype(qMetaTypeId<QAbstractGraphicsShapeItem*>()));

    ADD_METHOD(proto, rect);
    ADD_METHOD(proto, setRect);
    ADD_METHOD(proto, toString);

    QScript::registerPointerMetaType<QGraphicsRectItem>(eng, proto);

    return eng->newFunction(ctor, proto);
}

QT_END_NAMESPACE
