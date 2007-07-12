#include <QtScript/QScriptValue>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptContext>
#include <QtScript/QScriptable>
#include <QResizeEvent>
#include "../global.h"

Q_DECLARE_METATYPE(QEvent*)
DECLARE_POINTER_METATYPE(QResizeEvent)

static QScriptValue newResizeEvent(QScriptEngine *eng, QResizeEvent *re)
{
    return QScript::wrapPointer(eng, re);
}

static QScriptValue ctor(QScriptContext *ctx, QScriptEngine *eng)
{
    QSize size = qscriptvalue_cast<QSize>(ctx->argument(0));
    QSize oldSize = qscriptvalue_cast<QSize>(ctx->argument(1));
    return newResizeEvent(eng, new QResizeEvent(size, oldSize));
}

static QScriptValue oldSize(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QResizeEvent, oldSize);
    return eng->toScriptValue(self->oldSize());
}

static QScriptValue size(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QResizeEvent, size);
    return eng->toScriptValue(self->size());
}

static QScriptValue toString(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QResizeEvent, toString);
    return QScriptValue(eng, QLatin1String("QResizeEvent"));
}

QScriptValue constructResizeEventClass(QScriptEngine *eng)
{
    QScriptValue proto = newResizeEvent(eng, new QResizeEvent(QSize(), QSize()));
    proto.setPrototype(eng->defaultPrototype(qMetaTypeId<QEvent*>()));
    ADD_METHOD(proto, oldSize);
    ADD_METHOD(proto, size);
    ADD_METHOD(proto, toString);

    QScript::registerPointerMetaType<QResizeEvent>(eng, proto);

    QScriptValue ctorFun = eng->newFunction(ctor, proto);

    return ctorFun;
}
