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
    DECLARE_SELF(Image, allGray);
    return QScriptValue(eng, self->allGray());
}

/////////////////////////////////////////////////////////////

static QScriptValue alphaChannel(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Image, alphaChannel);
    return newImage(eng, self->alphaChannel());
}

/////////////////////////////////////////////////////////////

static QScriptValue bits(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(Image, bits);
    return ctx->throwError("Image.prototype.bits is not implemented");
}

/////////////////////////////////////////////////////////////

static QScriptValue bytesPerLine(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Image, bytesPerLine);
    return QScriptValue(eng, self->bytesPerLine());
}

/////////////////////////////////////////////////////////////

#if 0
static QScriptValue cacheKey(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Image, cacheKey);
    return QScriptValue(eng, self->cacheKey());
}
#endif

/////////////////////////////////////////////////////////////

static QScriptValue color(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Image, color);
    return eng->toScriptValue(self->color(ctx->argument(0).toInt32()));
}

/////////////////////////////////////////////////////////////

static QScriptValue colorTable(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Image, colorTable);
    return eng->toScriptValue(self->colorTable());
}

/////////////////////////////////////////////////////////////

static QScriptValue convertToFormat(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Image, convertToFormat);
    QImage::Format format = static_cast<QImage::Format>(ctx->argument(0).toInt32());
    return newImage(eng, self->convertToFormat(format));
}

/////////////////////////////////////////////////////////////

static QScriptValue copy(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Image, copy);
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
    DECLARE_SELF(Image, createAlphaMask);
    return ctx->throwError("Image.prototype.createAlphaMask is not implemented");
}

/////////////////////////////////////////////////////////////

static QScriptValue createHeuristicMask(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(Image, createHeuristicMask);
    return ctx->throwError("Image.prototype.createHeuristicMask is not implemented");
}

/////////////////////////////////////////////////////////////

static QScriptValue createMaskFromColor(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(Image, createMaskFromColor);
    return ctx->throwError("Image.prototype.createMaskFromColor is not implemented");
}

/////////////////////////////////////////////////////////////

static QScriptValue depth(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Image, depth);
    return QScriptValue(eng, self->depth());
}

/////////////////////////////////////////////////////////////

static QScriptValue dotsPerMeterX(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Image, dotsPerMeterX);
    return QScriptValue(eng, self->dotsPerMeterX());
}

/////////////////////////////////////////////////////////////

static QScriptValue dotsPerMeterY(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Image, dotsPerMeterY);
    return QScriptValue(eng, self->dotsPerMeterY());
}

/////////////////////////////////////////////////////////////

static QScriptValue fill(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Image, fill);
    self->fill(ctx->argument(0).toUInt32());
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue format(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Image, format);
    return QScriptValue(eng, static_cast<int>(self->format()));
}

/////////////////////////////////////////////////////////////

static QScriptValue hasAlphaChannel(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Image, hasAlphaChannel);
    return QScriptValue(eng, self->hasAlphaChannel());
}

/////////////////////////////////////////////////////////////

static QScriptValue height(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Image, height);
    return QScriptValue(eng, self->height());
}

/////////////////////////////////////////////////////////////

static QScriptValue invertPixels(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Image, invertPixels);
    if (ctx->argumentCount() == 0)
        self->invertPixels();
    else
        self->invertPixels(static_cast<QImage::InvertMode>(ctx->argument(0).toInt32()));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue isGrayscale(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Image, isGrayscale);
    return QScriptValue(eng, self->isGrayscale());
}

/////////////////////////////////////////////////////////////

static QScriptValue isNull(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Image, isNull);
    return QScriptValue(eng, self->isNull());
}

/////////////////////////////////////////////////////////////

static QScriptValue load(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Image, load);
    return QScriptValue(eng, self->load(ctx->argument(0).toString()));
}

/////////////////////////////////////////////////////////////

static QScriptValue loadFromData(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(Image, loadFromData);
    return ctx->throwError("Image.prototype.loadFromData is not implemented");
}

/////////////////////////////////////////////////////////////

static QScriptValue mirrored(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(Image, mirrored);
    return ctx->throwError("Image.prototype.mirrored is not implemented");
}

/////////////////////////////////////////////////////////////

static QScriptValue numBytes(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Image, numBytes);
    return QScriptValue(eng, self->numBytes());
}

/////////////////////////////////////////////////////////////

static QScriptValue numColors(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Image, numColors);
    return QScriptValue(eng, self->numColors());
}

/////////////////////////////////////////////////////////////

static QScriptValue offset(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Image, offset);
    return eng->toScriptValue(self->offset());
}

/////////////////////////////////////////////////////////////

static QScriptValue pixel(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Image, pixel);
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
    DECLARE_SELF(Image, pixelIndex);
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
    DECLARE_SELF(Image, rect);
    return eng->toScriptValue(self->rect());
}

/////////////////////////////////////////////////////////////

static QScriptValue rgbSwapped(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Image, rgbSwapped);
    return newImage(eng, self->rgbSwapped());
}

/////////////////////////////////////////////////////////////

static QScriptValue save(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Image, save);
    return QScriptValue(eng, self->save(ctx->argument(0).toString()));
}

/////////////////////////////////////////////////////////////

static QScriptValue scaled(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(Image, scaled);
    return ctx->throwError("Image.prototype.scaled is not implemented");
}

/////////////////////////////////////////////////////////////

static QScriptValue scaledToHeight(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(Image, scaledToHeight);
    return ctx->throwError("Image.prototype.scaledToHeight is not implemented");
}

/////////////////////////////////////////////////////////////

static QScriptValue scaledToWidth(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(Image, scaledToWidth);
    return ctx->throwError("Image.prototype.scaledToWidth is not implemented");
}

/////////////////////////////////////////////////////////////

static QScriptValue scanLine(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(Image, scanLine);
    return ctx->throwError("Image.prototype.scanLine is not implemented");
}

/////////////////////////////////////////////////////////////

static QScriptValue setAlphaChannel(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Image, setAlphaChannel);
    self->setAlphaChannel(qscriptvalue_cast<QImage>(ctx->argument(0)));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setColor(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Image, setColor);
    self->setColor(ctx->argument(0).toInt32(), qscriptvalue_cast<QRgb>(ctx->argument(1)));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setColorTable(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Image, setColorTable);
    self->setColorTable(qscriptvalue_cast<QVector<QRgb> >(ctx->argument(0)));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setDotsPerMeterX(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Image, setDotsPerMeterX);
    self->setDotsPerMeterX(ctx->argument(0).toInt32());
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setDotsPerMeterY(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Image, setDotsPerMeterY);
    self->setDotsPerMeterY(ctx->argument(0).toInt32());
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setNumColors(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Image, setNumColors);
    self->setNumColors(ctx->argument(0).toInt32());
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setOffset(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Image, setOffset);
    self->setOffset(qscriptvalue_cast<QPoint>(ctx->argument(0)));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setPixel(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Image, setPixel);
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
    DECLARE_SELF(Image, setText);
    self->setText(ctx->argument(0).toString(), ctx->argument(1).toString());
#endif
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue size(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Image, size);
    return eng->toScriptValue(self->size());
}

/////////////////////////////////////////////////////////////

static QScriptValue text(QScriptContext *ctx, QScriptEngine *eng)
{
#ifndef QT_NO_IMAGE_TEXT
    DECLARE_SELF(Image, text);
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
    DECLARE_SELF(Image, textKeys);
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
    DECLARE_SELF(Image, transformed);
    return ctx->throwError("Image.prototype.transformed is not implemented");
}

/////////////////////////////////////////////////////////////

static QScriptValue valid(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Image, valid);
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
    DECLARE_SELF(Image, width);
    return QScriptValue(eng, self->width());
}

/////////////////////////////////////////////////////////////

static QScriptValue toString(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Image, toString);
    return QScriptValue(eng, "Image");
}

/////////////////////////////////////////////////////////////

QScriptValue constructImageClass(QScriptEngine *eng)
{
    QScriptValue proto = newImage(eng, QImage());
    ADD_PROTO_FUNCTION(proto, allGray);
    ADD_PROTO_FUNCTION(proto, alphaChannel);
    ADD_PROTO_FUNCTION(proto, bits);
    ADD_PROTO_FUNCTION(proto, bytesPerLine);
    //ADD_PROTO_FUNCTION(proto, cacheKey);
    ADD_PROTO_FUNCTION(proto, color);
    ADD_PROTO_FUNCTION(proto, colorTable);
    ADD_PROTO_FUNCTION(proto, convertToFormat);
    ADD_PROTO_FUNCTION(proto, copy);
    ADD_PROTO_FUNCTION(proto, createAlphaMask);
    ADD_PROTO_FUNCTION(proto, createHeuristicMask);
    ADD_PROTO_FUNCTION(proto, createMaskFromColor);
    ADD_PROTO_FUNCTION(proto, depth);
    ADD_PROTO_FUNCTION(proto, dotsPerMeterX);
    ADD_PROTO_FUNCTION(proto, dotsPerMeterY);
    ADD_PROTO_FUNCTION(proto, fill);
    ADD_PROTO_FUNCTION(proto, format);
    ADD_PROTO_FUNCTION(proto, hasAlphaChannel);
    ADD_PROTO_FUNCTION(proto, height);
    ADD_PROTO_FUNCTION(proto, invertPixels);
    ADD_PROTO_FUNCTION(proto, isGrayscale);
    ADD_PROTO_FUNCTION(proto, isNull);
    ADD_PROTO_FUNCTION(proto, load);
    ADD_PROTO_FUNCTION(proto, loadFromData);
    ADD_PROTO_FUNCTION(proto, mirrored);
    ADD_PROTO_FUNCTION(proto, numBytes);
    ADD_PROTO_FUNCTION(proto, numColors);
    ADD_PROTO_FUNCTION(proto, offset);
    ADD_PROTO_FUNCTION(proto, pixel);
    ADD_PROTO_FUNCTION(proto, pixelIndex);
    ADD_PROTO_FUNCTION(proto, rect);
    ADD_PROTO_FUNCTION(proto, rgbSwapped);
    ADD_PROTO_FUNCTION(proto, save);
    ADD_PROTO_FUNCTION(proto, scaled);
    ADD_PROTO_FUNCTION(proto, scaledToHeight);
    ADD_PROTO_FUNCTION(proto, scaledToWidth);
    ADD_PROTO_FUNCTION(proto, scanLine);
    ADD_PROTO_FUNCTION(proto, setAlphaChannel);
    ADD_PROTO_FUNCTION(proto, setColor);
    ADD_PROTO_FUNCTION(proto, setColorTable);
    ADD_PROTO_FUNCTION(proto, setDotsPerMeterX);
    ADD_PROTO_FUNCTION(proto, setDotsPerMeterY);
    ADD_PROTO_FUNCTION(proto, setNumColors);
    ADD_PROTO_FUNCTION(proto, setOffset);
    ADD_PROTO_FUNCTION(proto, setPixel);
    ADD_PROTO_FUNCTION(proto, setText);
    ADD_PROTO_FUNCTION(proto, size);
    ADD_PROTO_FUNCTION(proto, text);
    ADD_PROTO_FUNCTION(proto, textKeys);
    ADD_PROTO_FUNCTION(proto, toString);
    ADD_PROTO_FUNCTION(proto, transformed);
    ADD_PROTO_FUNCTION(proto, valid);
    ADD_PROTO_FUNCTION(proto, width);

    eng->setDefaultPrototype(qMetaTypeId<QImage>(), proto);
    eng->setDefaultPrototype(qMetaTypeId<QImage*>(), proto);

    return eng->newFunction(ctor, proto);
}
