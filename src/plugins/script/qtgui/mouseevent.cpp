#include <QtScript/QScriptValue>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptContext>
#include <QtScript/QScriptable>
#include <QMouseEvent>
#include "../global.h"

Q_DECLARE_METATYPE(QEvent*)
Q_DECLARE_METATYPE(QMouseEvent*)
Q_DECLARE_METATYPE(QScript::Wrapper<QMouseEvent*>::pointer_type)

static QScriptValue newMouseEvent(QScriptEngine *eng, QMouseEvent *e)
{
    return eng->newVariant(qVariantFromValue(QScript::Wrapper<QMouseEvent*>::wrap(e)));
}

/////////////////////////////////////////////////////////////

static QScriptValue ctor(QScriptContext *ctx, QScriptEngine *eng)
{
    QEvent::Type type = QEvent::Type(ctx->argument(0).toInt32());
    QPoint pos = qscriptvalue_cast<QPoint>(ctx->argument(1));
    if (ctx->argumentCount() < 6) {
        Qt::MouseButton button = Qt::MouseButton(ctx->argument(2).toInt32());
        Qt::MouseButtons buttons = Qt::MouseButtons(ctx->argument(3).toInt32());
        Qt::KeyboardModifiers modifiers = Qt::KeyboardModifiers(ctx->argument(4).toInt32());
        return newMouseEvent(eng, new QMouseEvent(type, pos, button,
                                                  buttons, modifiers));
    } else {
        QPoint globalPos = qscriptvalue_cast<QPoint>(ctx->argument(2));
        Qt::MouseButton button = Qt::MouseButton(ctx->argument(3).toInt32());
        Qt::MouseButtons buttons = Qt::MouseButtons(ctx->argument(4).toInt32());
        Qt::KeyboardModifiers modifiers = Qt::KeyboardModifiers(ctx->argument(5).toInt32());
        return newMouseEvent(eng, new QMouseEvent(type, pos, globalPos,
                                                  button, buttons, modifiers));
    }
}

/////////////////////////////////////////////////////////////

static QScriptValue button(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(MouseEvent, button);
    return QScriptValue(eng, self->button());
}

/////////////////////////////////////////////////////////////

static QScriptValue buttons(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(MouseEvent, buttons);
    return QScriptValue(eng, self->buttons());
}

/////////////////////////////////////////////////////////////

static QScriptValue globalPos(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(MouseEvent, globalPos);
    return eng->toScriptValue(self->globalPos());
}

/////////////////////////////////////////////////////////////

static QScriptValue globalX(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(MouseEvent, globalX);
    return QScriptValue(eng, self->globalX());
}

/////////////////////////////////////////////////////////////

static QScriptValue globalY(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(MouseEvent, globalY);
    return QScriptValue(eng, self->globalY());
}

/////////////////////////////////////////////////////////////

static QScriptValue pos(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(MouseEvent, pos);
    return eng->toScriptValue(self->pos());
}

/////////////////////////////////////////////////////////////

static QScriptValue x(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(MouseEvent, x);
    return QScriptValue(eng, self->x());
}

/////////////////////////////////////////////////////////////

static QScriptValue y(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(MouseEvent, y);
    return QScriptValue(eng, self->y());
}

/////////////////////////////////////////////////////////////

static QScriptValue toString(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(MouseEvent, toString);
    return QScriptValue(eng, QString::fromLatin1("QMouseEvent"));
}

/////////////////////////////////////////////////////////////

QScriptValue constructMouseEventClass(QScriptEngine *eng)
{
    QScriptValue proto = newMouseEvent(eng, new QMouseEvent(QEvent::MouseButtonPress,
                                                            QPoint(0, 0),
                                                            Qt::LeftButton,
                                                            Qt::NoButton,
                                                            Qt::NoModifier));
    proto.setPrototype(eng->defaultPrototype(qMetaTypeId<QEvent*>())); /// ### QInputEvent
    ADD_PROTO_FUNCTION(proto, button);
    ADD_PROTO_FUNCTION(proto, buttons);
    ADD_PROTO_FUNCTION(proto, globalPos);
    ADD_PROTO_FUNCTION(proto, globalX);
    ADD_PROTO_FUNCTION(proto, globalY);
    ADD_PROTO_FUNCTION(proto, pos);
    ADD_PROTO_FUNCTION(proto, x);
    ADD_PROTO_FUNCTION(proto, y);
    ADD_PROTO_FUNCTION(proto, toString);

    QScript::registerMetaTypeWrapper<QScript::Wrapper<QMouseEvent*> >(eng, proto);

    QScriptValue ctorFun = eng->newFunction(ctor, proto);

    return ctorFun;
}
