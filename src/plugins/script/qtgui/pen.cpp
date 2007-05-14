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
    DECLARE_SELF(Pen, brush);
    return eng->toScriptValue(self->brush());
}

/////////////////////////////////////////////////////////////

static QScriptValue capStyle(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Pen, capStyle);
    return QScriptValue(eng, static_cast<int>(self->capStyle()));
}

/////////////////////////////////////////////////////////////

static QScriptValue color(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Pen, color);
    return eng->toScriptValue(self->color());
}

/////////////////////////////////////////////////////////////

static QScriptValue dashOffset(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Pen, dashOffset);
    return QScriptValue(eng, self->dashOffset());
}

/////////////////////////////////////////////////////////////

static QScriptValue dashPattern(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Pen, dashPattern);
    return eng->toScriptValue(self->dashPattern());
}

/////////////////////////////////////////////////////////////

static QScriptValue isCosmetic(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Pen, isCosmetic);
    return QScriptValue(eng, self->isCosmetic());
}

/////////////////////////////////////////////////////////////

static QScriptValue isSolid(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Pen, isSolid);
    return QScriptValue(eng, self->isSolid());
}

/////////////////////////////////////////////////////////////

static QScriptValue joinStyle(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Pen, joinStyle);
    return QScriptValue(eng, static_cast<int>(self->joinStyle()));
}

/////////////////////////////////////////////////////////////

static QScriptValue miterLimit(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Pen, miterLimit);
    return QScriptValue(eng, self->miterLimit());
}

/////////////////////////////////////////////////////////////

static QScriptValue setBrush(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Pen, setBrush);
    self->setBrush(qscriptvalue_cast<QBrush>(ctx->argument(0)));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setCapStyle(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Pen, setCapStyle);
    self->setCapStyle(static_cast<Qt::PenCapStyle>(ctx->argument(0).toInt32()));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setColor(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Pen, setColor);
    self->setBrush(qscriptvalue_cast<QColor>(ctx->argument(0)));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setCosmetic(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Pen, setCosmetic);
    self->setCosmetic(ctx->argument(0).toBoolean());
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setDashOffset(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Pen, setDashOffset);
    self->setDashOffset(ctx->argument(0).toNumber());
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setDashPattern(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Pen, setDashPattern);
    self->setDashPattern(qscriptvalue_cast<QVector<qreal> >(ctx->argument(0)));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setJoinStyle(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Pen, setJoinStyle);
    self->setJoinStyle(static_cast<Qt::PenJoinStyle>(ctx->argument(0).toInt32()));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setMiterLimit(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Pen, setMiterLimit);
    self->setMiterLimit(ctx->argument(0).toNumber());
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setStyle(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Pen, setStyle);
    self->setStyle(static_cast<Qt::PenStyle>(ctx->argument(0).toInt32()));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setWidth(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Pen, setWidth);
    self->setWidthF(ctx->argument(0).toNumber());
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue style(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Pen, style);
    return QScriptValue(eng, static_cast<int>(self->style()));
}

/////////////////////////////////////////////////////////////

static QScriptValue toString(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Pen, toString);
    return QScriptValue(eng, QString::fromLatin1("QPen(width=%0)")
                        .arg(self->width()));
}

/////////////////////////////////////////////////////////////

static QScriptValue width(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Pen, width);
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
    ADD_PROTO_FUNCTION(proto, brush);
    ADD_PROTO_FUNCTION(proto, capStyle);
    ADD_PROTO_FUNCTION(proto, color);
    ADD_PROTO_FUNCTION(proto, dashOffset);
    ADD_PROTO_FUNCTION(proto, dashPattern);
    ADD_PROTO_FUNCTION(proto, isCosmetic);
    ADD_PROTO_FUNCTION(proto, isSolid);
    ADD_PROTO_FUNCTION(proto, joinStyle);
    ADD_PROTO_FUNCTION(proto, miterLimit);
    ADD_PROTO_FUNCTION(proto, setBrush);
    ADD_PROTO_FUNCTION(proto, setCapStyle);
    ADD_PROTO_FUNCTION(proto, setColor);
    ADD_PROTO_FUNCTION(proto, setCosmetic);
    ADD_PROTO_FUNCTION(proto, setDashOffset);
    ADD_PROTO_FUNCTION(proto, setDashPattern);
    ADD_PROTO_FUNCTION(proto, setJoinStyle);
    ADD_PROTO_FUNCTION(proto, setMiterLimit);
    ADD_PROTO_FUNCTION(proto, setStyle);
    ADD_PROTO_FUNCTION(proto, setWidth);
    ADD_PROTO_FUNCTION(proto, style);
    ADD_PROTO_FUNCTION(proto, toString);
    ADD_PROTO_FUNCTION(proto, width);

    qScriptRegisterMetaType<QPen>(eng, penToScriptValue, penFromScriptValue, proto);
    eng->setDefaultPrototype(qMetaTypeId<QPen*>(), proto);

    qScriptRegisterSequenceMetaType<QVector<qreal> >(eng);

    return eng->newFunction(ctor, proto);
}
