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
    DECLARE_SELF(QByteArray, append);
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
    DECLARE_SELF(QByteArray, at);
    return QScriptValue(eng, self->at(ctx->argument(0).toInt32()));
}

/////////////////////////////////////////////////////////////

static QScriptValue capacity(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QByteArray, capacity);
    return QScriptValue(eng, self->capacity());
}

/////////////////////////////////////////////////////////////

static QScriptValue chop(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QByteArray, chop);
    self->chop(ctx->argument(0).toInt32());
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue clear(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QByteArray, clear);
    self->clear();
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue constData(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QByteArray, constData);
    return eng->toScriptValue(self->constData());
}

/////////////////////////////////////////////////////////////

static QScriptValue contains(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QByteArray, contains);
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
    DECLARE_SELF(QByteArray, count);
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
    DECLARE_SELF(QByteArray, data);
    return ctx->throwError("QByteArray.prototype.data is not implemented");
}

/////////////////////////////////////////////////////////////

static QScriptValue endsWith(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QByteArray, endsWith);
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
    DECLARE_SELF(QByteArray, fill);
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
    DECLARE_SELF(QByteArray, indexOf);
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
    DECLARE_SELF(QByteArray, insert);
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
    DECLARE_SELF(QByteArray, isEmpty);
    return QScriptValue(eng, self->isEmpty());
}

/////////////////////////////////////////////////////////////

static QScriptValue isNull(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QByteArray, isNull);
    return QScriptValue(eng, self->isNull());
}

/////////////////////////////////////////////////////////////

static QScriptValue lastIndexOf(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QByteArray, lastIndexOf);
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
    DECLARE_SELF(QByteArray, left);
    return newByteArray(eng, self->left(ctx->argument(0).toInt32()));
}

/////////////////////////////////////////////////////////////

static QScriptValue leftJustified(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(QByteArray, leftJustified);
    return ctx->throwError("QByteArray.prototype.leftJustified is not implemented");
}

/////////////////////////////////////////////////////////////

static QScriptValue length(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QByteArray, length);
    return QScriptValue(eng, self->length());
}

/////////////////////////////////////////////////////////////

static QScriptValue mid(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QByteArray, mid);
    int pos = ctx->argument(0).toInt32();
    if (ctx->argument(1).isUndefined())
        return newByteArray(eng, self->mid(pos));
    else
        return newByteArray(eng, self->mid(pos, ctx->argument(1).toInt32()));
}

/////////////////////////////////////////////////////////////

static QScriptValue prepend(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(QByteArray, prepend);
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
    DECLARE_SELF(QByteArray, push_back);
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
    DECLARE_SELF(QByteArray, push_front);
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
    DECLARE_SELF(QByteArray, remove);
    self->remove(ctx->argument(0).toInt32(), ctx->argument(1).toInt32());
    return ctx->thisObject();
}

/////////////////////////////////////////////////////////////

static QScriptValue replace(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(QByteArray, replace);
    return ctx->throwError("QByteArray.prototype.replace is not implemented");
}

/////////////////////////////////////////////////////////////

static QScriptValue reserve(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QByteArray, reserve);
    self->reserve(ctx->argument(0).toInt32());
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue resize(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QByteArray, resize);
    self->resize(ctx->argument(0).toInt32());
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue right(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QByteArray, right);
    return newByteArray(eng, self->right(ctx->argument(0).toInt32()));
}

/////////////////////////////////////////////////////////////

static QScriptValue rightJustified(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(QByteArray, rightJustified);
    return ctx->throwError("QByteArray.prototype.rightJustified is not implemented");
}

/////////////////////////////////////////////////////////////

static QScriptValue setNum(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(QByteArray, setNum);
    return ctx->throwError("QByteArray.prototype.setNum is not implemented");
}

/////////////////////////////////////////////////////////////

static QScriptValue simplified(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QByteArray, simplified);
    return newByteArray(eng, self->simplified());
}

/////////////////////////////////////////////////////////////

static QScriptValue size(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QByteArray, size);
    return QScriptValue(eng, self->size());
}

/////////////////////////////////////////////////////////////

static QScriptValue split(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QByteArray, split);
    return eng->toScriptValue(self->split(ctx->argument(0).toInt32()));
}

/////////////////////////////////////////////////////////////

static QScriptValue squeeze(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QByteArray, squeeze);
    self->squeeze();
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue startsWith(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QByteArray, startsWith);
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
    DECLARE_SELF(QByteArray, toBase64);
    return newByteArray(eng, self->toBase64());
}

/////////////////////////////////////////////////////////////

static QScriptValue toDouble(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QByteArray, toDouble);
    return QScriptValue(eng, self->toDouble());
}

/////////////////////////////////////////////////////////////

static QScriptValue toFloat(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QByteArray, toFloat);
    return QScriptValue(eng, self->toFloat());
}

/////////////////////////////////////////////////////////////

static QScriptValue toHex(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QByteArray, toHex);
    return newByteArray(eng, self->toHex());
}

/////////////////////////////////////////////////////////////

static QScriptValue toInt(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QByteArray, toInt);
    return QScriptValue(eng, self->toInt());
}

/////////////////////////////////////////////////////////////

#if 0
static QScriptValue toLongLong(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QByteArray, toLongLong);
    return QScriptValue(eng, self->toLongLong());
}
#endif

/////////////////////////////////////////////////////////////

static QScriptValue toLower(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QByteArray, toLower);
    return newByteArray(eng, self->toLower());
}

/////////////////////////////////////////////////////////////

static QScriptValue toShort(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QByteArray, toShort);
    return QScriptValue(eng, self->toShort());
}

/////////////////////////////////////////////////////////////

static QScriptValue toUInt(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QByteArray, toUInt);
    return QScriptValue(eng, self->toUInt());
}

/////////////////////////////////////////////////////////////

static QScriptValue toULong(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(QByteArray, toULong);
    return ctx->throwError("QByteArray.prototype.toULong is not implemented");
}

/////////////////////////////////////////////////////////////

#if 0
static QScriptValue toULongLong(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QByteArray, toULongLong);
    return QScriptValue(eng, self->toULongLong());
}
#endif

/////////////////////////////////////////////////////////////

static QScriptValue toUShort(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QByteArray, toUShort);
    return QScriptValue(eng, self->toUShort());
}

/////////////////////////////////////////////////////////////

static QScriptValue toUpper(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QByteArray, toUpper);
    return newByteArray(eng, self->toUpper());    
}

/////////////////////////////////////////////////////////////

static QScriptValue trimmed(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QByteArray, trimmed);
    return newByteArray(eng, self->trimmed());
}

/////////////////////////////////////////////////////////////

static QScriptValue truncate(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QByteArray, truncate);
    self->truncate(ctx->argument(0).toInt32());
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue toString(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QByteArray, toString);
    return QScriptValue(eng, QString::fromLocal8Bit(self->constData(), self->size()));
}

/////////////////////////////////////////////////////////////

QScriptValue constructByteArrayClass(QScriptEngine *eng)
{
    QScriptValue proto = newByteArray(eng, QByteArray());
    ADD_METHOD(proto, append);
    ADD_METHOD(proto, at);
    ADD_METHOD(proto, capacity);
    ADD_METHOD(proto, chop);
    ADD_METHOD(proto, clear);
    ADD_METHOD(proto, constData);
    ADD_METHOD(proto, contains);
    ADD_METHOD(proto, count);
    ADD_METHOD(proto, data);
    ADD_METHOD(proto, endsWith);
    ADD_METHOD(proto, fill);
    ADD_METHOD(proto, indexOf);
    ADD_METHOD(proto, insert);
    ADD_METHOD(proto, isEmpty);
    ADD_METHOD(proto, isNull);
    ADD_METHOD(proto, lastIndexOf);
    ADD_METHOD(proto, left);
    ADD_METHOD(proto, leftJustified);
    ADD_METHOD(proto, length);
    ADD_METHOD(proto, mid);
    ADD_METHOD(proto, prepend);
    ADD_METHOD(proto, push_back);
    ADD_METHOD(proto, push_front);
    ADD_METHOD(proto, remove);
    ADD_METHOD(proto, replace);
    ADD_METHOD(proto, reserve);
    ADD_METHOD(proto, resize);
    ADD_METHOD(proto, right);
    ADD_METHOD(proto, rightJustified);
    ADD_METHOD(proto, setNum);
    ADD_METHOD(proto, simplified);
    ADD_METHOD(proto, size);
    ADD_METHOD(proto, split);
    ADD_METHOD(proto, squeeze);
    ADD_METHOD(proto, startsWith);
    ADD_METHOD(proto, toBase64);
    ADD_METHOD(proto, toDouble);
    ADD_METHOD(proto, toFloat);
    ADD_METHOD(proto, toHex);
    ADD_METHOD(proto, toInt);
    //ADD_METHOD(proto, toLongLong);
    ADD_METHOD(proto, toLower);
    ADD_METHOD(proto, toShort);
    ADD_METHOD(proto, toUInt);
    ADD_METHOD(proto, toULong);
    //ADD_METHOD(proto, toULongLong);
    ADD_METHOD(proto, toUShort);
    ADD_METHOD(proto, toUpper);
    ADD_METHOD(proto, trimmed);
    ADD_METHOD(proto, truncate);
    ADD_METHOD(proto, toString);

    eng->setDefaultPrototype(qMetaTypeId<QByteArray>(), proto);
    eng->setDefaultPrototype(qMetaTypeId<QByteArray*>(), proto);

    return eng->newFunction(ctor, proto);
}
