#include <QtScript/QScriptValue>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptContext>
#include <QtGui/QBrush>
#include <QtGui/QPen>
#include <qdebug.h>
#include "../global.h"

Q_DECLARE_METATYPE(QBrush*)
Q_DECLARE_METATYPE(QColor*)
Q_DECLARE_METATYPE(QPen)
Q_DECLARE_METATYPE(QPen*)
Q_DECLARE_METATYPE(QVector<qreal>)

static inline QScriptValue newPen(QScriptEngine *eng, const QPen &pen)
{
    return eng->toScriptValue(pen);
}

/////////////////////////////////////////////////////////////

static QScriptValue ctor(QScriptContext *ctx, QScriptEngine *eng)
{
    if (ctx->argumentCount() == 0)
        return newPen(eng, QPen());

    QScriptValue arg = ctx->argument(0);
    if (QColor *color = qscriptvalue_cast<QColor*>(arg)) {
        return newPen(eng, QPen(*color));
    } else if (QBrush *brush = qscriptvalue_cast<QBrush*>(arg)) {
        // ### args
        int width = ctx->argument(1).toInt32();
        return newPen(eng, QPen(*brush, width));
    } else if (QPen *other = qscriptvalue_cast<QPen*>(arg)) {
        return newPen(eng, QPen(*other));
    }
    return ctx->throwError("QPen constructor: invalid argument(s)");
}

/////////////////////////////////////////////////////////////

static QScriptValue brush(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPen, brush);
    return eng->toScriptValue(self->brush());
}

/////////////////////////////////////////////////////////////

static QScriptValue capStyle(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPen, capStyle);
    return QScriptValue(eng, static_cast<int>(self->capStyle()));
}

/////////////////////////////////////////////////////////////

static QScriptValue color(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPen, color);
    return eng->toScriptValue(self->color());
}

/////////////////////////////////////////////////////////////

static QScriptValue dashOffset(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPen, dashOffset);
    return QScriptValue(eng, self->dashOffset());
}

/////////////////////////////////////////////////////////////

static QScriptValue dashPattern(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPen, dashPattern);
    return eng->toScriptValue(self->dashPattern());
}

/////////////////////////////////////////////////////////////

static QScriptValue isCosmetic(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPen, isCosmetic);
    return QScriptValue(eng, self->isCosmetic());
}

/////////////////////////////////////////////////////////////

static QScriptValue isSolid(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPen, isSolid);
    return QScriptValue(eng, self->isSolid());
}

/////////////////////////////////////////////////////////////

static QScriptValue joinStyle(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPen, joinStyle);
    return QScriptValue(eng, static_cast<int>(self->joinStyle()));
}

/////////////////////////////////////////////////////////////

static QScriptValue miterLimit(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPen, miterLimit);
    return QScriptValue(eng, self->miterLimit());
}

/////////////////////////////////////////////////////////////

static QScriptValue setBrush(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPen, setBrush);
    self->setBrush(qscriptvalue_cast<QBrush>(ctx->argument(0)));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setCapStyle(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPen, setCapStyle);
    self->setCapStyle(static_cast<Qt::PenCapStyle>(ctx->argument(0).toInt32()));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setColor(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPen, setColor);
    self->setBrush(qscriptvalue_cast<QColor>(ctx->argument(0)));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setCosmetic(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPen, setCosmetic);
    self->setCosmetic(ctx->argument(0).toBoolean());
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setDashOffset(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPen, setDashOffset);
    self->setDashOffset(ctx->argument(0).toNumber());
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setDashPattern(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPen, setDashPattern);
    self->setDashPattern(qscriptvalue_cast<QVector<qreal> >(ctx->argument(0)));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setJoinStyle(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPen, setJoinStyle);
    self->setJoinStyle(static_cast<Qt::PenJoinStyle>(ctx->argument(0).toInt32()));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setMiterLimit(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPen, setMiterLimit);
    self->setMiterLimit(ctx->argument(0).toNumber());
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setStyle(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPen, setStyle);
    self->setStyle(static_cast<Qt::PenStyle>(ctx->argument(0).toInt32()));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setWidth(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPen, setWidth);
    self->setWidthF(ctx->argument(0).toNumber());
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue style(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPen, style);
    return QScriptValue(eng, static_cast<int>(self->style()));
}

/////////////////////////////////////////////////////////////

static QScriptValue toString(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPen, toString);
    return QScriptValue(eng, QString::fromLatin1("QPen(width=%0)")
                        .arg(self->width()));
}

/////////////////////////////////////////////////////////////

static QScriptValue width(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPen, width);
    return QScriptValue(eng, self->widthF());
}

/////////////////////////////////////////////////////////////

static QScriptValue penToScriptValue(QScriptEngine *eng, const QPen &pen)
{
    return eng->newVariant(qVariantFromValue(pen));
}

static void penFromScriptValue(const QScriptValue &value, QPen &pen)
{
    if (!value.isVariant()) {
        pen = QPen();
    } else {
        QVariant var = value.toVariant();
        if (qVariantCanConvert<QPen>(var))
            pen = qvariant_cast<QPen>(var);
        else if (qVariantCanConvert<QColor>(var))
            pen = qvariant_cast<QColor>(var);
        else if (qVariantCanConvert<QPen*>(var))
            pen = *qvariant_cast<QPen*>(var);
        else if (qVariantCanConvert<QColor*>(var))
            pen = *qvariant_cast<QColor*>(var);
        else
            pen = QPen();
    }
}

QScriptValue constructPenClass(QScriptEngine *eng)
{
    QScriptValue proto = newPen(eng, QPen());
    ADD_METHOD(proto, brush);
    ADD_METHOD(proto, capStyle);
    ADD_METHOD(proto, color);
    ADD_METHOD(proto, dashOffset);
    ADD_METHOD(proto, dashPattern);
    ADD_METHOD(proto, isCosmetic);
    ADD_METHOD(proto, isSolid);
    ADD_METHOD(proto, joinStyle);
    ADD_METHOD(proto, miterLimit);
    ADD_METHOD(proto, setBrush);
    ADD_METHOD(proto, setCapStyle);
    ADD_METHOD(proto, setColor);
    ADD_METHOD(proto, setCosmetic);
    ADD_METHOD(proto, setDashOffset);
    ADD_METHOD(proto, setDashPattern);
    ADD_METHOD(proto, setJoinStyle);
    ADD_METHOD(proto, setMiterLimit);
    ADD_METHOD(proto, setStyle);
    ADD_METHOD(proto, setWidth);
    ADD_METHOD(proto, style);
    ADD_METHOD(proto, toString);
    ADD_METHOD(proto, width);

    qScriptRegisterMetaType<QPen>(eng, penToScriptValue, penFromScriptValue, proto);
    eng->setDefaultPrototype(qMetaTypeId<QPen*>(), proto);

    qScriptRegisterSequenceMetaType<QVector<qreal> >(eng);

    return eng->newFunction(ctor, proto);
}
