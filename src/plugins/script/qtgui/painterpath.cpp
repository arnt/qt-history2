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
    DECLARE_SELF(QPainterPath, addEllipse);
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
    DECLARE_SELF(QPainterPath, addPath);
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
    DECLARE_SELF(QPainterPath, addPolygon);
    self->addPolygon(qscriptvalue_cast<QPolygonF>(ctx->argument(0)));
    return ctx->thisObject();
}

/////////////////////////////////////////////////////////////

static QScriptValue addRect(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(QPainterPath, addRect);
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
    DECLARE_SELF(QPainterPath, addRegion);
    self->addRegion(qscriptvalue_cast<QRegion>(ctx->argument(0)));
    return ctx->thisObject();
}

/////////////////////////////////////////////////////////////

static QScriptValue addRoundRect(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(QPainterPath, addRoundRect);
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
    DECLARE_SELF(QPainterPath, addText);
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
    DECLARE_SELF(QPainterPath, angleAtPercent);
    qreal t = ctx->argument(0).toNumber();
    return QScriptValue(eng, self->angleAtPercent(t));
}

/////////////////////////////////////////////////////////////

static QScriptValue arcMoveTo(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(QPainterPath, arcMoveTo);
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
    DECLARE_SELF(QPainterPath, arcTo);
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
    DECLARE_SELF(QPainterPath, boundingRect);
    return eng->toScriptValue(self->boundingRect());
}

/////////////////////////////////////////////////////////////

static QScriptValue closeSubpath(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(QPainterPath, closeSubpath);
    self->closeSubpath();
    return ctx->thisObject();
}

/////////////////////////////////////////////////////////////

static QScriptValue connectPath(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(QPainterPath, connectPath);
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
    DECLARE_SELF(QPainterPath, contains);
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
    DECLARE_SELF(QPainterPath, controlPointRect);
    return eng->toScriptValue(self->controlPointRect());
}

/////////////////////////////////////////////////////////////

static QScriptValue cubicTo(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(QPainterPath, cubicTo);
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
    DECLARE_SELF(QPainterPath, currentPosition);
    return eng->toScriptValue(self->currentPosition());
}

/////////////////////////////////////////////////////////////

// ### elementAt
// ### elementCount

/////////////////////////////////////////////////////////////

static QScriptValue fillRule(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPainterPath, fillRule);
    return eng->toScriptValue(static_cast<int>(self->fillRule()));
}

/////////////////////////////////////////////////////////////

static QScriptValue intersected(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPainterPath, intersected);
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
    DECLARE_SELF(QPainterPath, intersects);
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
    DECLARE_SELF(QPainterPath, isEmpty);
    return QScriptValue(eng, self->isEmpty());
}

/////////////////////////////////////////////////////////////

static QScriptValue length(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPainterPath, length);
    return QScriptValue(eng, self->length());
}

/////////////////////////////////////////////////////////////

static QScriptValue lineTo(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(QPainterPath, lineTo);
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
    DECLARE_SELF(QPainterPath, moveTo);
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
    DECLARE_SELF(QPainterPath, percentAtLength);
    qreal len = ctx->argument(0).toNumber();
    return QScriptValue(eng, self->percentAtLength(len));
}

/////////////////////////////////////////////////////////////

static QScriptValue pointAtPercent(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPainterPath, pointAtPercent);
    qreal t = ctx->argument(0).toNumber();
    return eng->toScriptValue(self->pointAtPercent(t));
}

/////////////////////////////////////////////////////////////

static QScriptValue quadTo(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(QPainterPath, quadTo);
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
    DECLARE_SELF(QPainterPath, setFillRule);
    self->setFillRule(static_cast<Qt::FillRule>(ctx->argument(0).toInt32()));
    return ctx->thisObject();
}

/////////////////////////////////////////////////////////////

static QScriptValue slopeAtPercent(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPainterPath, slopeAtPercent);
    qreal t = ctx->argument(0).toNumber();
    return QScriptValue(eng, self->slopeAtPercent(t));
}

/////////////////////////////////////////////////////////////

static QScriptValue subtracted(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPainterPath, subtracted);
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
    DECLARE_SELF(QPainterPath, subtractedInverted);
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
    DECLARE_SELF(QPainterPath, toFillPolygon);
    // ### QTransform overload
    QMatrix matrix = qscriptvalue_cast<QMatrix>(ctx->argument(0));
    return eng->toScriptValue(self->toFillPolygon(matrix));
}

/////////////////////////////////////////////////////////////

static QScriptValue toFillPolygons(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPainterPath, toFillPolygons);
    // ### QTransform overload
    QMatrix matrix = qscriptvalue_cast<QMatrix>(ctx->argument(0));
    return eng->toScriptValue(self->toFillPolygons(matrix));
}

/////////////////////////////////////////////////////////////

static QScriptValue toReversed(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPainterPath, toReversed);
    return newPainterPath(eng, self->toReversed());
}

/////////////////////////////////////////////////////////////

static QScriptValue toSubpathPolygons(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPainterPath, toSubpathPolygons);
    // ### QTransform overload
    QMatrix matrix = qscriptvalue_cast<QMatrix>(ctx->argument(0));
    return eng->toScriptValue(self->toSubpathPolygons(matrix));
}

/////////////////////////////////////////////////////////////

static QScriptValue united(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPainterPath, united);
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
    ADD_METHOD(proto, addEllipse);
    ADD_METHOD(proto, addPath);
    ADD_METHOD(proto, addPolygon);
    ADD_METHOD(proto, addRect);
    ADD_METHOD(proto, addRegion);
    ADD_METHOD(proto, addRoundRect);
    ADD_METHOD(proto, addText);
    ADD_METHOD(proto, angleAtPercent);
    ADD_METHOD(proto, arcMoveTo);
    ADD_METHOD(proto, arcTo);
    ADD_METHOD(proto, boundingRect);
    ADD_METHOD(proto, closeSubpath);
    ADD_METHOD(proto, connectPath);
    ADD_METHOD(proto, contains);
    ADD_METHOD(proto, controlPointRect);
    ADD_METHOD(proto, cubicTo);
    ADD_METHOD(proto, currentPosition);
    ADD_METHOD(proto, fillRule);
    ADD_METHOD(proto, intersected);
    ADD_METHOD(proto, intersects);
    ADD_METHOD(proto, isEmpty);
    ADD_METHOD(proto, length);
    ADD_METHOD(proto, lineTo);
    ADD_METHOD(proto, moveTo);
    ADD_METHOD(proto, percentAtLength);
    ADD_METHOD(proto, pointAtPercent);
    ADD_METHOD(proto, quadTo);
    ADD_METHOD(proto, setFillRule);
    ADD_METHOD(proto, slopeAtPercent);
    ADD_METHOD(proto, subtracted);
    ADD_METHOD(proto, subtractedInverted);
    ADD_METHOD(proto, toFillPolygon);
    ADD_METHOD(proto, toFillPolygons);
    ADD_METHOD(proto, toReversed);
    ADD_METHOD(proto, toSubpathPolygons);
    ADD_METHOD(proto, united);
    ADD_METHOD(proto, toString);

    eng->setDefaultPrototype(qMetaTypeId<QPainterPath>(), proto);

    qScriptRegisterSequenceMetaType<QList<QPolygonF> >(eng);

    return eng->newFunction(ctor, proto);
}
