#include <QtScript/QScriptValue>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptContext>
#include <QtGui/QColor>
#include "../global.h"

Q_DECLARE_METATYPE(QColor)
Q_DECLARE_METATYPE(QColor*)

static inline QScriptValue newColor(QScriptEngine *eng, const QColor &color)
{
    return eng->newVariant(qVariantFromValue(color));
}

static QScriptValue ctor(QScriptContext *ctx, QScriptEngine *eng)
{
    if (ctx->argumentCount() == 0)
        return newColor(eng, QColor());
    if (ctx->argumentCount() == 1) {
        QScriptValue arg = ctx->argument(0);
        if (arg.isString())
            return newColor(eng, QColor(arg.toString()));
        else if (arg.isNumber())
            return newColor(eng, Qt::GlobalColor(arg.toInt32()));
        return ctx->throwError(
            QString::fromLatin1("QColor constructor: "
                                "don't know how to construct a color from `%0'")
            .arg(arg.toString()));
    }
    int r = ctx->argument(0).toInt32();
    int g = ctx->argument(1).toInt32();
    int b = ctx->argument(2).toInt32();
    if (ctx->argumentCount() < 4)
        return newColor(eng, QColor(r, g, b));
    int a = ctx->argument(3).toInt32();
    return newColor(eng, QColor(r, g, b, a));
}

static QScriptValue alpha(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QColor, alpha);
    return QScriptValue(eng, self->alpha());
}

static QScriptValue alphaF(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QColor, alphaF);
    return QScriptValue(eng, self->alphaF());
}

static QScriptValue black(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QColor, black);
    return QScriptValue(eng, self->black());
}

static QScriptValue blackF(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QColor, blackF);
    return QScriptValue(eng, self->blackF());
}

static QScriptValue blue(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QColor, blue);
    return QScriptValue(eng, self->blue());
}

static QScriptValue blueF(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QColor, blueF);
    return QScriptValue(eng, self->blueF());
}

static QScriptValue convertTo(QScriptContext *ctx, QScriptEngine *)
{
    return ctx->throwError("QColor.prototype.convertTo is not implemented");
}

static QScriptValue cyan(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QColor, cyan);
    return QScriptValue(eng, self->cyan());
}

static QScriptValue cyanF(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QColor, cyanF);
    return QScriptValue(eng, self->cyanF());
}

static QScriptValue darker(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QColor, darker);
    if (ctx->argumentCount() > 0) {
        int factor = ctx->argument(0).toInt32();
        return qScriptValueFromValue(eng, self->darker(factor));
    }
    return qScriptValueFromValue(eng, self->darker());
}

static QScriptValue green(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QColor, green);
    return QScriptValue(eng, self->green());
}

static QScriptValue greenF(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QColor, greenF);
    return QScriptValue(eng, self->greenF());
}

static QScriptValue hue(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QColor, hue);
    return QScriptValue(eng, self->hue());
}

static QScriptValue hueF(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QColor, hueF);
    return QScriptValue(eng, self->hueF());
}

static QScriptValue isValid(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QColor, isValid);
    return QScriptValue(eng, self->isValid());
}

static QScriptValue lighter(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QColor, lighter);
    if (ctx->argumentCount() > 0) {
        int factor = ctx->argument(0).toInt32();
        return qScriptValueFromValue(eng, self->lighter(factor));
    }
    return qScriptValueFromValue(eng, self->lighter());
}

static QScriptValue magenta(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QColor, magenta);
    return QScriptValue(eng, self->magenta());
}

static QScriptValue magentaF(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QColor, magentaF);
    return QScriptValue(eng, self->magentaF());
}

static QScriptValue name(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QColor, name);
    return QScriptValue(eng, self->name());
}

static QScriptValue red(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QColor, red);
    return QScriptValue(eng, self->red());
}

static QScriptValue redF(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QColor, redF);
    return QScriptValue(eng, self->redF());
}

static QScriptValue rgb(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QColor, rgb);
    return QScriptValue(eng, self->rgb());
}

static QScriptValue rgba(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QColor, rgba);
    return QScriptValue(eng, self->rgba());
}

static QScriptValue saturation(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QColor, saturation);
    return QScriptValue(eng, self->saturation());
}

static QScriptValue saturationF(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QColor, saturationF);
    return QScriptValue(eng, self->saturationF());
}

static QScriptValue setAlpha(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(QColor, setAlpha);
    QScriptValue arg = ctx->argument(0);
    self->setAlpha(arg.toInt32());
    return arg;
}

static QScriptValue setAlphaF(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(QColor, setAlphaF);
    QScriptValue arg = ctx->argument(0);
    self->setAlphaF(arg.toNumber());
    return arg;
}

static QScriptValue setBlue(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(QColor, setBlue);
    QScriptValue arg = ctx->argument(0);
    self->setBlue(arg.toInt32());
    return arg;
}

static QScriptValue setBlueF(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(QColor, setBlueF);
    QScriptValue arg = ctx->argument(0);
    self->setBlueF(arg.toNumber());
    return arg;
}

static QScriptValue setCmyk(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QColor, setCmyk);
    int c = ctx->argument(0).toInt32();
    int m = ctx->argument(1).toInt32();
    int y = ctx->argument(2).toInt32();
    int k = ctx->argument(3).toInt32();
    if (ctx->argumentCount() > 4) {
        int a = ctx->argument(4).toInt32();
        self->setCmyk(c, m, y, k, a);
    } else
        self->setCmyk(c, m, y, k);
    return eng->undefinedValue();
}

static QScriptValue setCmykF(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QColor, setCmykF);
    qreal c = ctx->argument(0).toNumber();
    qreal m = ctx->argument(1).toNumber();
    qreal y = ctx->argument(2).toNumber();
    qreal k = ctx->argument(3).toNumber();
    if (ctx->argumentCount() > 4) {
        qreal a = ctx->argument(4).toNumber();
        self->setCmykF(c, m, y, k, a);
    } else
        self->setCmykF(c, m, y, k);
    return eng->undefinedValue();
}

static QScriptValue setGreen(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(QColor, setGreen);
    QScriptValue arg = ctx->argument(0);
    self->setGreen(arg.toInt32());
    return arg;
}

static QScriptValue setGreenF(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(QColor, setGreenF);
    QScriptValue arg = ctx->argument(0);
    self->setGreenF(arg.toNumber());
    return arg;
}

static QScriptValue setHsv(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QColor, setHsv);
    int h = ctx->argument(0).toInt32();
    int s = ctx->argument(1).toInt32();
    int v = ctx->argument(2).toInt32();
    if (ctx->argumentCount() > 3) {
        int a = ctx->argument(3).toInt32();
        self->setHsv(h, s, v, a);
    } else
        self->setHsv(h, s, v);
    return eng->undefinedValue();
}

static QScriptValue setHsvF(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QColor, setHsvF);
    qreal h = ctx->argument(0).toNumber();
    qreal s = ctx->argument(1).toNumber();
    qreal v = ctx->argument(2).toNumber();
    if (ctx->argumentCount() > 3) {
        qreal a = ctx->argument(3).toNumber();
        self->setHsvF(h, s, v, a);
    } else
        self->setHsvF(h, s, v);
    return eng->undefinedValue();
}

static QScriptValue setNamedColor(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QColor, setNamedColor);
    QString name = ctx->argument(0).toString();
    self->setNamedColor(name);
    return eng->undefinedValue();
}

static QScriptValue setRed(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(QColor, setRed);
    QScriptValue arg = ctx->argument(0);
    self->setRed(arg.toInt32());
    return arg;
}

static QScriptValue setRedF(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(QColor, setRedF);
    QScriptValue arg = ctx->argument(0);
    self->setRedF(arg.toNumber());
    return arg;
}

static QScriptValue setRgb(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QColor, setRgb);
    int r = ctx->argument(0).toInt32();
    int g = ctx->argument(1).toInt32();
    int b = ctx->argument(2).toInt32();
    if (ctx->argumentCount() > 3) {
        int a = ctx->argument(3).toInt32();
        self->setRgb(r, g, b, a);
    } else
        self->setRgb(r, g, b);
    return eng->undefinedValue();
}

static QScriptValue setRgbF(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QColor, setRgbF);
    qreal r = ctx->argument(0).toNumber();
    qreal g = ctx->argument(1).toNumber();
    qreal b = ctx->argument(2).toNumber();
    if (ctx->argumentCount() > 3) {
        qreal a = ctx->argument(3).toNumber();
        self->setRgbF(r, g, b, a);
    } else
        self->setRgbF(r, g, b);
    return eng->undefinedValue();
}

static QScriptValue setRgba(QScriptContext *ctx, QScriptEngine *)
{
    return ctx->throwError("QColor.prototype.setRgba is not implemented");
}

static QScriptValue spec(QScriptContext *ctx, QScriptEngine *)
{
    return ctx->throwError("QColor.prototype.spec is not implemented");
}

static QScriptValue toCmyk(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QColor, toCmyk);
    return qScriptValueFromValue(eng, self->toCmyk());
}

static QScriptValue toHsv(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QColor, toHsv);
    return qScriptValueFromValue(eng, self->toHsv());
}

static QScriptValue toRgb(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QColor, toRgb);
    return qScriptValueFromValue(eng, self->toRgb());
}

static QScriptValue value(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QColor, value);
    return QScriptValue(eng, self->value());
}

static QScriptValue valueF(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QColor, valueF);
    return QScriptValue(eng, self->valueF());
}

static QScriptValue yellow(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QColor, yellow);
    return QScriptValue(eng, self->yellow());
}

static QScriptValue yellowF(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QColor, yellowF);
    return QScriptValue(eng, self->yellowF());
}

static QScriptValue toString(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QColor, toString);
    return QScriptValue(eng, QString("QColor(%0)").arg(self->name()));
}

QScriptValue constructColorClass(QScriptEngine *eng)
{
    QScriptValue proto = qScriptValueFromValue(eng, QColor());
    QScriptValue::PropertyFlags getter = QScriptValue::PropertyGetter;
    QScriptValue::PropertyFlags setter = QScriptValue::PropertySetter;
    proto.setProperty("alpha", eng->newFunction(alpha), getter);
    proto.setProperty("alphaF", eng->newFunction(alphaF), getter);
    proto.setProperty("black", eng->newFunction(black), getter);
    proto.setProperty("blackF", eng->newFunction(blackF), getter);
    proto.setProperty("blue", eng->newFunction(blue), getter);
    proto.setProperty("blueF", eng->newFunction(blueF), getter);
    proto.setProperty("convertTo", eng->newFunction(convertTo));
    proto.setProperty("cyan", eng->newFunction(cyan), getter);
    proto.setProperty("cyanF", eng->newFunction(cyanF), getter);
    proto.setProperty("darker", eng->newFunction(darker));
    proto.setProperty("green", eng->newFunction(green), getter);
    proto.setProperty("greenF", eng->newFunction(greenF), getter);
    proto.setProperty("hue", eng->newFunction(hue), getter);
    proto.setProperty("hueF", eng->newFunction(hueF), getter);
    proto.setProperty("isValid", eng->newFunction(isValid), getter);
    proto.setProperty("lighter", eng->newFunction(lighter));
    proto.setProperty("magenta", eng->newFunction(magenta), getter);
    proto.setProperty("magentaF", eng->newFunction(magentaF), getter);
    proto.setProperty("name", eng->newFunction(name), getter);
    proto.setProperty("red", eng->newFunction(red), getter);
    proto.setProperty("redF", eng->newFunction(redF), getter);
    proto.setProperty("getRgb", eng->newFunction(rgb));
    proto.setProperty("rgba", eng->newFunction(rgba), getter);
    proto.setProperty("saturation", eng->newFunction(saturation), getter);
    proto.setProperty("saturationF", eng->newFunction(saturationF), getter);
    proto.setProperty("alpha", eng->newFunction(setAlpha), setter);
    proto.setProperty("alphaF", eng->newFunction(setAlphaF), setter);
    proto.setProperty("blue", eng->newFunction(setBlue), setter);
    proto.setProperty("blueF", eng->newFunction(setBlueF), setter);
    proto.setProperty("setCmyk", eng->newFunction(setCmyk));
    proto.setProperty("setCmykF", eng->newFunction(setCmykF));
    proto.setProperty("green", eng->newFunction(setGreen), setter);
    proto.setProperty("greenF", eng->newFunction(setGreenF), setter);
    proto.setProperty("setHsv", eng->newFunction(setHsv));
    proto.setProperty("setHsvF", eng->newFunction(setHsvF));
    proto.setProperty("setNamedColor", eng->newFunction(setNamedColor));
    proto.setProperty("red", eng->newFunction(setRed), setter);
    proto.setProperty("redF", eng->newFunction(setRedF), setter);
    proto.setProperty("setRgb", eng->newFunction(setRgb));
    proto.setProperty("setRgbF", eng->newFunction(setRgbF));
    proto.setProperty("rgba", eng->newFunction(setRgba), setter);
    proto.setProperty("spec", eng->newFunction(spec));
    proto.setProperty("toCmyk", eng->newFunction(toCmyk));
    proto.setProperty("toHsv", eng->newFunction(toHsv));
    proto.setProperty("toRgb", eng->newFunction(toRgb));
    proto.setProperty("value", eng->newFunction(value), getter);
    proto.setProperty("valueF", eng->newFunction(valueF), getter);
    proto.setProperty("yellow", eng->newFunction(yellow), getter);
    proto.setProperty("yellowF", eng->newFunction(yellowF), getter);
    proto.setProperty("toString", eng->newFunction(toString));

    eng->setDefaultPrototype(qMetaTypeId<QColor>(), proto);

    return eng->newFunction(ctor, proto);
}
