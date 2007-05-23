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
    return ctx->throwError("QBrush constructor: invalid argument");
}

/////////////////////////////////////////////////////////////

static QScriptValue color(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QBrush, color);
    return eng->toScriptValue(self->color());
}

/////////////////////////////////////////////////////////////

static QScriptValue gradient(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QBrush, gradient);
    return eng->toScriptValue(const_cast<QGradient*>(self->gradient()));
}

/////////////////////////////////////////////////////////////

static QScriptValue isOpaque(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QBrush, isOpaque);
    return QScriptValue(eng, self->isOpaque());
}

/////////////////////////////////////////////////////////////

static QScriptValue matrix(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QBrush, matrix);
    return eng->toScriptValue(self->matrix());
}

/////////////////////////////////////////////////////////////

static QScriptValue setColor(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QBrush, setColor);
    self->setColor(qscriptvalue_cast<QColor>(ctx->argument(0)));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setMatrix(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QBrush, setMatrix);
    self->setMatrix(qscriptvalue_cast<QMatrix>(ctx->argument(0)));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setStyle(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QBrush, setStyle);
    self->setStyle(static_cast<Qt::BrushStyle>(ctx->argument(0).toInt32()));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setTexture(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QBrush, setTexture);
    self->setTexture(qscriptvalue_cast<QPixmap>(ctx->argument(0)));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setTextureImage(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QBrush, setTextureImage);
    self->setTextureImage(qscriptvalue_cast<QImage>(ctx->argument(0)));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setTransform(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QBrush, setTransform);
    self->setTransform(qscriptvalue_cast<QTransform>(ctx->argument(0)));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue style(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QBrush, style);
    return QScriptValue(eng, static_cast<int>(self->style()));
}

/////////////////////////////////////////////////////////////

static QScriptValue texture(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QBrush, texture);
    return eng->toScriptValue(self->texture());
}

/////////////////////////////////////////////////////////////

static QScriptValue toString(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QBrush, toString);
    return QScriptValue(eng, "QBrush");
}

/////////////////////////////////////////////////////////////

static QScriptValue textureImage(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QBrush, textureImage);
    return eng->toScriptValue(self->textureImage());
}

/////////////////////////////////////////////////////////////

static QScriptValue transform(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QBrush, transform);
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
    ADD_METHOD(proto, color);
    ADD_METHOD(proto, gradient);
    ADD_METHOD(proto, isOpaque);
    ADD_METHOD(proto, matrix);
    ADD_METHOD(proto, setColor);
    ADD_METHOD(proto, setMatrix);
    ADD_METHOD(proto, setStyle);
    ADD_METHOD(proto, setTexture);
    ADD_METHOD(proto, setTextureImage);
    ADD_METHOD(proto, setTransform);
    ADD_METHOD(proto, style);
    ADD_METHOD(proto, texture);
    ADD_METHOD(proto, textureImage);
    ADD_METHOD(proto, toString);
    ADD_METHOD(proto, transform);

    qScriptRegisterMetaType<QBrush>(eng, brushToScriptValue, brushFromScriptValue, proto);
    eng->setDefaultPrototype(qMetaTypeId<QBrush*>(), proto);

    return eng->newFunction(ctor, proto);
}
