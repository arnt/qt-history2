#include <QtScript/QScriptValue>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptContext>
#include <QtScript/QScriptable>
#include <QMouseEvent>
#include "../global.h"

Q_DECLARE_METATYPE(QEvent*)
DECLARE_POINTER_METATYPE(QMouseEvent)

static QScriptValue newMouseEvent(QScriptEngine *eng, QMouseEvent *me)
{
    return QScript::wrapPointer(eng, me);
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
    DECLARE_SELF(QMouseEvent, button);
    return QScriptValue(eng, self->button());
}

/////////////////////////////////////////////////////////////

static QScriptValue buttons(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QMouseEvent, buttons);
    return QScriptValue(eng, self->buttons());
}

/////////////////////////////////////////////////////////////

static QScriptValue globalPos(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QMouseEvent, globalPos);
    return qScriptValueFromValue(eng, self->globalPos());
}

/////////////////////////////////////////////////////////////

static QScriptValue globalX(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QMouseEvent, globalX);
    return QScriptValue(eng, self->globalX());
}

/////////////////////////////////////////////////////////////

static QScriptValue globalY(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QMouseEvent, globalY);
    return QScriptValue(eng, self->globalY());
}

/////////////////////////////////////////////////////////////

static QScriptValue pos(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QMouseEvent, pos);
    return qScriptValueFromValue(eng, self->pos());
}

/////////////////////////////////////////////////////////////

static QScriptValue x(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QMouseEvent, x);
    return QScriptValue(eng, self->x());
}

/////////////////////////////////////////////////////////////

static QScriptValue y(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QMouseEvent, y);
    return QScriptValue(eng, self->y());
}

/////////////////////////////////////////////////////////////

static QScriptValue toString(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QMouseEvent, toString);
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
    ADD_METHOD(proto, button);
    ADD_METHOD(proto, buttons);
    ADD_METHOD(proto, globalPos);
    ADD_METHOD(proto, globalX);
    ADD_METHOD(proto, globalY);
    ADD_METHOD(proto, pos);
    ADD_METHOD(proto, x);
    ADD_METHOD(proto, y);
    ADD_METHOD(proto, toString);

    QScript::registerPointerMetaType<QMouseEvent>(eng, proto);

    QScriptValue ctorFun = eng->newFunction(ctor, proto);

    return ctorFun;
}
