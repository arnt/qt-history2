#include <QtScript/QScriptValue>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptContext>
#include <QtGui/QBitmap>
#include <QtGui/QPixmap>
#include <qdebug.h>
#include "../global.h"

Q_DECLARE_METATYPE(QBitmap)
Q_DECLARE_METATYPE(QPixmap)
Q_DECLARE_METATYPE(QPixmap*)

static inline QScriptValue newPixmap(QScriptEngine *eng, const QPixmap &pixmap)
{
    return eng->newVariant(qVariantFromValue(pixmap));
}

/////////////////////////////////////////////////////////////

static QScriptValue ctor(QScriptContext *ctx, QScriptEngine *eng)
{
    if (ctx->argumentCount() == 0)
        return newPixmap(eng, QPixmap());
    if (QPixmap *other = qscriptvalue_cast<QPixmap*>(ctx->argument(0)))
        return newPixmap(eng, QPixmap(*other));
    if (ctx->argument(0).isString())
        return newPixmap(eng, QPixmap(ctx->argument(0).toString()));
    if (ctx->argumentCount() > 1) {
        return newPixmap(eng, QPixmap(ctx->argument(0).toInt32(),
                                      ctx->argument(1).toInt32()));
    }
    return newPixmap(eng, qscriptvalue_cast<QSize>(ctx->argument(0)));
}

/////////////////////////////////////////////////////////////

static QScriptValue alphaChannel(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPixmap, alphaChannel);
    return newPixmap(eng, self->alphaChannel());
}

/////////////////////////////////////////////////////////////

static QScriptValue cacheKey(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPixmap, cacheKey);
    return eng->toScriptValue(self->cacheKey());
}

/////////////////////////////////////////////////////////////

static QScriptValue copy(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPixmap, copy);
    if (ctx->argumentCount() == 0)
        return newPixmap(eng, self->copy());
    else if (ctx->argumentCount() == 1)
        return newPixmap(eng, self->copy(qscriptvalue_cast<QRect>(ctx->argument(0))));
    return newPixmap(eng, self->copy(ctx->argument(0).toInt32(),
                                     ctx->argument(1).toInt32(),
                                     ctx->argument(2).toInt32(),
                                     ctx->argument(3).toInt32()));
}

/////////////////////////////////////////////////////////////

static QScriptValue createHeuristicMask(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(QPixmap, createHeuristicMask);
    return ctx->throwError("QPixmap.prototype.createHeuristicMask is not implemented");
}

/////////////////////////////////////////////////////////////

static QScriptValue createMaskFromColor(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(QPixmap, createMaskFromColor);
    return ctx->throwError("QPixmap.prototype.createMaskFromColor is not implemented");
}

/////////////////////////////////////////////////////////////

static QScriptValue depth(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPixmap, depth);
    return QScriptValue(eng, self->depth());
}

/////////////////////////////////////////////////////////////

static QScriptValue detach(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPixmap, detach);
    self->detach();
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue fill(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPixmap, fill);
    if (ctx->argumentCount() == 0) {
        self->fill();
    } else {
        if (QWidget *widget = qscriptvalue_cast<QWidget*>(ctx->argument(0))) {
            if (ctx->argumentCount() > 2) {
                self->fill(widget,
                           ctx->argument(1).toInt32(),
                           ctx->argument(2).toInt32());
            } else {
                self->fill(widget,
                           qscriptvalue_cast<QPoint>(ctx->argument(1)));
            }
        } else {
            self->fill(qscriptvalue_cast<QColor>(ctx->argument(0)));
        }
    }
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue handle(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(QPixmap, handle);
    return ctx->throwError("QPixmap.prototype.handle is not implemented");
}

/////////////////////////////////////////////////////////////

static QScriptValue hasAlpha(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPixmap, hasAlpha);
    return QScriptValue(eng, self->hasAlpha());
}

/////////////////////////////////////////////////////////////

static QScriptValue hasAlphaChannel(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPixmap, hasAlphaChannel);
    return QScriptValue(eng, self->hasAlphaChannel());
}

/////////////////////////////////////////////////////////////

static QScriptValue height(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPixmap, height);
    return QScriptValue(eng, self->height());
}

/////////////////////////////////////////////////////////////

static QScriptValue isNull(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPixmap, isNull);
    return QScriptValue(eng, self->isNull());
}

/////////////////////////////////////////////////////////////

static QScriptValue isQBitmap(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPixmap, isQBitmap);
    return QScriptValue(eng, self->isQBitmap());
}

/////////////////////////////////////////////////////////////

static QScriptValue load(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPixmap, load);
    return QScriptValue(eng, self->load(ctx->argument(0).toString()));
}

/////////////////////////////////////////////////////////////

static QScriptValue loadFromData(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(QPixmap, loadFromData);
    return ctx->throwError("QPixmap.prototype.loadFromData is not implemented");
}

/////////////////////////////////////////////////////////////

static QScriptValue mask(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPixmap, mask);
    return eng->toScriptValue(self->mask());
}

/////////////////////////////////////////////////////////////

static QScriptValue rect(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPixmap, rect);
    return eng->toScriptValue(self->rect());
}

/////////////////////////////////////////////////////////////

static QScriptValue save(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPixmap, save);
    return QScriptValue(eng, self->save(ctx->argument(0).toString()));
}

/////////////////////////////////////////////////////////////

static QScriptValue scaled(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(QPixmap, scaled);
    return ctx->throwError("QPixmap.prototype.scaled is not implemented");
}

/////////////////////////////////////////////////////////////

static QScriptValue scaledToHeight(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(QPixmap, scaledToHeight);
    return ctx->throwError("QPixmap.prototype.scaledToHeight is not implemented");
}

/////////////////////////////////////////////////////////////

static QScriptValue scaledToWidth(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(QPixmap, scaledToWidth);
    return ctx->throwError("QPixmap.prototype.scaledToWidth is not implemented");
}

/////////////////////////////////////////////////////////////

static QScriptValue setAlphaChannel(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPixmap, setAlphaChannel);
    self->setAlphaChannel(qscriptvalue_cast<QPixmap>(ctx->argument(0)));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setMask(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPixmap, setMask);
    self->setMask(qscriptvalue_cast<QBitmap>(ctx->argument(0)));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue size(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPixmap, size);
    return eng->toScriptValue(self->size());
}

/////////////////////////////////////////////////////////////

static QScriptValue toImage(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPixmap, toImage);
    return eng->toScriptValue(self->toImage());
}

/////////////////////////////////////////////////////////////

static QScriptValue toMacCGImageRef(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(QPixmap, toMacCGImageRef);
    return ctx->throwError("QPixmap.prototype.toMacCGImageRef is not implemented");
}

/////////////////////////////////////////////////////////////

static QScriptValue toWinHBITMAP(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(QPixmap, toWinHBITMAP);
    return ctx->throwError("QPixmap.prototype.toWinHBITMAP is not implemented");
}

/////////////////////////////////////////////////////////////

static QScriptValue transformed(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(QPixmap, transformed);
    return ctx->throwError("QPixmap.prototype.transformed is not implemented");
}

/////////////////////////////////////////////////////////////

static QScriptValue width(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPixmap, width);
    return QScriptValue(eng, self->width());
}

/////////////////////////////////////////////////////////////

static QScriptValue x11Info(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(QPixmap, x11Info);
    return ctx->throwError("QPixmap.prototype.x11Info is not implemented");
}

/////////////////////////////////////////////////////////////

static QScriptValue x11PictureHandle(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(QPixmap, x11PictureHandle);
    return ctx->throwError("QPixmap.prototype.x11PictureHandle is not implemented");
}

/////////////////////////////////////////////////////////////

static QScriptValue toString(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPixmap, toString);
    if (self->isNull()) {
        return QScriptValue(eng, QString::fromLatin1("QPixmap(null)"));
    } else {
        return QScriptValue(eng, QString::fromLatin1("QPixmap(width=%0,height=%1)")
                            .arg(self->width()).arg(self->height()));
    }
}

/////////////////////////////////////////////////////////////

QScriptValue constructPixmapClass(QScriptEngine *eng)
{
    QScriptValue proto = newPixmap(eng, QPixmap());
    ADD_METHOD(proto, alphaChannel);
    ADD_METHOD(proto, cacheKey);
    ADD_METHOD(proto, copy);
    ADD_METHOD(proto, createHeuristicMask);
    ADD_METHOD(proto, createMaskFromColor);
    ADD_METHOD(proto, depth);
    ADD_METHOD(proto, detach);
    ADD_METHOD(proto, fill);
    ADD_METHOD(proto, handle);
    ADD_METHOD(proto, hasAlpha);
    ADD_METHOD(proto, hasAlphaChannel);
    ADD_METHOD(proto, height);
    ADD_METHOD(proto, isNull);
    ADD_METHOD(proto, isQBitmap);
    ADD_METHOD(proto, load);
    ADD_METHOD(proto, loadFromData);
    ADD_METHOD(proto, mask);
    ADD_METHOD(proto, rect);
    ADD_METHOD(proto, save);
    ADD_METHOD(proto, scaled);
    ADD_METHOD(proto, scaledToHeight);
    ADD_METHOD(proto, scaledToWidth);
    ADD_METHOD(proto, setAlphaChannel);
    ADD_METHOD(proto, setMask);
    ADD_METHOD(proto, size);
    ADD_METHOD(proto, toImage);
    ADD_METHOD(proto, toMacCGImageRef);
    ADD_METHOD(proto, toString);
    ADD_METHOD(proto, toWinHBITMAP);
    ADD_METHOD(proto, transformed);
    ADD_METHOD(proto, width);
    ADD_METHOD(proto, x11Info);
    ADD_METHOD(proto, x11PictureHandle);

    eng->setDefaultPrototype(qMetaTypeId<QPixmap>(), proto);
    eng->setDefaultPrototype(qMetaTypeId<QPixmap*>(), proto);

    return eng->newFunction(ctor, proto);
}
