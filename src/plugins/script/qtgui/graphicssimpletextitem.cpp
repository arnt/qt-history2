#include <QtScript/QScriptValue>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptContext>
#include <QtGui/QGraphicsSimpleTextItem>
#include <QtGui/QFont>
#include "../global.h"

Q_DECLARE_METATYPE(QAbstractGraphicsShapeItem*)

DECLARE_POINTER_METATYPE(QGraphicsSimpleTextItem)

QT_BEGIN_NAMESPACE

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
    DECLARE_SELF(QGraphicsSimpleTextItem, font);
    return qScriptValueFromValue(eng, self->font());
}

static QScriptValue setFont(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsSimpleTextItem, setFont);
    self->setFont(qscriptvalue_cast<QFont>(ctx->argument(0)));
    return eng->undefinedValue();
}

static QScriptValue setText(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsSimpleTextItem, setText);
    self->setText(ctx->argument(0).toString());
    return eng->undefinedValue();
}

static QScriptValue text(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsSimpleTextItem, text);
    return QScriptValue(eng, self->text());
}

static QScriptValue toString(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsSimpleTextItem, toString);
    return QScriptValue(eng, "QGraphicsSimpleTextItem");
}

QScriptValue constructGraphicsSimpleTextItemClass(QScriptEngine *eng)
{
    QScriptValue proto = QScript::wrapGVPointer(eng, new QGraphicsSimpleTextItem());
    proto.setPrototype(eng->defaultPrototype(qMetaTypeId<QAbstractGraphicsShapeItem*>()));

    ADD_METHOD(proto, font);
    ADD_METHOD(proto, setFont);
    ADD_METHOD(proto, setText);
    ADD_METHOD(proto, text);
    ADD_METHOD(proto, toString);

    QScript::registerPointerMetaType<QGraphicsSimpleTextItem>(eng, proto);

    return eng->newFunction(ctor, proto);
}

QT_END_NAMESPACE
