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
    DECLARE_SELF(QModelIndex, child);
    return newModelIndex(eng, self->child(ctx->argument(0).toInt32(),
                                          ctx->argument(1).toInt32()));
}

/////////////////////////////////////////////////////////////

static QScriptValue column(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QModelIndex, column);
    return QScriptValue(eng, self->column());
}

/////////////////////////////////////////////////////////////

static QScriptValue data(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QModelIndex, data);
    if (ctx->argumentCount() == 0)
        return eng->newVariant(self->data());
    else
        return eng->newVariant(self->data(ctx->argument(0).toInt32()));
}

/////////////////////////////////////////////////////////////

static QScriptValue flags(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QModelIndex, flags);
    return QScriptValue(eng, static_cast<int>(self->flags()));
}

/////////////////////////////////////////////////////////////

static QScriptValue internalId(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QModelIndex, internalId);
    return eng->toScriptValue(self->internalId());
}

/////////////////////////////////////////////////////////////

static QScriptValue internalPointer(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QModelIndex, internalPointer);
    return eng->toScriptValue(self->internalPointer());
}

/////////////////////////////////////////////////////////////

static QScriptValue isValid(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QModelIndex, isValid);
    return QScriptValue(eng, self->isValid());
}

/////////////////////////////////////////////////////////////

static QScriptValue model(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QModelIndex, model);
    return eng->newQObject(const_cast<QAbstractItemModel*>(self->model()));
}

/////////////////////////////////////////////////////////////

static QScriptValue parent(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QModelIndex, parent);
    return newModelIndex(eng, self->parent());
}

/////////////////////////////////////////////////////////////

static QScriptValue row(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QModelIndex, row);
    return QScriptValue(eng, self->row());
}

/////////////////////////////////////////////////////////////

static QScriptValue sibling(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QModelIndex, sibling);
    return newModelIndex(eng, self->sibling(ctx->argument(0).toInt32(),
                                            ctx->argument(1).toInt32()));
}

/////////////////////////////////////////////////////////////

static QScriptValue toString(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QModelIndex, toString);
    return QScriptValue(eng, QString::fromLatin1("QModelIndex(%0,%1)")
        .arg(self->row()).arg(self->column()));
}

/////////////////////////////////////////////////////////////

QScriptValue constructModelIndexClass(QScriptEngine *eng)
{
    QScriptValue proto = newModelIndex(eng, QModelIndex());
    ADD_METHOD(proto, child);
    ADD_METHOD(proto, column);
    ADD_METHOD(proto, data);
    ADD_METHOD(proto, flags);
    ADD_METHOD(proto, internalId);
    ADD_METHOD(proto, internalPointer);
    ADD_METHOD(proto, isValid);
    ADD_METHOD(proto, model);
    ADD_METHOD(proto, parent);
    ADD_METHOD(proto, row);
    ADD_METHOD(proto, sibling);
    ADD_METHOD(proto, toString);

    eng->setDefaultPrototype(qMetaTypeId<QModelIndex>(), proto);
    eng->setDefaultPrototype(qMetaTypeId<QModelIndex*>(), proto);

    return eng->newFunction(ctor, proto);
}
