#include <QtScript/QScriptValue>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptContext>
#include <QtCore/QModelIndex>
#include "../global.h"

Q_DECLARE_METATYPE(QModelIndex)
Q_DECLARE_METATYPE(QModelIndex*)

static inline QScriptValue newModelIndex(QScriptEngine *eng, const QModelIndex &index)
{
    return eng->newVariant(qVariantFromValue(index));
}

/////////////////////////////////////////////////////////////

static QScriptValue ctor(QScriptContext *ctx, QScriptEngine *eng)
{
    if (ctx->argumentCount() == 0)
        return newModelIndex(eng, QModelIndex());
    return newModelIndex(eng, QModelIndex(qscriptvalue_cast<QModelIndex>(ctx->argument(0))));
}

/////////////////////////////////////////////////////////////

static QScriptValue child(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(ModelIndex, child);
    return newModelIndex(eng, self->child(ctx->argument(0).toInt32(),
                                          ctx->argument(1).toInt32()));
}

/////////////////////////////////////////////////////////////

static QScriptValue column(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(ModelIndex, column);
    return QScriptValue(eng, self->column());
}

/////////////////////////////////////////////////////////////

static QScriptValue data(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(ModelIndex, data);
    if (ctx->argumentCount() == 0)
        return eng->newVariant(self->data());
    else
        return eng->newVariant(self->data(ctx->argument(0).toInt32()));
}

/////////////////////////////////////////////////////////////

static QScriptValue flags(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(ModelIndex, flags);
    return QScriptValue(eng, static_cast<int>(self->flags()));
}

/////////////////////////////////////////////////////////////

static QScriptValue internalId(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(ModelIndex, internalId);
    return eng->toScriptValue(self->internalId());
}

/////////////////////////////////////////////////////////////

static QScriptValue internalPointer(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(ModelIndex, internalPointer);
    return eng->toScriptValue(self->internalPointer());
}

/////////////////////////////////////////////////////////////

static QScriptValue isValid(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(ModelIndex, isValid);
    return QScriptValue(eng, self->isValid());
}

/////////////////////////////////////////////////////////////

static QScriptValue model(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(ModelIndex, model);
    return eng->newQObject(const_cast<QAbstractItemModel*>(self->model()));
}

/////////////////////////////////////////////////////////////

static QScriptValue parent(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(ModelIndex, parent);
    return newModelIndex(eng, self->parent());
}

/////////////////////////////////////////////////////////////

static QScriptValue row(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(ModelIndex, row);
    return QScriptValue(eng, self->row());
}

/////////////////////////////////////////////////////////////

static QScriptValue sibling(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(ModelIndex, sibling);
    return newModelIndex(eng, self->sibling(ctx->argument(0).toInt32(),
                                            ctx->argument(1).toInt32()));
}

/////////////////////////////////////////////////////////////

static QScriptValue toString(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(ModelIndex, toString);
    return QScriptValue(eng, QString::fromLatin1("QModelIndex(%0,%1)")
        .arg(self->row()).arg(self->column()));
}

/////////////////////////////////////////////////////////////

QScriptValue constructModelIndexClass(QScriptEngine *eng)
{
    QScriptValue proto = newModelIndex(eng, QModelIndex());
    ADD_PROTO_FUNCTION(proto, child);
    ADD_PROTO_FUNCTION(proto, column);
    ADD_PROTO_FUNCTION(proto, data);
    ADD_PROTO_FUNCTION(proto, flags);
    ADD_PROTO_FUNCTION(proto, internalId);
    ADD_PROTO_FUNCTION(proto, internalPointer);
    ADD_PROTO_FUNCTION(proto, isValid);
    ADD_PROTO_FUNCTION(proto, model);
    ADD_PROTO_FUNCTION(proto, parent);
    ADD_PROTO_FUNCTION(proto, row);
    ADD_PROTO_FUNCTION(proto, sibling);
    ADD_PROTO_FUNCTION(proto, toString);

    eng->setDefaultPrototype(qMetaTypeId<QModelIndex>(), proto);
    eng->setDefaultPrototype(qMetaTypeId<QModelIndex*>(), proto);

    return eng->newFunction(ctor, proto);
}
