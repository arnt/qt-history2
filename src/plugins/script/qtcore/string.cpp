
#include <QtScript/QScriptValue>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptContext>
#include <QtCore/QByteArray>
#include "../global.h"

/////////////////////////////////////////////////////////////

static QScriptValue toUtf8(QScriptContext *ctx, QScriptEngine *eng)
{
    QString str = ctx->thisObject().toString();
    return eng->toScriptValue<QByteArray>(str.toUtf8());
}

static QScriptValue fromUtf8(QScriptContext *ctx, QScriptEngine *eng)
{
    QByteArray array = qscriptvalue_cast<QByteArray>(ctx->argument(0));
    return QScriptValue(eng, QString::fromUtf8(array));
}

static QScriptValue startsWith(QScriptContext *ctx, QScriptEngine *eng)
{
    QString str = ctx->thisObject().toString();
    return QScriptValue(eng, str.startsWith(ctx->argument(0).toString()));
}

static QScriptValue endsWith(QScriptContext *ctx, QScriptEngine *eng)
{
    QString str = ctx->thisObject().toString();
    return QScriptValue(eng, str.endsWith(ctx->argument(0).toString()));
}

static QScriptValue left(QScriptContext *ctx, QScriptEngine *eng)
{
    QString str = ctx->thisObject().toString();
    return QScriptValue(eng, str.left(ctx->argument(0).toInt32()));
}

static QScriptValue right(QScriptContext *ctx, QScriptEngine *eng)
{
    QString str = ctx->thisObject().toString();
    return QScriptValue(eng, str.right(ctx->argument(0).toInt32()));
}

static QScriptValue mid(QScriptContext *ctx, QScriptEngine *eng)
{
    QString str = ctx->thisObject().toString();
    int len = -1;
    if (ctx->argumentCount() >= 2)
        len = ctx->argument(1).toInt32();
    return QScriptValue(eng, str.mid(ctx->argument(0).toInt32(), len));
}

static QScriptValue trimmed(QScriptContext *ctx, QScriptEngine *eng)
{
    QString str = ctx->thisObject().toString();
    return QScriptValue(eng, str.trimmed());
}

static QScriptValue simplified(QScriptContext *ctx, QScriptEngine *eng)
{
    QString str = ctx->thisObject().toString();
    return QScriptValue(eng, str.simplified());
}

/////////////////////////////////////////////////////////////

void extendStringPrototype(QScriptEngine *eng)
{
    QScriptValue proto = eng->globalObject().property("String").property("prototype");
    Q_ASSERT(proto.isValid());
    ADD_METHOD(proto, toUtf8);
    ADD_METHOD(proto, startsWith);
    ADD_METHOD(proto, endsWith);
    ADD_METHOD(proto, left);
    ADD_METHOD(proto, right);
    ADD_METHOD(proto, mid);
    ADD_METHOD(proto, simplified);
    ADD_METHOD(proto, trimmed);
    eng->globalObject().property("String").setProperty("fromUtf8", eng->newFunction(fromUtf8));
}

