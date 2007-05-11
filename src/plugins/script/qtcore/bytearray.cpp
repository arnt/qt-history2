#include <QtScript/QScriptValue>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptContext>
#include <QtCore/QByteArray>
#include "../global.h"

Q_DECLARE_METATYPE(QByteArray)
Q_DECLARE_METATYPE(QByteArray*)
Q_DECLARE_METATYPE(const char *)
Q_DECLARE_METATYPE(QList<QByteArray>)

static inline QScriptValue newByteArray(QScriptEngine *eng, const QByteArray &array)
{
    return eng->newVariant(qVariantFromValue(array));
}

/////////////////////////////////////////////////////////////

static QScriptValue ctor(QScriptContext *ctx, QScriptEngine *eng)
{
    if (ctx->argumentCount() == 0)
        return newByteArray(eng, QByteArray());
    if (QByteArray *other = qscriptvalue_cast<QByteArray*>(ctx->argument(0)))
        return newByteArray(eng, QByteArray(*other));
    if (const char *str = qscriptvalue_cast<const char *>(ctx->argument(0)))
        return newByteArray(eng, QByteArray(str));
    return newByteArray(eng, QByteArray(ctx->argument(0).toInt32(),
                                        ctx->argument(1).toInt32()));
}

/////////////////////////////////////////////////////////////

static QScriptValue append(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(ByteArray, append);
    QScriptValue arg = ctx->argument(0);
    if (arg.isNumber())
        self->append(arg.toInt32());
    else if (QByteArray *other = qscriptvalue_cast<QByteArray*>(arg))
        self->append(*other);
    else if (const char *str = qscriptvalue_cast<const char *>(arg))
        self->append(str);
    else
        self->append(arg.toString());
    return ctx->thisObject();
}

/////////////////////////////////////////////////////////////

static QScriptValue at(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(ByteArray, at);
    return QScriptValue(eng, self->at(ctx->argument(0).toInt32()));
}

/////////////////////////////////////////////////////////////

static QScriptValue capacity(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(ByteArray, capacity);
    return QScriptValue(eng, self->capacity());
}

/////////////////////////////////////////////////////////////

static QScriptValue chop(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(ByteArray, chop);
    self->chop(ctx->argument(0).toInt32());
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue clear(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(ByteArray, clear);
    self->clear();
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue constData(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(ByteArray, constData);
    return eng->toScriptValue(self->constData());
}

/////////////////////////////////////////////////////////////

static QScriptValue contains(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(ByteArray, contains);
    QScriptValue arg = ctx->argument(0);
    if (QByteArray *other = qscriptvalue_cast<QByteArray*>(arg))
        return QScriptValue(eng, self->contains(*other));
    else if (const char *str = qscriptvalue_cast<const char *>(arg))
        return QScriptValue(eng, self->contains(str));
    else
        return QScriptValue(eng, self->contains(arg.toInt32()));
}

/////////////////////////////////////////////////////////////

static QScriptValue count(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(ByteArray, count);
    QScriptValue arg = ctx->argument(0);
    if (arg.isUndefined())
        return QScriptValue(eng, self->count());
    else if (QByteArray *other = qscriptvalue_cast<QByteArray*>(arg))
        return QScriptValue(eng, self->count(*other));
    else if (const char *str = qscriptvalue_cast<const char *>(arg))
        return QScriptValue(eng, self->count(str));
    else
        return QScriptValue(eng, self->count(arg.toInt32()));
}

/////////////////////////////////////////////////////////////

static QScriptValue data(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(ByteArray, data);
    return ctx->throwError("ByteArray.prototype.data is not implemented");
}

/////////////////////////////////////////////////////////////

static QScriptValue endsWith(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(ByteArray, endsWith);
    QScriptValue arg = ctx->argument(0);
    if (QByteArray *other = qscriptvalue_cast<QByteArray*>(arg))
        return QScriptValue(eng, self->endsWith(*other));
    else if (const char *str = qscriptvalue_cast<const char *>(arg))
        return QScriptValue(eng, self->endsWith(str));
    else
        return QScriptValue(eng, self->endsWith(arg.toInt32()));
}

/////////////////////////////////////////////////////////////

static QScriptValue fill(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(ByteArray, fill);
    char ch = ctx->argument(0).toInt32();
    if (ctx->argumentCount() > 0)
        self->fill(ch, ctx->argument(1).toInt32());
    else
        self->fill(ch);
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue indexOf(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(ByteArray, indexOf);
    int from = ctx->argument(1).toInt32();
    QScriptValue arg = ctx->argument(0);
    if (QByteArray *other = qscriptvalue_cast<QByteArray*>(arg))
        return QScriptValue(eng, self->indexOf(*other, from));
    else if (const char *str = qscriptvalue_cast<const char *>(arg))
        return QScriptValue(eng, self->indexOf(str, from));
    else if (arg.isNumber())
        return QScriptValue(eng, self->indexOf(arg.toInt32(), from));
    else
        return QScriptValue(eng, self->indexOf(arg.toString(), from));
}

/////////////////////////////////////////////////////////////

static QScriptValue insert(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(ByteArray, insert);
    int i = ctx->argument(0).toInt32();
    QScriptValue arg = ctx->argument(1);
    if (QByteArray *other = qscriptvalue_cast<QByteArray*>(arg))
        self->insert(i, *other);
    else if (const char *str = qscriptvalue_cast<const char *>(arg))
        self->insert(i, str);
    else if (arg.isNumber())
        self->insert(i, arg.toInt32());
    else
        self->insert(i, arg.toString());
    return ctx->thisObject();
}

/////////////////////////////////////////////////////////////

static QScriptValue isEmpty(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(ByteArray, isEmpty);
    return QScriptValue(eng, self->isEmpty());
}

/////////////////////////////////////////////////////////////

static QScriptValue isNull(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(ByteArray, isNull);
    return QScriptValue(eng, self->isNull());
}

/////////////////////////////////////////////////////////////

static QScriptValue lastIndexOf(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(ByteArray, lastIndexOf);
    int from = ctx->argument(1).isUndefined() ? -1 : ctx->argument(1).toInt32();
    QScriptValue arg = ctx->argument(0);
    if (QByteArray *other = qscriptvalue_cast<QByteArray*>(arg))
        return QScriptValue(eng, self->lastIndexOf(*other, from));
    else if (const char *str = qscriptvalue_cast<const char *>(arg))
        return QScriptValue(eng, self->lastIndexOf(str, from));
    else if (arg.isNumber())
        return QScriptValue(eng, self->lastIndexOf(arg.toInt32(), from));
    else
        return QScriptValue(eng, self->lastIndexOf(arg.toString(), from));
}

/////////////////////////////////////////////////////////////

static QScriptValue left(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(ByteArray, left);
    return newByteArray(eng, self->left(ctx->argument(0).toInt32()));
}

/////////////////////////////////////////////////////////////

static QScriptValue leftJustified(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(ByteArray, leftJustified);
    return ctx->throwError("ByteArray.prototype.leftJustified is not implemented");
}

/////////////////////////////////////////////////////////////

static QScriptValue length(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(ByteArray, length);
    return QScriptValue(eng, self->length());
}

/////////////////////////////////////////////////////////////

static QScriptValue mid(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(ByteArray, mid);
    int pos = ctx->argument(0).toInt32();
    if (ctx->argument(1).isUndefined())
        return newByteArray(eng, self->mid(pos));
    else
        return newByteArray(eng, self->mid(pos, ctx->argument(1).toInt32()));
}

/////////////////////////////////////////////////////////////

static QScriptValue prepend(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(ByteArray, prepend);
    QScriptValue arg = ctx->argument(0);
    if (QByteArray *other = qscriptvalue_cast<QByteArray*>(arg))
        self->prepend(*other);
    else if (const char *str = qscriptvalue_cast<const char *>(arg))
        self->prepend(str);
    else
        self->prepend(arg.toInt32());
    return ctx->thisObject();
}

/////////////////////////////////////////////////////////////

static QScriptValue push_back(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(ByteArray, push_back);
    QScriptValue arg = ctx->argument(0);
    if (QByteArray *other = qscriptvalue_cast<QByteArray*>(arg))
        self->push_back(*other);
    else if (const char *str = qscriptvalue_cast<const char *>(arg))
        self->push_back(str);
    else
        self->push_back(arg.toInt32());
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue push_front(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(ByteArray, push_front);
    QScriptValue arg = ctx->argument(0);
    if (QByteArray *other = qscriptvalue_cast<QByteArray*>(arg))
        self->push_front(*other);
    else if (const char *str = qscriptvalue_cast<const char *>(arg))
        self->push_front(str);
    else
        self->push_front(arg.toInt32());
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue remove(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(ByteArray, remove);
    self->remove(ctx->argument(0).toInt32(), ctx->argument(1).toInt32());
    return ctx->thisObject();
}

/////////////////////////////////////////////////////////////

static QScriptValue replace(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(ByteArray, replace);
    return ctx->throwError("ByteArray.prototype.replace is not implemented");
}

/////////////////////////////////////////////////////////////

static QScriptValue reserve(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(ByteArray, reserve);
    self->reserve(ctx->argument(0).toInt32());
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue resize(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(ByteArray, resize);
    self->resize(ctx->argument(0).toInt32());
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue right(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(ByteArray, right);
    return newByteArray(eng, self->right(ctx->argument(0).toInt32()));
}

/////////////////////////////////////////////////////////////

static QScriptValue rightJustified(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(ByteArray, rightJustified);
    return ctx->throwError("ByteArray.prototype.rightJustified is not implemented");
}

/////////////////////////////////////////////////////////////

static QScriptValue setNum(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(ByteArray, setNum);
    return ctx->throwError("ByteArray.prototype.setNum is not implemented");
}

/////////////////////////////////////////////////////////////

static QScriptValue simplified(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(ByteArray, simplified);
    return newByteArray(eng, self->simplified());
}

/////////////////////////////////////////////////////////////

static QScriptValue size(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(ByteArray, size);
    return QScriptValue(eng, self->size());
}

/////////////////////////////////////////////////////////////

static QScriptValue split(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(ByteArray, split);
    return eng->toScriptValue(self->split(ctx->argument(0).toInt32()));
}

/////////////////////////////////////////////////////////////

static QScriptValue squeeze(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(ByteArray, squeeze);
    self->squeeze();
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue startsWith(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(ByteArray, startsWith);
    QScriptValue arg = ctx->argument(0);
    if (QByteArray *other = qscriptvalue_cast<QByteArray*>(arg))
        return QScriptValue(eng, self->startsWith(*other));
    else if (const char *str = qscriptvalue_cast<const char *>(arg))
        return QScriptValue(eng, self->startsWith(str));
    else
        return QScriptValue(eng, self->startsWith(arg.toInt32()));
}

/////////////////////////////////////////////////////////////

static QScriptValue toBase64(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(ByteArray, toBase64);
    return newByteArray(eng, self->toBase64());
}

/////////////////////////////////////////////////////////////

static QScriptValue toDouble(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(ByteArray, toDouble);
    return QScriptValue(eng, self->toDouble());
}

/////////////////////////////////////////////////////////////

static QScriptValue toFloat(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(ByteArray, toFloat);
    return QScriptValue(eng, self->toFloat());
}

/////////////////////////////////////////////////////////////

static QScriptValue toHex(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(ByteArray, toHex);
    return newByteArray(eng, self->toHex());
}

/////////////////////////////////////////////////////////////

static QScriptValue toInt(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(ByteArray, toInt);
    return QScriptValue(eng, self->toInt());
}

/////////////////////////////////////////////////////////////

#if 0
static QScriptValue toLongLong(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(ByteArray, toLongLong);
    return QScriptValue(eng, self->toLongLong());
}
#endif

/////////////////////////////////////////////////////////////

static QScriptValue toLower(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(ByteArray, toLower);
    return newByteArray(eng, self->toLower());
}

/////////////////////////////////////////////////////////////

static QScriptValue toShort(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(ByteArray, toShort);
    return QScriptValue(eng, self->toShort());
}

/////////////////////////////////////////////////////////////

static QScriptValue toUInt(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(ByteArray, toUInt);
    return QScriptValue(eng, self->toUInt());
}

/////////////////////////////////////////////////////////////

static QScriptValue toULong(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(ByteArray, toULong);
    return ctx->throwError("ByteArray.prototype.toULong is not implemented");
}

/////////////////////////////////////////////////////////////

#if 0
static QScriptValue toULongLong(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(ByteArray, toULongLong);
    return QScriptValue(eng, self->toULongLong());
}
#endif

/////////////////////////////////////////////////////////////

static QScriptValue toUShort(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(ByteArray, toUShort);
    return QScriptValue(eng, self->toUShort());
}

/////////////////////////////////////////////////////////////

static QScriptValue toUpper(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(ByteArray, toUpper);
    return newByteArray(eng, self->toUpper());    
}

/////////////////////////////////////////////////////////////

static QScriptValue trimmed(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(ByteArray, trimmed);
    return newByteArray(eng, self->trimmed());
}

/////////////////////////////////////////////////////////////

static QScriptValue truncate(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(ByteArray, truncate);
    self->truncate(ctx->argument(0).toInt32());
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue toString(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(ByteArray, toString);
    return QScriptValue(eng, "ByteArray");
}

/////////////////////////////////////////////////////////////

QScriptValue constructByteArrayClass(QScriptEngine *eng)
{
    QScriptValue proto = newByteArray(eng, QByteArray());
    ADD_PROTO_FUNCTION(proto, append);
    ADD_PROTO_FUNCTION(proto, at);
    ADD_PROTO_FUNCTION(proto, capacity);
    ADD_PROTO_FUNCTION(proto, chop);
    ADD_PROTO_FUNCTION(proto, clear);
    ADD_PROTO_FUNCTION(proto, constData);
    ADD_PROTO_FUNCTION(proto, contains);
    ADD_PROTO_FUNCTION(proto, count);
    ADD_PROTO_FUNCTION(proto, data);
    ADD_PROTO_FUNCTION(proto, endsWith);
    ADD_PROTO_FUNCTION(proto, fill);
    ADD_PROTO_FUNCTION(proto, indexOf);
    ADD_PROTO_FUNCTION(proto, insert);
    ADD_PROTO_FUNCTION(proto, isEmpty);
    ADD_PROTO_FUNCTION(proto, isNull);
    ADD_PROTO_FUNCTION(proto, lastIndexOf);
    ADD_PROTO_FUNCTION(proto, left);
    ADD_PROTO_FUNCTION(proto, leftJustified);
    ADD_PROTO_FUNCTION(proto, length);
    ADD_PROTO_FUNCTION(proto, mid);
    ADD_PROTO_FUNCTION(proto, prepend);
    ADD_PROTO_FUNCTION(proto, push_back);
    ADD_PROTO_FUNCTION(proto, push_front);
    ADD_PROTO_FUNCTION(proto, remove);
    ADD_PROTO_FUNCTION(proto, replace);
    ADD_PROTO_FUNCTION(proto, reserve);
    ADD_PROTO_FUNCTION(proto, resize);
    ADD_PROTO_FUNCTION(proto, right);
    ADD_PROTO_FUNCTION(proto, rightJustified);
    ADD_PROTO_FUNCTION(proto, setNum);
    ADD_PROTO_FUNCTION(proto, simplified);
    ADD_PROTO_FUNCTION(proto, size);
    ADD_PROTO_FUNCTION(proto, split);
    ADD_PROTO_FUNCTION(proto, squeeze);
    ADD_PROTO_FUNCTION(proto, startsWith);
    ADD_PROTO_FUNCTION(proto, toBase64);
    ADD_PROTO_FUNCTION(proto, toDouble);
    ADD_PROTO_FUNCTION(proto, toFloat);
    ADD_PROTO_FUNCTION(proto, toHex);
    ADD_PROTO_FUNCTION(proto, toInt);
    //ADD_PROTO_FUNCTION(proto, toLongLong);
    ADD_PROTO_FUNCTION(proto, toLower);
    ADD_PROTO_FUNCTION(proto, toShort);
    ADD_PROTO_FUNCTION(proto, toUInt);
    ADD_PROTO_FUNCTION(proto, toULong);
    //ADD_PROTO_FUNCTION(proto, toULongLong);
    ADD_PROTO_FUNCTION(proto, toUShort);
    ADD_PROTO_FUNCTION(proto, toUpper);
    ADD_PROTO_FUNCTION(proto, trimmed);
    ADD_PROTO_FUNCTION(proto, truncate);
    ADD_PROTO_FUNCTION(proto, toString);

    eng->setDefaultPrototype(qMetaTypeId<QByteArray>(), proto);
    eng->setDefaultPrototype(qMetaTypeId<QByteArray*>(), proto);

    return eng->newFunction(ctor, proto);
}
