#include <QtScript/QScriptValue>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptContext>
#include <QtScript/QScriptable>
#include <QtCore/QTimer>
#include "../global.h"

Q_DECLARE_METATYPE(QTimer*)

QT_BEGIN_NAMESPACE

static QScriptValue newTimer(QScriptEngine *eng, QTimer *timer)
{
    return eng->newQObject(timer, QScriptEngine::AutoOwnership);
}

static QScriptValue ctor(QScriptContext *ctx, QScriptEngine *eng)
{
    return newTimer(eng, new QTimer(qscriptvalue_cast<QObject*>(ctx->argument(0))));
}

static QScriptValue toString(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QTimer, toString);
    return QScriptValue(eng, QString::fromLatin1("QTimer(interval=%0)")
                        .arg(self->interval()));
}

QScriptValue constructTimerClass(QScriptEngine *eng)
{
    QScriptValue proto = newTimer(eng, new QTimer());
    ADD_METHOD(proto, toString);
    eng->setDefaultPrototype(qMetaTypeId<QTimer*>(), proto);

    return eng->newFunction(ctor, proto);
}

QT_END_NAMESPACE
