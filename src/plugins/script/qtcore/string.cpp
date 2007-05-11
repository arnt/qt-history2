
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
    ADD_PROTO_FUNCTION(proto, toUtf8);
    ADD_PROTO_FUNCTION(proto, startsWith);
    ADD_PROTO_FUNCTION(proto, endsWith);
    ADD_PROTO_FUNCTION(proto, left);
    ADD_PROTO_FUNCTION(proto, right);
    ADD_PROTO_FUNCTION(proto, mid);
    ADD_PROTO_FUNCTION(proto, simplified);
    ADD_PROTO_FUNCTION(proto, trimmed);
}

