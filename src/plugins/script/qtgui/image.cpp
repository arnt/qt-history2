#include <QtScript/QScriptValue>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptContext>
#include <QtCore/QStringList>
#include <QtGui/QImage>
#include "../global.h"

Q_DECLARE_METATYPE(QImage)
Q_DECLARE_METATYPE(QImage*)
Q_DECLARE_METATYPE(QVector<QRgb>)

static inline QScriptValue newImage(QScriptEngine *eng, const QImage &image)
{
    return eng->newVariant(qVariantFromValue(image));
}

/////////////////////////////////////////////////////////////

static QScriptValue ctor(QScriptContext *ctx, QScriptEngine *eng)
{
    if (ctx->argumentCount() == 0)
        return newImage(eng, QImage());
    else if (ctx->argumentCount() == 1) {
        if (QImage *other = qscriptvalue_cast<QImage*>(ctx->argument(0)))
            return newImage(eng, QImage(*other));
        return newImage(eng, QImage(ctx->argument(0).toString()));
    } else if (ctx->argumentCount() == 2) {
        return newImage(eng, QImage(qscriptvalue_cast<QSize>(ctx->argument(0)),
                                    static_cast<QImage::Format>(ctx->argument(1).toInt32())));
    } else {
        return newImage(eng, QImage(ctx->argument(0).toInt32(),
                                    ctx->argument(1).toInt32(),
                                    static_cast<QImage::Format>(ctx->argument(2).toInt32())));
    }
}

/////////////////////////////////////////////////////////////

static QScriptValue allGray(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QImage, allGray);
    return QScriptValue(eng, self->allGray());
}

/////////////////////////////////////////////////////////////

static QScriptValue alphaChannel(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QImage, alphaChannel);
    return newImage(eng, self->alphaChannel());
}

/////////////////////////////////////////////////////////////

static QScriptValue bits(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(QImage, bits);
    return ctx->throwError("QImage.prototype.bits is not implemented");
}

/////////////////////////////////////////////////////////////

static QScriptValue bytesPerLine(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QImage, bytesPerLine);
    return QScriptValue(eng, self->bytesPerLine());
}

/////////////////////////////////////////////////////////////

#if 0
static QScriptValue cacheKey(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QImage, cacheKey);
    return QScriptValue(eng, self->cacheKey());
}
#endif

/////////////////////////////////////////////////////////////

static QScriptValue color(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QImage, color);
    return eng->toScriptValue(self->color(ctx->argument(0).toInt32()));
}

/////////////////////////////////////////////////////////////

static QScriptValue colorTable(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QImage, colorTable);
    return eng->toScriptValue(self->colorTable());
}

/////////////////////////////////////////////////////////////

static QScriptValue convertToFormat(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QImage, convertToFormat);
    QImage::Format format = static_cast<QImage::Format>(ctx->argument(0).toInt32());
    return newImage(eng, self->convertToFormat(format));
}

/////////////////////////////////////////////////////////////

static QScriptValue copy(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QImage, copy);
    if (ctx->argumentCount() == 0)
        return newImage(eng, self->copy());
    else if (ctx->argumentCount() == 1)
        return newImage(eng, self->copy(qscriptvalue_cast<QRect>(ctx->argument(0))));
    else
        return newImage(eng, self->copy(ctx->argument(0).toInt32(),
                                        ctx->argument(1).toInt32(),
                                        ctx->argument(2).toInt32(),
                                        ctx->argument(3).toInt32()));
}

/////////////////////////////////////////////////////////////

static QScriptValue createAlphaMask(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(QImage, createAlphaMask);
    return ctx->throwError("QImage.prototype.createAlphaMask is not implemented");
}

/////////////////////////////////////////////////////////////

static QScriptValue createHeuristicMask(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(QImage, createHeuristicMask);
    return ctx->throwError("QImage.prototype.createHeuristicMask is not implemented");
}

/////////////////////////////////////////////////////////////

static QScriptValue createMaskFromColor(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(QImage, createMaskFromColor);
    return ctx->throwError("QImage.prototype.createMaskFromColor is not implemented");
}

/////////////////////////////////////////////////////////////

static QScriptValue depth(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QImage, depth);
    return QScriptValue(eng, self->depth());
}

/////////////////////////////////////////////////////////////

static QScriptValue dotsPerMeterX(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QImage, dotsPerMeterX);
    return QScriptValue(eng, self->dotsPerMeterX());
}

/////////////////////////////////////////////////////////////

static QScriptValue dotsPerMeterY(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QImage, dotsPerMeterY);
    return QScriptValue(eng, self->dotsPerMeterY());
}

/////////////////////////////////////////////////////////////

static QScriptValue fill(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QImage, fill);
    self->fill(ctx->argument(0).toUInt32());
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue format(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QImage, format);
    return QScriptValue(eng, static_cast<int>(self->format()));
}

/////////////////////////////////////////////////////////////

static QScriptValue hasAlphaChannel(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QImage, hasAlphaChannel);
    return QScriptValue(eng, self->hasAlphaChannel());
}

/////////////////////////////////////////////////////////////

static QScriptValue height(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QImage, height);
    return QScriptValue(eng, self->height());
}

/////////////////////////////////////////////////////////////

static QScriptValue invertPixels(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QImage, invertPixels);
    if (ctx->argumentCount() == 0)
        self->invertPixels();
    else
        self->invertPixels(static_cast<QImage::InvertMode>(ctx->argument(0).toInt32()));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue isGrayscale(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QImage, isGrayscale);
    return QScriptValue(eng, self->isGrayscale());
}

/////////////////////////////////////////////////////////////

static QScriptValue isNull(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QImage, isNull);
    return QScriptValue(eng, self->isNull());
}

/////////////////////////////////////////////////////////////

static QScriptValue load(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QImage, load);
    return QScriptValue(eng, self->load(ctx->argument(0).toString()));
}

/////////////////////////////////////////////////////////////

static QScriptValue loadFromData(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(QImage, loadFromData);
    return ctx->throwError("QImage.prototype.loadFromData is not implemented");
}

/////////////////////////////////////////////////////////////

static QScriptValue mirrored(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(QImage, mirrored);
    return ctx->throwError("QImage.prototype.mirrored is not implemented");
}

/////////////////////////////////////////////////////////////

static QScriptValue numBytes(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QImage, numBytes);
    return QScriptValue(eng, self->numBytes());
}

/////////////////////////////////////////////////////////////

static QScriptValue numColors(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QImage, numColors);
    return QScriptValue(eng, self->numColors());
}

/////////////////////////////////////////////////////////////

static QScriptValue offset(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QImage, offset);
    return eng->toScriptValue(self->offset());
}

/////////////////////////////////////////////////////////////

static QScriptValue pixel(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QImage, pixel);
    if (ctx->argumentCount() > 1) {
        return eng->toScriptValue(self->pixel(ctx->argument(0).toInt32(),
                                              ctx->argument(1).toInt32()));
    } else {
        return eng->toScriptValue(self->pixel(qscriptvalue_cast<QPoint>(ctx->argument(0))));
    }
}

/////////////////////////////////////////////////////////////

static QScriptValue pixelIndex(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QImage, pixelIndex);
    if (ctx->argumentCount() > 1) {
        return QScriptValue(eng, self->pixelIndex(ctx->argument(0).toInt32(),
                                                  ctx->argument(1).toInt32()));
    } else {
        return QScriptValue(eng, self->pixelIndex(qscriptvalue_cast<QPoint>(ctx->argument(0))));
    }
}

/////////////////////////////////////////////////////////////

static QScriptValue rect(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QImage, rect);
    return eng->toScriptValue(self->rect());
}

/////////////////////////////////////////////////////////////

static QScriptValue rgbSwapped(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QImage, rgbSwapped);
    return newImage(eng, self->rgbSwapped());
}

/////////////////////////////////////////////////////////////

static QScriptValue save(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QImage, save);
    return QScriptValue(eng, self->save(ctx->argument(0).toString()));
}

/////////////////////////////////////////////////////////////

static QScriptValue scaled(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(QImage, scaled);
    return ctx->throwError("QImage.prototype.scaled is not implemented");
}

/////////////////////////////////////////////////////////////

static QScriptValue scaledToHeight(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(QImage, scaledToHeight);
    return ctx->throwError("QImage.prototype.scaledToHeight is not implemented");
}

/////////////////////////////////////////////////////////////

static QScriptValue scaledToWidth(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(QImage, scaledToWidth);
    return ctx->throwError("QImage.prototype.scaledToWidth is not implemented");
}

/////////////////////////////////////////////////////////////

static QScriptValue scanLine(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(QImage, scanLine);
    return ctx->throwError("QImage.prototype.scanLine is not implemented");
}

/////////////////////////////////////////////////////////////

static QScriptValue setAlphaChannel(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QImage, setAlphaChannel);
    self->setAlphaChannel(qscriptvalue_cast<QImage>(ctx->argument(0)));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setColor(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QImage, setColor);
    self->setColor(ctx->argument(0).toInt32(), qscriptvalue_cast<QRgb>(ctx->argument(1)));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setColorTable(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QImage, setColorTable);
    self->setColorTable(qscriptvalue_cast<QVector<QRgb> >(ctx->argument(0)));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setDotsPerMeterX(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QImage, setDotsPerMeterX);
    self->setDotsPerMeterX(ctx->argument(0).toInt32());
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setDotsPerMeterY(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QImage, setDotsPerMeterY);
    self->setDotsPerMeterY(ctx->argument(0).toInt32());
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setNumColors(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QImage, setNumColors);
    self->setNumColors(ctx->argument(0).toInt32());
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setOffset(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QImage, setOffset);
    self->setOffset(qscriptvalue_cast<QPoint>(ctx->argument(0)));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setPixel(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QImage, setPixel);
    if (ctx->argumentCount() > 2) {
        self->setPixel(ctx->argument(0).toInt32(),
                       ctx->argument(1).toInt32(),
                       ctx->argument(2).toUInt32());
    } else {
        self->setPixel(qscriptvalue_cast<QPoint>(ctx->argument(0)),
                       ctx->argument(1).toUInt32());
    }
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setText(QScriptContext *ctx, QScriptEngine *eng)
{
#ifndef QT_NO_IMAGE_TEXT
    DECLARE_SELF(QImage, setText);
    self->setText(ctx->argument(0).toString(), ctx->argument(1).toString());
#endif
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue size(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QImage, size);
    return eng->toScriptValue(self->size());
}

/////////////////////////////////////////////////////////////

static QScriptValue text(QScriptContext *ctx, QScriptEngine *eng)
{
#ifndef QT_NO_IMAGE_TEXT
    DECLARE_SELF(QImage, text);
    if (ctx->argumentCount() == 0)
        return QScriptValue(eng, self->text());
    return QScriptValue(eng, self->text(ctx->argument(0).toString()));
#else
    Q_UNUSED(ctx);
    Q_UNUSED(eng);
    return eng->undefinedValue();
#endif
}

/////////////////////////////////////////////////////////////

static QScriptValue textKeys(QScriptContext *ctx, QScriptEngine *eng)
{
#ifndef QT_NO_IMAGE_TEXT
    DECLARE_SELF(QImage, textKeys);
    return eng->toScriptValue(self->textKeys());
#else
    Q_UNUSED(ctx);
    Q_UNUSED(eng);
    return eng->undefinedValue();
#endif
}

/////////////////////////////////////////////////////////////

static QScriptValue transformed(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(QImage, transformed);
    return ctx->throwError("QImage.prototype.transformed is not implemented");
}

/////////////////////////////////////////////////////////////

static QScriptValue valid(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QImage, valid);
    if (ctx->argumentCount() > 1) {
        return QScriptValue(eng, self->valid(ctx->argument(0).toInt32(),
                                             ctx->argument(1).toInt32()));
    } else {
        return QScriptValue(eng, self->valid(qscriptvalue_cast<QPoint>(ctx->argument(0))));
    }
}

/////////////////////////////////////////////////////////////

static QScriptValue width(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QImage, width);
    return QScriptValue(eng, self->width());
}

/////////////////////////////////////////////////////////////

static QScriptValue toString(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QImage, toString);
    return QScriptValue(eng, "QImage");
}

/////////////////////////////////////////////////////////////

QScriptValue constructImageClass(QScriptEngine *eng)
{
    QScriptValue proto = newImage(eng, QImage());
    ADD_METHOD(proto, allGray);
    ADD_METHOD(proto, alphaChannel);
    ADD_METHOD(proto, bits);
    ADD_METHOD(proto, bytesPerLine);
    //ADD_METHOD(proto, cacheKey);
    ADD_METHOD(proto, color);
    ADD_METHOD(proto, colorTable);
    ADD_METHOD(proto, convertToFormat);
    ADD_METHOD(proto, copy);
    ADD_METHOD(proto, createAlphaMask);
    ADD_METHOD(proto, createHeuristicMask);
    ADD_METHOD(proto, createMaskFromColor);
    ADD_METHOD(proto, depth);
    ADD_METHOD(proto, dotsPerMeterX);
    ADD_METHOD(proto, dotsPerMeterY);
    ADD_METHOD(proto, fill);
    ADD_METHOD(proto, format);
    ADD_METHOD(proto, hasAlphaChannel);
    ADD_METHOD(proto, height);
    ADD_METHOD(proto, invertPixels);
    ADD_METHOD(proto, isGrayscale);
    ADD_METHOD(proto, isNull);
    ADD_METHOD(proto, load);
    ADD_METHOD(proto, loadFromData);
    ADD_METHOD(proto, mirrored);
    ADD_METHOD(proto, numBytes);
    ADD_METHOD(proto, numColors);
    ADD_METHOD(proto, offset);
    ADD_METHOD(proto, pixel);
    ADD_METHOD(proto, pixelIndex);
    ADD_METHOD(proto, rect);
    ADD_METHOD(proto, rgbSwapped);
    ADD_METHOD(proto, save);
    ADD_METHOD(proto, scaled);
    ADD_METHOD(proto, scaledToHeight);
    ADD_METHOD(proto, scaledToWidth);
    ADD_METHOD(proto, scanLine);
    ADD_METHOD(proto, setAlphaChannel);
    ADD_METHOD(proto, setColor);
    ADD_METHOD(proto, setColorTable);
    ADD_METHOD(proto, setDotsPerMeterX);
    ADD_METHOD(proto, setDotsPerMeterY);
    ADD_METHOD(proto, setNumColors);
    ADD_METHOD(proto, setOffset);
    ADD_METHOD(proto, setPixel);
    ADD_METHOD(proto, setText);
    ADD_METHOD(proto, size);
    ADD_METHOD(proto, text);
    ADD_METHOD(proto, textKeys);
    ADD_METHOD(proto, toString);
    ADD_METHOD(proto, transformed);
    ADD_METHOD(proto, valid);
    ADD_METHOD(proto, width);

    eng->setDefaultPrototype(qMetaTypeId<QImage>(), proto);
    eng->setDefaultPrototype(qMetaTypeId<QImage*>(), proto);

    return eng->newFunction(ctor, proto);
}
