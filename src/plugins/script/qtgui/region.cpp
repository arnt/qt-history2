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
    return ctx->throwError("Region constructor");
}

/////////////////////////////////////////////////////////////

static QScriptValue boundingRect(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Region, boundingRect);
    return eng->toScriptValue(self->boundingRect());
}

/////////////////////////////////////////////////////////////

static QScriptValue contains(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Region, contains);
    QRect r = qscriptvalue_cast<QRect>(ctx->argument(0));
    if (r.isValid())
        return QScriptValue(eng, self->contains(r));
    QPoint p = qscriptvalue_cast<QPoint>(ctx->argument(0));
    return QScriptValue(eng, self->contains(p));
}

/////////////////////////////////////////////////////////////

static QScriptValue handle(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(Region, handle);
    return ctx->throwError("Region.prototype.handle is not implemented");
}

/////////////////////////////////////////////////////////////

static QScriptValue intersected(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Region, intersected);
    QRegion r = qscriptvalue_cast<QRegion>(ctx->argument(0));
    return newRegion(eng, self->intersected(r));
}

/////////////////////////////////////////////////////////////

static QScriptValue intersects(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Region, intersects);
    QRect r = qscriptvalue_cast<QRect>(ctx->argument(0));
    if (r.isValid())
        return QScriptValue(eng, self->intersects(r));
    return QScriptValue(eng, self->intersects(qscriptvalue_cast<QRegion>(ctx->argument(0))));
}

/////////////////////////////////////////////////////////////

static QScriptValue isEmpty(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Region, isEmpty);
    return QScriptValue(eng, self->isEmpty());
}

/////////////////////////////////////////////////////////////

static QScriptValue rects(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Region, rects);
    return eng->toScriptValue(self->rects());
}

/////////////////////////////////////////////////////////////

static QScriptValue setRects(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Region, setRects);
    QVector<QRect> rects = qscriptvalue_cast<QVector<QRect> >(ctx->argument(0));
    const QRect* rectsData = reinterpret_cast<const QRect*>(rects.constData());
    self->setRects(rectsData, rects.size());
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue subtracted(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Region, subtracted);
    QRegion r = qscriptvalue_cast<QRegion>(ctx->argument(0));
    return newRegion(eng, self->subtracted(r));
}

/////////////////////////////////////////////////////////////

static QScriptValue translate(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Region, translate);
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
    DECLARE_SELF(Region, translated);
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
    DECLARE_SELF(Region, united);
    QRegion r = qscriptvalue_cast<QRegion>(ctx->argument(0));
    return newRegion(eng, self->united(r));
}

/////////////////////////////////////////////////////////////

static QScriptValue xored(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Region, xored);
    QRegion r = qscriptvalue_cast<QRegion>(ctx->argument(0));
    return newRegion(eng, self->xored(r));
}

/////////////////////////////////////////////////////////////

static QScriptValue toString(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Region, toString);
    return QScriptValue(eng, "Region");
}

/////////////////////////////////////////////////////////////

QScriptValue constructRegionClass(QScriptEngine *eng)
{
    QScriptValue proto = newRegion(eng, QRegion());
    ADD_PROTO_FUNCTION(proto, boundingRect);
    ADD_PROTO_FUNCTION(proto, contains);
    ADD_PROTO_FUNCTION(proto, handle);
    ADD_PROTO_FUNCTION(proto, intersected);
    ADD_PROTO_FUNCTION(proto, intersects);
    ADD_PROTO_FUNCTION(proto, isEmpty);
    ADD_PROTO_FUNCTION(proto, rects);
    ADD_PROTO_FUNCTION(proto, setRects);
    ADD_PROTO_FUNCTION(proto, subtracted);
    ADD_PROTO_FUNCTION(proto, toString);
    ADD_PROTO_FUNCTION(proto, translate);
    ADD_PROTO_FUNCTION(proto, translated);
    ADD_PROTO_FUNCTION(proto, united);
    ADD_PROTO_FUNCTION(proto, xored);

    eng->setDefaultPrototype(qMetaTypeId<QRegion>(), proto);
    eng->setDefaultPrototype(qMetaTypeId<QRegion*>(), proto);

    qScriptRegisterSequenceMetaType<QVector<QRect> >(eng);

    return eng->newFunction(ctor, proto);
}
