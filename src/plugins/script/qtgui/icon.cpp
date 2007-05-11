#include <QtScript/QScriptValue>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptContext>
#include <QtGui/QIcon>
#include "../global.h"

Q_DECLARE_METATYPE(QIcon)
Q_DECLARE_METATYPE(QIcon*)
Q_DECLARE_METATYPE(QIconEngine*)
Q_DECLARE_METATYPE(QIconEngineV2*)
Q_DECLARE_METATYPE(QPixmap*)
Q_DECLARE_METATYPE(QPainter*)

static inline QScriptValue newIcon(QScriptEngine *eng, const QIcon &icon)
{
    return eng->newVariant(qVariantFromValue(icon));
}

/////////////////////////////////////////////////////////////

static QScriptValue ctor(QScriptContext *ctx, QScriptEngine *eng)
{
    QScriptValue arg = ctx->argument(0);
    if (arg.isUndefined())
        return newIcon(eng, QIcon());
    if (QPixmap *pixmap = qscriptvalue_cast<QPixmap*>(arg))
        return newIcon(eng, QIcon(*pixmap));
    if (QIcon *other = qscriptvalue_cast<QIcon*>(arg))
        return newIcon(eng, QIcon(*other));
    if (QIconEngine *engine = qscriptvalue_cast<QIconEngine*>(arg))
        return newIcon(eng, QIcon(engine));
    if (QIconEngineV2 *engine = qscriptvalue_cast<QIconEngineV2*>(arg))
        return newIcon(eng, QIcon(engine));
    return newIcon(eng, QIcon(arg.toString()));
}

/////////////////////////////////////////////////////////////

static QScriptValue actualSize(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Icon, actualSize);
    QSize size = qscriptvalue_cast<QSize>(ctx->argument(0));
    if (ctx->argumentCount() >= 3) {
        QIcon::Mode mode = static_cast<QIcon::Mode>(ctx->argument(1).toInt32());
        QIcon::State state = static_cast<QIcon::State>(ctx->argument(2).toInt32());
        return eng->toScriptValue(self->actualSize(size, mode, state));
    } else if (ctx->argumentCount() == 2) {
        QIcon::Mode mode = static_cast<QIcon::Mode>(ctx->argument(1).toInt32());
        return eng->toScriptValue(self->actualSize(size, mode));
    } else {
        return eng->toScriptValue(self->actualSize(size));
    }
}

/////////////////////////////////////////////////////////////

static QScriptValue addFile(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Icon, addFile);
    QString fileName = ctx->argument(0).toString();
    if (ctx->argumentCount() >= 4) {
        QSize size = qscriptvalue_cast<QSize>(ctx->argument(1));
        QIcon::Mode mode = static_cast<QIcon::Mode>(ctx->argument(2).toInt32());
        QIcon::State state = static_cast<QIcon::State>(ctx->argument(3).toInt32());
        self->addFile(fileName, size, mode, state);
    } else if (ctx->argumentCount() == 3) {
        QSize size = qscriptvalue_cast<QSize>(ctx->argument(1));
        QIcon::Mode mode = static_cast<QIcon::Mode>(ctx->argument(2).toInt32());
        self->addFile(fileName, size, mode);
    } else if (ctx->argumentCount() == 2) {
        QSize size = qscriptvalue_cast<QSize>(ctx->argument(1));
        self->addFile(fileName, size);
    } else {
        self->addFile(fileName);
    }
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue addPixmap(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Icon, addPixmap);
    QPixmap pixmap = qscriptvalue_cast<QPixmap>(ctx->argument(0));
    if (ctx->argumentCount() >= 3) {
        QIcon::Mode mode = static_cast<QIcon::Mode>(ctx->argument(1).toInt32());
        QIcon::State state = static_cast<QIcon::State>(ctx->argument(2).toInt32());
        self->addPixmap(pixmap, mode, state);
    } else if (ctx->argumentCount() == 2) {
        QIcon::Mode mode = static_cast<QIcon::Mode>(ctx->argument(1).toInt32());
        self->addPixmap(pixmap, mode);
    } else {
        self->addPixmap(pixmap);
    }
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue cacheKey(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Icon, cacheKey);
    return eng->toScriptValue(self->cacheKey());
}

/////////////////////////////////////////////////////////////

static QScriptValue isNull(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Icon, isNull);
    return QScriptValue(eng, self->isNull());
}

/////////////////////////////////////////////////////////////

static QScriptValue paint(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Icon, paint);
    QPainter *painter = qscriptvalue_cast<QPainter*>(ctx->argument(0));
    if (!painter) {
        return ctx->throwError(QScriptContext::TypeError,
                               "Icon.prototype.paint: argument is not a Painter");
    }
    QRect rect = qscriptvalue_cast<QRect>(ctx->argument(1));
    if (ctx->argumentCount() >= 5) {
        Qt::Alignment alignment = static_cast<Qt::Alignment>(ctx->argument(2).toInt32());
        QIcon::Mode mode = static_cast<QIcon::Mode>(ctx->argument(3).toInt32());
        QIcon::State state = static_cast<QIcon::State>(ctx->argument(4).toInt32());
        self->paint(painter, rect, alignment, mode, state);
    } else if (ctx->argumentCount() == 4) {
        Qt::Alignment alignment = static_cast<Qt::Alignment>(ctx->argument(2).toInt32());
        QIcon::Mode mode = static_cast<QIcon::Mode>(ctx->argument(3).toInt32());
        self->paint(painter, rect, alignment, mode);
    } else if (ctx->argumentCount() == 3) {
        Qt::Alignment alignment = static_cast<Qt::Alignment>(ctx->argument(2).toInt32());
        self->paint(painter, rect, alignment);
    } else {
        self->paint(painter, rect);
    }
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue pixmap(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Icon, pixmap);
    QSize size = qscriptvalue_cast<QSize>(ctx->argument(0));
    // ### overloads
    return eng->toScriptValue(self->pixmap(size));
}

/////////////////////////////////////////////////////////////

static QScriptValue toString(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Icon, toString);
    return QScriptValue(eng, "Icon");
}

/////////////////////////////////////////////////////////////

QScriptValue constructIconClass(QScriptEngine *eng)
{
    QScriptValue proto = newIcon(eng, QIcon());
    ADD_PROTO_FUNCTION(proto, actualSize);
    ADD_PROTO_FUNCTION(proto, addFile);
    ADD_PROTO_FUNCTION(proto, addPixmap);
    ADD_PROTO_FUNCTION(proto, cacheKey);
    ADD_PROTO_FUNCTION(proto, isNull);
    ADD_PROTO_FUNCTION(proto, paint);
    ADD_PROTO_FUNCTION(proto, pixmap);
    ADD_PROTO_FUNCTION(proto, toString);

    eng->setDefaultPrototype(qMetaTypeId<QIcon>(), proto);
    eng->setDefaultPrototype(qMetaTypeId<QIcon*>(), proto);

    return eng->newFunction(ctor, proto);
}
