#include <QtScript/QScriptValue>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptContext>
#include <QtScript/QScriptable>
#include <QActionEvent>
#include "../global.h"

Q_DECLARE_METATYPE(QAction*)
Q_DECLARE_METATYPE(QEvent*)
DECLARE_POINTER_METATYPE(QActionEvent)

QT_BEGIN_NAMESPACE

static QScriptValue newActionEvent(QScriptEngine *eng, QActionEvent *ae)
{
    return QScript::wrapPointer(eng, ae);
}

static QScriptValue ctor(QScriptContext *ctx, QScriptEngine *eng)
{
    QEvent::Type type = QEvent::Type(ctx->argument(0).toInt32());
    QAction *action = qscriptvalue_cast<QAction*>(ctx->argument(1));
    QAction *after = qscriptvalue_cast<QAction*>(ctx->argument(2));
    return newActionEvent(eng, new QActionEvent(type, action, after));
}

static QScriptValue action(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QActionEvent, action);
    return qScriptValueFromValue(eng, self->action());
}

static QScriptValue before(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QActionEvent, before);
    return qScriptValueFromValue(eng, self->before());
}

static QScriptValue toString(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QActionEvent, toString);
    return QScriptValue(eng, QLatin1String("QActionEvent"));
}

QScriptValue constructActionEventClass(QScriptEngine *eng)
{
    QScriptValue proto = newActionEvent(eng, new QActionEvent(QEvent::ActionAdded, 0));
    proto.setPrototype(eng->defaultPrototype(qMetaTypeId<QEvent*>()));
    ADD_METHOD(proto, action);
    ADD_METHOD(proto, before);
    ADD_METHOD(proto, toString);

    QScript::registerPointerMetaType<QActionEvent>(eng, proto);

    QScriptValue ctorFun = eng->newFunction(ctor, proto);

    return ctorFun;
}

QT_END_NAMESPACE
