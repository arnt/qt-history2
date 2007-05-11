#include <QtScript/QScriptValue>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptContext>
#include <QtGui/QBrush>
#include "../global.h"

Q_DECLARE_METATYPE(QGradient)
Q_DECLARE_METATYPE(QGradient*)
Q_DECLARE_METATYPE(QConicalGradient)
Q_DECLARE_METATYPE(QConicalGradient*)
Q_DECLARE_METATYPE(QLinearGradient)
Q_DECLARE_METATYPE(QLinearGradient*)
Q_DECLARE_METATYPE(QRadialGradient)
Q_DECLARE_METATYPE(QRadialGradient*)
Q_DECLARE_METATYPE(QColor*)
Q_DECLARE_METATYPE(QPixmap*)
Q_DECLARE_METATYPE(QImage*)
Q_DECLARE_METATYPE(QBrush*)

static inline QScriptValue newBrush(QScriptEngine *eng, const QBrush &brush)
{
    return eng->newVariant(qVariantFromValue(brush));
}

/////////////////////////////////////////////////////////////

static QScriptValue ctor(QScriptContext *ctx, QScriptEngine *eng)
{
    if (ctx->argumentCount() == 0)
        return newBrush(eng, QBrush());

    QScriptValue arg = ctx->argument(0);
    if (QColor *color = qscriptvalue_cast<QColor*>(arg)) {
        return newBrush(eng, QBrush(*color));
    } else if (QPixmap *pixmap = qscriptvalue_cast<QPixmap*>(arg)) {
        return newBrush(eng, QBrush(*pixmap));
    } else if (QImage *image = qscriptvalue_cast<QImage*>(arg)) {
        return newBrush(eng, QBrush(*image));
    } else if (QBrush *other = qscriptvalue_cast<QBrush*>(arg)) {
        return newBrush(eng, QBrush(*other));
    } else if (QGradient *gradient = qscriptvalue_cast<QGradient*>(arg)) {
        return newBrush(eng, QBrush(*gradient));
    }
    return ctx->throwError("Brush constructor: invalid argument");
}

/////////////////////////////////////////////////////////////

static QScriptValue color(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Brush, color);
    return eng->toScriptValue(self->color());
}

/////////////////////////////////////////////////////////////

static QScriptValue gradient(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Brush, gradient);
    return eng->toScriptValue(const_cast<QGradient*>(self->gradient()));
}

/////////////////////////////////////////////////////////////

static QScriptValue isOpaque(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Brush, isOpaque);
    return QScriptValue(eng, self->isOpaque());
}

/////////////////////////////////////////////////////////////

static QScriptValue matrix(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Brush, matrix);
    return eng->toScriptValue(self->matrix());
}

/////////////////////////////////////////////////////////////

static QScriptValue setColor(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Brush, setColor);
    self->setColor(qscriptvalue_cast<QColor>(ctx->argument(0)));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setMatrix(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Brush, setMatrix);
    self->setMatrix(qscriptvalue_cast<QMatrix>(ctx->argument(0)));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setStyle(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Brush, setStyle);
    self->setStyle(static_cast<Qt::BrushStyle>(ctx->argument(0).toInt32()));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setTexture(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Brush, setTexture);
    self->setTexture(qscriptvalue_cast<QPixmap>(ctx->argument(0)));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setTextureImage(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Brush, setTextureImage);
    self->setTextureImage(qscriptvalue_cast<QImage>(ctx->argument(0)));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setTransform(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Brush, setTransform);
    self->setTransform(qscriptvalue_cast<QTransform>(ctx->argument(0)));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue style(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Brush, style);
    return QScriptValue(eng, static_cast<int>(self->style()));
}

/////////////////////////////////////////////////////////////

static QScriptValue texture(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Brush, texture);
    return eng->toScriptValue(self->texture());
}

/////////////////////////////////////////////////////////////

static QScriptValue toString(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Brush, toString);
    return QScriptValue(eng, "Brush");
}

/////////////////////////////////////////////////////////////

static QScriptValue textureImage(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Brush, textureImage);
    return eng->toScriptValue(self->textureImage());
}

/////////////////////////////////////////////////////////////

static QScriptValue transform(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Brush, transform);
    return eng->toScriptValue(self->transform());
}

/////////////////////////////////////////////////////////////

static QScriptValue brushToScriptValue(QScriptEngine *eng, const QBrush &brush)
{
    return eng->newVariant(qVariantFromValue(brush));
}

static void brushFromScriptValue(const QScriptValue &value, QBrush &brush)
{
    if (!value.isVariant()) {
        brush = QBrush();
    } else {
        QVariant var = value.toVariant();
        if (qVariantCanConvert<QBrush>(var))
            brush = qvariant_cast<QBrush>(var);
        else if (qVariantCanConvert<QGradient>(var))
            brush = qvariant_cast<QGradient>(var);
        else if (qVariantCanConvert<QConicalGradient>(var))
            brush = qvariant_cast<QConicalGradient>(var);
        else if (qVariantCanConvert<QLinearGradient>(var))
            brush = qvariant_cast<QLinearGradient>(var);
        else if (qVariantCanConvert<QRadialGradient>(var))
            brush = qvariant_cast<QRadialGradient>(var);
        else if (qVariantCanConvert<QBrush*>(var))
            brush = *qvariant_cast<QBrush*>(var);
        else if (qVariantCanConvert<QGradient*>(var))
            brush = *qvariant_cast<QGradient*>(var);
        else if (qVariantCanConvert<QConicalGradient*>(var))
            brush = *qvariant_cast<QConicalGradient*>(var);
        else if (qVariantCanConvert<QLinearGradient*>(var))
            brush = *qvariant_cast<QLinearGradient*>(var);
        else if (qVariantCanConvert<QRadialGradient*>(var))
            brush = *qvariant_cast<QRadialGradient*>(var);
        else
            brush = QBrush();
    }
}

QScriptValue constructBrushClass(QScriptEngine *eng)
{
    QScriptValue proto = newBrush(eng, QBrush());
    ADD_PROTO_FUNCTION(proto, color);
    ADD_PROTO_FUNCTION(proto, gradient);
    ADD_PROTO_FUNCTION(proto, isOpaque);
    ADD_PROTO_FUNCTION(proto, matrix);
    ADD_PROTO_FUNCTION(proto, setColor);
    ADD_PROTO_FUNCTION(proto, setMatrix);
    ADD_PROTO_FUNCTION(proto, setStyle);
    ADD_PROTO_FUNCTION(proto, setTexture);
    ADD_PROTO_FUNCTION(proto, setTextureImage);
    ADD_PROTO_FUNCTION(proto, setTransform);
    ADD_PROTO_FUNCTION(proto, style);
    ADD_PROTO_FUNCTION(proto, texture);
    ADD_PROTO_FUNCTION(proto, textureImage);
    ADD_PROTO_FUNCTION(proto, toString);
    ADD_PROTO_FUNCTION(proto, transform);

    qScriptRegisterMetaType<QBrush>(eng, brushToScriptValue, brushFromScriptValue, proto);
    eng->setDefaultPrototype(qMetaTypeId<QBrush*>(), proto);

    return eng->newFunction(ctor, proto);
}
