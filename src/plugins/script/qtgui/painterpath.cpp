#include <QtScript>
#include <QtGui>
#include "../global.h"

Q_DECLARE_METATYPE(QPainterPath)
Q_DECLARE_METATYPE(QPainterPath*)
Q_DECLARE_METATYPE(QPolygonF)
Q_DECLARE_METATYPE(QList<QPolygonF>)

static inline QScriptValue newPainterPath(QScriptEngine *eng, const QPainterPath &path)
{
    return eng->newVariant(qVariantFromValue(path));
}

/////////////////////////////////////////////////////////////

static QScriptValue ctor(QScriptContext *ctx, QScriptEngine *eng)
{
    QPainterPath result;
    if (ctx->argumentCount() == 1) {
        QScriptValue arg = ctx->argument(0);
        QPainterPath *other = qscriptvalue_cast<QPainterPath*>(arg);
        if (other)
            result = QPainterPath(*other);
        else
            result = QPainterPath(qscriptvalue_cast<QPointF>(arg));
    }
    return newPainterPath(eng, result);
}

/////////////////////////////////////////////////////////////

static QScriptValue addEllipse(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(PainterPath, addEllipse);
    if (ctx->argumentCount() == 1) {
        self->addEllipse(qscriptvalue_cast<QRectF>(ctx->argument(0)));
    } else if (ctx->argumentCount() == 4) {
        self->addEllipse(ctx->argument(0).toNumber(),
                         ctx->argument(1).toNumber(),
                         ctx->argument(2).toNumber(),
                         ctx->argument(3).toNumber());
    }
    return ctx->thisObject();
}

/////////////////////////////////////////////////////////////

static QScriptValue addPath(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(PainterPath, addPath);
    QPainterPath *other = qscriptvalue_cast<QPainterPath*>(ctx->argument(0));
    if (!other) {
        return ctx->throwError(QScriptContext::TypeError,
                               "QPainterPath.prototype.addEllipse: argument is not a PainterPath");
    }
    self->addPath(*other);
    return ctx->thisObject();
}

/////////////////////////////////////////////////////////////

static QScriptValue addPolygon(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(PainterPath, addPolygon);
    self->addPolygon(qscriptvalue_cast<QPolygonF>(ctx->argument(0)));
    return ctx->thisObject();
}

/////////////////////////////////////////////////////////////

static QScriptValue addRect(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(PainterPath, addRect);
    if (ctx->argumentCount() == 1) {
        self->addRect(qscriptvalue_cast<QRectF>(ctx->argument(0)));
    } else if (ctx->argumentCount() == 4) {
        self->addRect(ctx->argument(0).toNumber(),
                      ctx->argument(1).toNumber(),
                      ctx->argument(2).toNumber(),
                      ctx->argument(3).toNumber());
    }
    return ctx->thisObject();
}

/////////////////////////////////////////////////////////////

static QScriptValue addRegion(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(PainterPath, addRegion);
    self->addRegion(qscriptvalue_cast<QRegion>(ctx->argument(0)));
    return ctx->thisObject();
}

/////////////////////////////////////////////////////////////

static QScriptValue addRoundRect(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(PainterPath, addRoundRect);
    if (ctx->argumentCount() == 3) {
        self->addRoundRect(qscriptvalue_cast<QRectF>(ctx->argument(0)),
                           ctx->argument(1).toInt32(),
                           ctx->argument(2).toInt32());
    } else if (ctx->argumentCount() == 6) {
        self->addRoundRect(ctx->argument(0).toNumber(),
                           ctx->argument(1).toNumber(),
                           ctx->argument(2).toNumber(),
                           ctx->argument(3).toNumber(),
                           ctx->argument(4).toInt32(),
                           ctx->argument(5).toInt32());
    }
    return ctx->thisObject();
}

/////////////////////////////////////////////////////////////

static QScriptValue addText(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(PainterPath, addText);
    if (ctx->argumentCount() == 3) {
        self->addText(qscriptvalue_cast<QPointF>(ctx->argument(0)),
                      qscriptvalue_cast<QFont>(ctx->argument(1)),
                      ctx->argument(2).toString());
    } else if (ctx->argumentCount() == 4) {
        self->addText(ctx->argument(0).toNumber(),
                      ctx->argument(1).toNumber(),
                      qscriptvalue_cast<QFont>(ctx->argument(1)),
                      ctx->argument(2).toString());
    }
    return ctx->thisObject();
}

/////////////////////////////////////////////////////////////

static QScriptValue angleAtPercent(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(PainterPath, angleAtPercent);
    qreal t = ctx->argument(0).toNumber();
    return QScriptValue(eng, self->angleAtPercent(t));
}

/////////////////////////////////////////////////////////////

static QScriptValue arcMoveTo(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(PainterPath, arcMoveTo);
    if (ctx->argumentCount() == 2) {
        self->arcMoveTo(qscriptvalue_cast<QRectF>(ctx->argument(0)),
                        ctx->argument(1).toNumber());
    } else if (ctx->argumentCount() == 5) {
        self->arcMoveTo(ctx->argument(0).toNumber(),
                        ctx->argument(1).toNumber(),
                        ctx->argument(2).toNumber(),
                        ctx->argument(3).toNumber(),
                        ctx->argument(4).toNumber());
    }
    return ctx->thisObject();
}

/////////////////////////////////////////////////////////////

static QScriptValue arcTo(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(PainterPath, arcTo);
    if (ctx->argumentCount() == 3) {
        self->arcTo(qscriptvalue_cast<QRectF>(ctx->argument(0)),
                    ctx->argument(1).toNumber(),
                    ctx->argument(2).toNumber());
    } else if (ctx->argumentCount() == 6) {
        self->arcTo(ctx->argument(0).toNumber(),
                    ctx->argument(1).toNumber(),
                    ctx->argument(2).toNumber(),
                    ctx->argument(3).toNumber(),
                    ctx->argument(4).toNumber(),
                    ctx->argument(5).toNumber());
    }
    return ctx->thisObject();
}

/////////////////////////////////////////////////////////////

static QScriptValue boundingRect(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(PainterPath, boundingRect);
    return eng->toScriptValue(self->boundingRect());
}

/////////////////////////////////////////////////////////////

static QScriptValue closeSubpath(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(PainterPath, closeSubpath);
    self->closeSubpath();
    return ctx->thisObject();
}

/////////////////////////////////////////////////////////////

static QScriptValue connectPath(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(PainterPath, connectPath);
    QPainterPath *other = qscriptvalue_cast<QPainterPath*>(ctx->argument(0));
    if (!other) {
        return ctx->throwError(QScriptContext::TypeError,
                               "QPainterPath.prototype.connectPath: argument is not a PainterPath");
    }
    self->connectPath(*other);
    return ctx->thisObject();
}

/////////////////////////////////////////////////////////////

static QScriptValue contains(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(PainterPath, contains);
    bool result = false;
    QScriptValue arg = ctx->argument(0);
    QPainterPath *other = qscriptvalue_cast<QPainterPath*>(arg);
    if (other) {
        result = self->contains(*other);
    } else {
        if (arg.property("width").isValid()) {
            // assume rect
            result = self->contains(qscriptvalue_cast<QRectF>(arg));
        } else {
            result = self->contains(qscriptvalue_cast<QPointF>(arg));
        }
    }
    return QScriptValue(eng, result);
}

/////////////////////////////////////////////////////////////

static QScriptValue controlPointRect(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(PainterPath, controlPointRect);
    return eng->toScriptValue(self->controlPointRect());
}

/////////////////////////////////////////////////////////////

static QScriptValue cubicTo(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(PainterPath, cubicTo);
    if (ctx->argumentCount() == 3) {
        self->cubicTo(qscriptvalue_cast<QPointF>(ctx->argument(0)),
                      qscriptvalue_cast<QPointF>(ctx->argument(1)),
                      qscriptvalue_cast<QPointF>(ctx->argument(2)));
    } else if (ctx->argumentCount() == 6) {
        self->cubicTo(ctx->argument(0).toNumber(),
                      ctx->argument(1).toNumber(),
                      ctx->argument(2).toNumber(),
                      ctx->argument(3).toNumber(),
                      ctx->argument(4).toNumber(),
                      ctx->argument(5).toNumber());
    }
    return ctx->thisObject();
}

/////////////////////////////////////////////////////////////

static QScriptValue currentPosition(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(PainterPath, currentPosition);
    return eng->toScriptValue(self->currentPosition());
}

/////////////////////////////////////////////////////////////

// ### elementAt
// ### elementCount

/////////////////////////////////////////////////////////////

static QScriptValue fillRule(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(PainterPath, fillRule);
    return eng->toScriptValue(static_cast<int>(self->fillRule()));
}

/////////////////////////////////////////////////////////////

static QScriptValue intersected(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(PainterPath, intersected);
    QPainterPath *other = qscriptvalue_cast<QPainterPath*>(ctx->argument(0));
    if (!other) {
        return ctx->throwError(QScriptContext::TypeError,
                               "QPainterPath.prototype.intersected: argument is not a PainterPath");
    }
    return newPainterPath(eng, self->intersected(*other));
}

/////////////////////////////////////////////////////////////

static QScriptValue intersects(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(PainterPath, intersects);
    QScriptValue arg = ctx->argument(0);
    QPainterPath *other = qscriptvalue_cast<QPainterPath*>(arg);
    bool result = false;
    if (other) {
        result = self->intersects(*other);
    } else {
        result = self->intersects(qscriptvalue_cast<QRectF>(arg));
    }
    return QScriptValue(eng, result);
}

/////////////////////////////////////////////////////////////

static QScriptValue isEmpty(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(PainterPath, isEmpty);
    return QScriptValue(eng, self->isEmpty());
}

/////////////////////////////////////////////////////////////

static QScriptValue length(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(PainterPath, length);
    return QScriptValue(eng, self->length());
}

/////////////////////////////////////////////////////////////

static QScriptValue lineTo(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(PainterPath, lineTo);
    if (ctx->argumentCount() == 1) {
        self->lineTo(qscriptvalue_cast<QPointF>(ctx->argument(0)));
    } else if (ctx->argumentCount() == 2) {
        self->lineTo(ctx->argument(0).toNumber(),
                     ctx->argument(1).toNumber());
    }
    return ctx->thisObject();
}

/////////////////////////////////////////////////////////////

static QScriptValue moveTo(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(PainterPath, moveTo);
    if (ctx->argumentCount() == 1) {
        self->moveTo(qscriptvalue_cast<QPointF>(ctx->argument(0)));
    } else if (ctx->argumentCount() == 2) {
        self->moveTo(ctx->argument(0).toNumber(),
                     ctx->argument(1).toNumber());
    }
    return ctx->thisObject();
}

/////////////////////////////////////////////////////////////

static QScriptValue percentAtLength(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(PainterPath, percentAtLength);
    qreal len = ctx->argument(0).toNumber();
    return QScriptValue(eng, self->percentAtLength(len));
}

/////////////////////////////////////////////////////////////

static QScriptValue pointAtPercent(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(PainterPath, pointAtPercent);
    qreal t = ctx->argument(0).toNumber();
    return eng->toScriptValue(self->pointAtPercent(t));
}

/////////////////////////////////////////////////////////////

static QScriptValue quadTo(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(PainterPath, quadTo);
    if (ctx->argumentCount() == 2) {
        self->quadTo(qscriptvalue_cast<QPointF>(ctx->argument(0)),
                     qscriptvalue_cast<QPointF>(ctx->argument(1)));
    } else if (ctx->argumentCount() == 4) {
        self->quadTo(ctx->argument(0).toNumber(),
                     ctx->argument(1).toNumber(),
                     ctx->argument(2).toNumber(),
                     ctx->argument(3).toNumber());
    }
    return ctx->thisObject();
}

// ### setElementPositionAt

/////////////////////////////////////////////////////////////

static QScriptValue setFillRule(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(PainterPath, setFillRule);
    self->setFillRule(static_cast<Qt::FillRule>(ctx->argument(0).toInt32()));
    return ctx->thisObject();
}

/////////////////////////////////////////////////////////////

static QScriptValue slopeAtPercent(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(PainterPath, slopeAtPercent);
    qreal t = ctx->argument(0).toNumber();
    return QScriptValue(eng, self->slopeAtPercent(t));
}

/////////////////////////////////////////////////////////////

static QScriptValue subtracted(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(PainterPath, subtracted);
    QPainterPath *other = qscriptvalue_cast<QPainterPath*>(ctx->argument(0));
    if (!other) {
        return ctx->throwError(QScriptContext::TypeError,
                               "QPainterPath.prototype.subtracted: argument is not a PainterPath");
    }
    return newPainterPath(eng, self->subtracted(*other));
}

/////////////////////////////////////////////////////////////

static QScriptValue subtractedInverted(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(PainterPath, subtractedInverted);
    QPainterPath *other = qscriptvalue_cast<QPainterPath*>(ctx->argument(0));
    if (!other) {
        return ctx->throwError(QScriptContext::TypeError,
                               "QPainterPath.prototype.subtractedInverted: argument is not a PainterPath");
    }
    return newPainterPath(eng, self->subtractedInverted(*other));
}

/////////////////////////////////////////////////////////////

static QScriptValue toFillPolygon(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(PainterPath, toFillPolygon);
    // ### QTransform overload
    QMatrix matrix = qscriptvalue_cast<QMatrix>(ctx->argument(0));
    return eng->toScriptValue(self->toFillPolygon(matrix));
}

/////////////////////////////////////////////////////////////

static QScriptValue toFillPolygons(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(PainterPath, toFillPolygons);
    // ### QTransform overload
    QMatrix matrix = qscriptvalue_cast<QMatrix>(ctx->argument(0));
    return eng->toScriptValue(self->toFillPolygons(matrix));
}

/////////////////////////////////////////////////////////////

static QScriptValue toReversed(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(PainterPath, toReversed);
    return newPainterPath(eng, self->toReversed());
}

/////////////////////////////////////////////////////////////

static QScriptValue toSubpathPolygons(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(PainterPath, toSubpathPolygons);
    // ### QTransform overload
    QMatrix matrix = qscriptvalue_cast<QMatrix>(ctx->argument(0));
    return eng->toScriptValue(self->toSubpathPolygons(matrix));
}

/////////////////////////////////////////////////////////////

static QScriptValue united(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(PainterPath, united);
    QPainterPath *other = qscriptvalue_cast<QPainterPath*>(ctx->argument(0));
    if (!other) {
        return ctx->throwError(QScriptContext::TypeError,
                               "QPainterPath.prototype.united: argument is not a PainterPath");
    }
    return newPainterPath(eng, self->united(*other));
}

/////////////////////////////////////////////////////////////

static QScriptValue toString(QScriptContext *, QScriptEngine *eng)
{
    return QScriptValue(eng, "QPainterPath");
}

/////////////////////////////////////////////////////////////

QScriptValue constructPainterPathClass(QScriptEngine *eng)
{
    QScriptValue proto = qScriptValueFromValue(eng, QPainterPath());
    ADD_PROTO_FUNCTION(proto, addEllipse);
    ADD_PROTO_FUNCTION(proto, addPath);
    ADD_PROTO_FUNCTION(proto, addPolygon);
    ADD_PROTO_FUNCTION(proto, addRect);
    ADD_PROTO_FUNCTION(proto, addRegion);
    ADD_PROTO_FUNCTION(proto, addRoundRect);
    ADD_PROTO_FUNCTION(proto, addText);
    ADD_PROTO_FUNCTION(proto, angleAtPercent);
    ADD_PROTO_FUNCTION(proto, arcMoveTo);
    ADD_PROTO_FUNCTION(proto, arcTo);
    ADD_PROTO_FUNCTION(proto, boundingRect);
    ADD_PROTO_FUNCTION(proto, closeSubpath);
    ADD_PROTO_FUNCTION(proto, connectPath);
    ADD_PROTO_FUNCTION(proto, contains);
    ADD_PROTO_FUNCTION(proto, controlPointRect);
    ADD_PROTO_FUNCTION(proto, cubicTo);
    ADD_PROTO_FUNCTION(proto, currentPosition);
    ADD_PROTO_FUNCTION(proto, fillRule);
    ADD_PROTO_FUNCTION(proto, intersected);
    ADD_PROTO_FUNCTION(proto, intersects);
    ADD_PROTO_FUNCTION(proto, isEmpty);
    ADD_PROTO_FUNCTION(proto, length);
    ADD_PROTO_FUNCTION(proto, lineTo);
    ADD_PROTO_FUNCTION(proto, moveTo);
    ADD_PROTO_FUNCTION(proto, percentAtLength);
    ADD_PROTO_FUNCTION(proto, pointAtPercent);
    ADD_PROTO_FUNCTION(proto, quadTo);
    ADD_PROTO_FUNCTION(proto, setFillRule);
    ADD_PROTO_FUNCTION(proto, slopeAtPercent);
    ADD_PROTO_FUNCTION(proto, subtracted);
    ADD_PROTO_FUNCTION(proto, subtractedInverted);
    ADD_PROTO_FUNCTION(proto, toFillPolygon);
    ADD_PROTO_FUNCTION(proto, toFillPolygons);
    ADD_PROTO_FUNCTION(proto, toReversed);
    ADD_PROTO_FUNCTION(proto, toSubpathPolygons);
    ADD_PROTO_FUNCTION(proto, united);
    ADD_PROTO_FUNCTION(proto, toString);

    eng->setDefaultPrototype(qMetaTypeId<QPainterPath>(), proto);

    qScriptRegisterSequenceMetaType<QList<QPolygonF> >(eng);

    return eng->newFunction(ctor, proto);
}
