#include <QtScript/QScriptValue>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptContext>
#include <QtCore/QVector>
#include <QtGui/QRegion>
#include "../global.h"

Q_DECLARE_METATYPE(QBitmap*)
Q_DECLARE_METATYPE(QRegion)
Q_DECLARE_METATYPE(QRegion*)
Q_DECLARE_METATYPE(QVector<QRect>)

static inline QScriptValue newRegion(QScriptEngine *eng, const QRegion &region)
{
    return eng->newVariant(qVariantFromValue(region));
}

/////////////////////////////////////////////////////////////

static QScriptValue ctor(QScriptContext *ctx, QScriptEngine *eng)
{
    if (ctx->argumentCount() == 0)
        return newRegion(eng, QRegion());
    if (QRegion *other = qscriptvalue_cast<QRegion*>(ctx->argument(0)))
        return newRegion(eng, QRegion(*other));
    if (QBitmap *bitmap = qscriptvalue_cast<QBitmap*>(ctx->argument(0)))
        return newRegion(eng, QRegion(*bitmap));
    // ### other constructors
    return ctx->throwError("QRegion constructor");
}

/////////////////////////////////////////////////////////////

static QScriptValue boundingRect(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QRegion, boundingRect);
    return eng->toScriptValue(self->boundingRect());
}

/////////////////////////////////////////////////////////////

static QScriptValue contains(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QRegion, contains);
    QRect r = qscriptvalue_cast<QRect>(ctx->argument(0));
    if (r.isValid())
        return QScriptValue(eng, self->contains(r));
    QPoint p = qscriptvalue_cast<QPoint>(ctx->argument(0));
    return QScriptValue(eng, self->contains(p));
}

/////////////////////////////////////////////////////////////

static QScriptValue handle(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(QRegion, handle);
    return ctx->throwError("QRegion.prototype.handle is not implemented");
}

/////////////////////////////////////////////////////////////

static QScriptValue intersected(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QRegion, intersected);
    QRegion r = qscriptvalue_cast<QRegion>(ctx->argument(0));
    return newRegion(eng, self->intersected(r));
}

/////////////////////////////////////////////////////////////

static QScriptValue intersects(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QRegion, intersects);
    QRect r = qscriptvalue_cast<QRect>(ctx->argument(0));
    if (r.isValid())
        return QScriptValue(eng, self->intersects(r));
    return QScriptValue(eng, self->intersects(qscriptvalue_cast<QRegion>(ctx->argument(0))));
}

/////////////////////////////////////////////////////////////

static QScriptValue isEmpty(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QRegion, isEmpty);
    return QScriptValue(eng, self->isEmpty());
}

/////////////////////////////////////////////////////////////

static QScriptValue rects(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QRegion, rects);
    return eng->toScriptValue(self->rects());
}

/////////////////////////////////////////////////////////////

static QScriptValue setRects(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QRegion, setRects);
    QVector<QRect> rects = qscriptvalue_cast<QVector<QRect> >(ctx->argument(0));
    const QRect* rectsData = reinterpret_cast<const QRect*>(rects.constData());
    self->setRects(rectsData, rects.size());
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue subtracted(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QRegion, subtracted);
    QRegion r = qscriptvalue_cast<QRegion>(ctx->argument(0));
    return newRegion(eng, self->subtracted(r));
}

/////////////////////////////////////////////////////////////

static QScriptValue translate(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QRegion, translate);
    if (ctx->argumentCount() == 2) {
        self->translate(ctx->argument(0).toInt32(),
                        ctx->argument(1).toInt32());
    } else {
        self->translate(qscriptvalue_cast<QPoint>(ctx->argument(0)));
    }
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue translated(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QRegion, translated);
    if (ctx->argumentCount() == 2) {
        return newRegion(eng, self->translated(ctx->argument(0).toInt32(),
                                               ctx->argument(1).toInt32()));
    } else {
        return newRegion(eng, self->translated(qscriptvalue_cast<QPoint>(ctx->argument(0))));
    }
}

/////////////////////////////////////////////////////////////

static QScriptValue united(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QRegion, united);
    QRegion r = qscriptvalue_cast<QRegion>(ctx->argument(0));
    return newRegion(eng, self->united(r));
}

/////////////////////////////////////////////////////////////

static QScriptValue xored(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QRegion, xored);
    QRegion r = qscriptvalue_cast<QRegion>(ctx->argument(0));
    return newRegion(eng, self->xored(r));
}

/////////////////////////////////////////////////////////////

static QScriptValue toString(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QRegion, toString);
    return QScriptValue(eng, "QRegion");
}

/////////////////////////////////////////////////////////////

QScriptValue constructRegionClass(QScriptEngine *eng)
{
    QScriptValue proto = newRegion(eng, QRegion());
    ADD_METHOD(proto, boundingRect);
    ADD_METHOD(proto, contains);
    ADD_METHOD(proto, handle);
    ADD_METHOD(proto, intersected);
    ADD_METHOD(proto, intersects);
    ADD_METHOD(proto, isEmpty);
    ADD_METHOD(proto, rects);
    ADD_METHOD(proto, setRects);
    ADD_METHOD(proto, subtracted);
    ADD_METHOD(proto, toString);
    ADD_METHOD(proto, translate);
    ADD_METHOD(proto, translated);
    ADD_METHOD(proto, united);
    ADD_METHOD(proto, xored);

    eng->setDefaultPrototype(qMetaTypeId<QRegion>(), proto);
    eng->setDefaultPrototype(qMetaTypeId<QRegion*>(), proto);

    qScriptRegisterSequenceMetaType<QVector<QRect> >(eng);

    return eng->newFunction(ctor, proto);
}
