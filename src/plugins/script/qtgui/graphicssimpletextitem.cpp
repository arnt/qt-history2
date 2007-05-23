#include <QtScript/QScriptValue>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptContext>
#include <QtGui/QGraphicsSimpleTextItem>
#include <QtGui/QFont>
#include "../global.h"

DECLARE_POINTER_METATYPE(QGraphicsSimpleTextItem)
Q_DECLARE_METATYPE(QAbstractGraphicsShapeItem*)

static QScriptValue ctor(QScriptContext *ctx, QScriptEngine *eng)
{
    if (ctx->argumentCount() > 1) {
        return QScript::wrapGVPointer(
            eng, new QGraphicsSimpleTextItem(ctx->argument(0).toString(),
                                             qscriptvalue_cast<QGraphicsItem*>(ctx->argument(1))));
    } else {
        return QScript::wrapGVPointer(
            eng, new QGraphicsSimpleTextItem(qscriptvalue_cast<QGraphicsItem*>(ctx->argument(0))));
    }
}

static QScriptValue font(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsSimpleTextItem, font);
    return eng->toScriptValue(self->font());
}

static QScriptValue setFont(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsSimpleTextItem, setFont);
    self->setFont(qscriptvalue_cast<QFont>(ctx->argument(0)));
    return eng->undefinedValue();
}

static QScriptValue setText(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsSimpleTextItem, setText);
    self->setText(ctx->argument(0).toString());
    return eng->undefinedValue();
}

static QScriptValue text(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsSimpleTextItem, text);
    return QScriptValue(eng, self->text());
}

static QScriptValue toString(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsSimpleTextItem, toString);
    return QScriptValue(eng, "QGraphicsSimpleTextItem");
}

QScriptValue constructGraphicsSimpleTextItemClass(QScriptEngine *eng)
{
    QScriptValue proto = QScript::wrapGVPointer(eng, new QGraphicsSimpleTextItem());
    proto.setPrototype(eng->defaultPrototype(qMetaTypeId<QAbstractGraphicsShapeItem*>()));

    ADD_PROTO_FUNCTION(proto, font);
    ADD_PROTO_FUNCTION(proto, setFont);
    ADD_PROTO_FUNCTION(proto, setText);
    ADD_PROTO_FUNCTION(proto, text);
    ADD_PROTO_FUNCTION(proto, toString);

    QScript::registerPointerMetaType<QGraphicsSimpleTextItem>(eng, proto);

    return eng->newFunction(ctor, proto);
}
