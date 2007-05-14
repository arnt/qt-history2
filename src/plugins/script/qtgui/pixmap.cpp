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
    DECLARE_SELF(Pixmap, alphaChannel);
    return newPixmap(eng, self->alphaChannel());
}

/////////////////////////////////////////////////////////////

static QScriptValue cacheKey(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Pixmap, cacheKey);
    return eng->toScriptValue(self->cacheKey());
}

/////////////////////////////////////////////////////////////

static QScriptValue copy(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Pixmap, copy);
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
    DECLARE_SELF(Pixmap, createHeuristicMask);
    return ctx->throwError("QPixmap.prototype.createHeuristicMask is not implemented");
}

/////////////////////////////////////////////////////////////

static QScriptValue createMaskFromColor(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(Pixmap, createMaskFromColor);
    return ctx->throwError("QPixmap.prototype.createMaskFromColor is not implemented");
}

/////////////////////////////////////////////////////////////

static QScriptValue depth(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Pixmap, depth);
    return QScriptValue(eng, self->depth());
}

/////////////////////////////////////////////////////////////

static QScriptValue detach(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Pixmap, detach);
    self->detach();
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue fill(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Pixmap, fill);
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
    DECLARE_SELF(Pixmap, handle);
    return ctx->throwError("QPixmap.prototype.handle is not implemented");
}

/////////////////////////////////////////////////////////////

static QScriptValue hasAlpha(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Pixmap, hasAlpha);
    return QScriptValue(eng, self->hasAlpha());
}

/////////////////////////////////////////////////////////////

static QScriptValue hasAlphaChannel(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Pixmap, hasAlphaChannel);
    return QScriptValue(eng, self->hasAlphaChannel());
}

/////////////////////////////////////////////////////////////

static QScriptValue height(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Pixmap, height);
    return QScriptValue(eng, self->height());
}

/////////////////////////////////////////////////////////////

static QScriptValue isNull(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Pixmap, isNull);
    return QScriptValue(eng, self->isNull());
}

/////////////////////////////////////////////////////////////

static QScriptValue isQBitmap(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Pixmap, isQBitmap);
    return QScriptValue(eng, self->isQBitmap());
}

/////////////////////////////////////////////////////////////

static QScriptValue load(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Pixmap, load);
    return QScriptValue(eng, self->load(ctx->argument(0).toString()));
}

/////////////////////////////////////////////////////////////

static QScriptValue loadFromData(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(Pixmap, loadFromData);
    return ctx->throwError("QPixmap.prototype.loadFromData is not implemented");
}

/////////////////////////////////////////////////////////////

static QScriptValue mask(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Pixmap, mask);
    return eng->toScriptValue(self->mask());
}

/////////////////////////////////////////////////////////////

static QScriptValue rect(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Pixmap, rect);
    return eng->toScriptValue(self->rect());
}

/////////////////////////////////////////////////////////////

static QScriptValue save(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Pixmap, save);
    return QScriptValue(eng, self->save(ctx->argument(0).toString()));
}

/////////////////////////////////////////////////////////////

static QScriptValue scaled(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(Pixmap, scaled);
    return ctx->throwError("QPixmap.prototype.scaled is not implemented");
}

/////////////////////////////////////////////////////////////

static QScriptValue scaledToHeight(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(Pixmap, scaledToHeight);
    return ctx->throwError("QPixmap.prototype.scaledToHeight is not implemented");
}

/////////////////////////////////////////////////////////////

static QScriptValue scaledToWidth(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(Pixmap, scaledToWidth);
    return ctx->throwError("QPixmap.prototype.scaledToWidth is not implemented");
}

/////////////////////////////////////////////////////////////

static QScriptValue setAlphaChannel(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Pixmap, setAlphaChannel);
    self->setAlphaChannel(qscriptvalue_cast<QPixmap>(ctx->argument(0)));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setMask(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Pixmap, setMask);
    self->setMask(qscriptvalue_cast<QBitmap>(ctx->argument(0)));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue size(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Pixmap, size);
    return eng->toScriptValue(self->size());
}

/////////////////////////////////////////////////////////////

static QScriptValue toImage(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Pixmap, toImage);
    return eng->toScriptValue(self->toImage());
}

/////////////////////////////////////////////////////////////

static QScriptValue toMacCGImageRef(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(Pixmap, toMacCGImageRef);
    return ctx->throwError("QPixmap.prototype.toMacCGImageRef is not implemented");
}

/////////////////////////////////////////////////////////////

static QScriptValue toWinHBITMAP(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(Pixmap, toWinHBITMAP);
    return ctx->throwError("QPixmap.prototype.toWinHBITMAP is not implemented");
}

/////////////////////////////////////////////////////////////

static QScriptValue transformed(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(Pixmap, transformed);
    return ctx->throwError("QPixmap.prototype.transformed is not implemented");
}

/////////////////////////////////////////////////////////////

static QScriptValue width(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Pixmap, width);
    return QScriptValue(eng, self->width());
}

/////////////////////////////////////////////////////////////

static QScriptValue x11Info(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(Pixmap, x11Info);
    return ctx->throwError("QPixmap.prototype.x11Info is not implemented");
}

/////////////////////////////////////////////////////////////

static QScriptValue x11PictureHandle(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(Pixmap, x11PictureHandle);
    return ctx->throwError("QPixmap.prototype.x11PictureHandle is not implemented");
}

/////////////////////////////////////////////////////////////

static QScriptValue toString(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Pixmap, toString);
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
    ADD_PROTO_FUNCTION(proto, alphaChannel);
    ADD_PROTO_FUNCTION(proto, cacheKey);
    ADD_PROTO_FUNCTION(proto, copy);
    ADD_PROTO_FUNCTION(proto, createHeuristicMask);
    ADD_PROTO_FUNCTION(proto, createMaskFromColor);
    ADD_PROTO_FUNCTION(proto, depth);
    ADD_PROTO_FUNCTION(proto, detach);
    ADD_PROTO_FUNCTION(proto, fill);
    ADD_PROTO_FUNCTION(proto, handle);
    ADD_PROTO_FUNCTION(proto, hasAlpha);
    ADD_PROTO_FUNCTION(proto, hasAlphaChannel);
    ADD_PROTO_FUNCTION(proto, height);
    ADD_PROTO_FUNCTION(proto, isNull);
    ADD_PROTO_FUNCTION(proto, isQBitmap);
    ADD_PROTO_FUNCTION(proto, load);
    ADD_PROTO_FUNCTION(proto, loadFromData);
    ADD_PROTO_FUNCTION(proto, mask);
    ADD_PROTO_FUNCTION(proto, rect);
    ADD_PROTO_FUNCTION(proto, save);
    ADD_PROTO_FUNCTION(proto, scaled);
    ADD_PROTO_FUNCTION(proto, scaledToHeight);
    ADD_PROTO_FUNCTION(proto, scaledToWidth);
    ADD_PROTO_FUNCTION(proto, setAlphaChannel);
    ADD_PROTO_FUNCTION(proto, setMask);
    ADD_PROTO_FUNCTION(proto, size);
    ADD_PROTO_FUNCTION(proto, toImage);
    ADD_PROTO_FUNCTION(proto, toMacCGImageRef);
    ADD_PROTO_FUNCTION(proto, toString);
    ADD_PROTO_FUNCTION(proto, toWinHBITMAP);
    ADD_PROTO_FUNCTION(proto, transformed);
    ADD_PROTO_FUNCTION(proto, width);
    ADD_PROTO_FUNCTION(proto, x11Info);
    ADD_PROTO_FUNCTION(proto, x11PictureHandle);

    eng->setDefaultPrototype(qMetaTypeId<QPixmap>(), proto);
    eng->setDefaultPrototype(qMetaTypeId<QPixmap*>(), proto);

    return eng->newFunction(ctor, proto);
}
