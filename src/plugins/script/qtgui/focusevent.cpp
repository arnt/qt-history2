#include <QtScript/QScriptValue>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptContext>
#include <QtScript/QScriptable>
#include <QFocusEvent>
#include "../global.h"

Q_DECLARE_METATYPE(QEvent*)
DECLARE_POINTER_METATYPE(QFocusEvent)

QT_BEGIN_NAMESPACE

static QScriptValue newFocusEvent(QScriptEngine *eng, QFocusEvent *fe)
{
    return QScript::wrapPointer(eng, fe);
}

static QScriptValue ctor(QScriptContext *ctx, QScriptEngine *eng)
{
    QEvent::Type type = QEvent::Type(ctx->argument(0).toInt32());
    int reason = ctx->argument(1).toInt32();
    return newFocusEvent(eng, new QFocusEvent(type, Qt::FocusReason(reason)));
}

static QScriptValue gotFocus(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QFocusEvent, gotFocus);
    return qScriptValueFromValue(eng, self->gotFocus());
}

static QScriptValue lostFocus(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QFocusEvent, lostFocus);
    return qScriptValueFromValue(eng, self->lostFocus());
}

static QScriptValue toString(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QFocusEvent, toString);
    return QScriptValue(eng, QLatin1String("QFocusEvent"));
}

QScriptValue constructFocusEventClass(QScriptEngine *eng)
{
    QScriptValue proto = newFocusEvent(eng, new QFocusEvent(QEvent::FocusIn));
    proto.setPrototype(eng->defaultPrototype(qMetaTypeId<QEvent*>()));
    ADD_METHOD(proto, gotFocus);
    ADD_METHOD(proto, lostFocus);
    ADD_METHOD(proto, toString);

    QScript::registerPointerMetaType<QFocusEvent>(eng, proto);

    QScriptValue ctorFun = eng->newFunction(ctor, proto);

    return ctorFun;
}

QT_END_NAMESPACE
