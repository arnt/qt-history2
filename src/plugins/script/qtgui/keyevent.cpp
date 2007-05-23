#include <QtScript/QScriptValue>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptContext>
#include <QtScript/QScriptable>
#include <QKeyEvent>
#include "../global.h"

Q_DECLARE_METATYPE(QEvent*)
DECLARE_POINTER_METATYPE(QKeyEvent)

static QScriptValue newKeyEvent(QScriptEngine *eng, QKeyEvent *ke)
{
    return QScript::wrapPointer(eng, ke);
}

/////////////////////////////////////////////////////////////

static QScriptValue ctor(QScriptContext *ctx, QScriptEngine *eng)
{
    QEvent::Type type = QEvent::Type(ctx->argument(0).toInt32());
    int key = ctx->argument(1).toInt32();
    Qt::KeyboardModifiers modifiers = Qt::KeyboardModifiers(ctx->argument(2).toInt32());
    QString text = QString();
    bool autorep = false;
    ushort count = 1;
    if (ctx->argumentCount() > 3)
        text = ctx->argument(3).toString();
    if (ctx->argumentCount() > 4)
        autorep = ctx->argument(4).toBoolean();
    if (ctx->argumentCount() > 5)
        count = ctx->argument(5).toUInt16();
    return newKeyEvent(eng, new QKeyEvent(type, key, modifiers, text, autorep, count));
}

/////////////////////////////////////////////////////////////

static QScriptValue count(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QKeyEvent, count);
    return QScriptValue(eng, self->count());
}

/////////////////////////////////////////////////////////////

static QScriptValue isAutoRepeat(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QKeyEvent, isAutoRepeat);
    return QScriptValue(eng, self->isAutoRepeat());
}

/////////////////////////////////////////////////////////////

static QScriptValue key(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QKeyEvent, key);
    return QScriptValue(eng, self->key());
}

/////////////////////////////////////////////////////////////

static QScriptValue matches(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QKeyEvent, matches);
    return QScriptValue(eng, self->matches(QKeySequence::StandardKey(ctx->argument(0).toInt32())));
}

/////////////////////////////////////////////////////////////

static QScriptValue modifiers(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QKeyEvent, modifiers);
    return QScriptValue(eng, self->modifiers());
}

/////////////////////////////////////////////////////////////

static QScriptValue nativeModifiers(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QKeyEvent, nativeModifiers);
    return QScriptValue(eng, self->nativeModifiers());
}

/////////////////////////////////////////////////////////////

static QScriptValue nativeScanCode(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QKeyEvent, nativeScanCode);
    return QScriptValue(eng, self->nativeScanCode());
}

/////////////////////////////////////////////////////////////

static QScriptValue nativeVirtualKey(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QKeyEvent, nativeVirtualKey);
    return QScriptValue(eng, self->nativeVirtualKey());
}

/////////////////////////////////////////////////////////////

static QScriptValue text(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QKeyEvent, text);
    return QScriptValue(eng, self->text());
}

/////////////////////////////////////////////////////////////

static QScriptValue toString(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QKeyEvent, toString);
    return QScriptValue(eng, QString::fromLatin1("QKeyEvent"));
}

/////////////////////////////////////////////////////////////

QScriptValue constructKeyEventClass(QScriptEngine *eng)
{
    QScriptValue proto = newKeyEvent(eng, new QKeyEvent(QEvent::KeyPress, Qt::Key_Dead_Grave, Qt::NoModifier));
    proto.setPrototype(eng->defaultPrototype(qMetaTypeId<QEvent*>())); /// ### QInputEvent
    ADD_METHOD(proto, count);
    ADD_METHOD(proto, isAutoRepeat);
    ADD_METHOD(proto, key);
    ADD_METHOD(proto, matches);
    ADD_METHOD(proto, modifiers);
    ADD_METHOD(proto, nativeModifiers);
    ADD_METHOD(proto, nativeScanCode);
    ADD_METHOD(proto, nativeVirtualKey);
    ADD_METHOD(proto, text);
    ADD_METHOD(proto, toString);

    QScript::registerPointerMetaType<QKeyEvent>(eng, proto);

    QScriptValue ctorFun = eng->newFunction(ctor, proto);

    return ctorFun;
}
