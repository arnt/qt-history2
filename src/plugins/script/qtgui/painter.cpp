#include <QtScript/QScriptValue>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptContext>

#include <QtGui/QPainter>
#include <QtGui/QPicture>
#include <QtGui/QPolygonF>
#include <QtGui/QWidget>

#include <QtCore/qdebug.h>

#include "../global.h"

Q_DECLARE_METATYPE(QPolygonF)
Q_DECLARE_METATYPE(QPainterPath)
Q_DECLARE_METATYPE(QPainterPath*)
Q_DECLARE_METATYPE(QPicture)
Q_DECLARE_METATYPE(QVector<QRectF>)
Q_DECLARE_METATYPE(QPaintDevice*)
Q_DECLARE_METATYPE(QPaintEngine*)

DECLARE_POINTER_METATYPE(QPainter)

static QScriptValue newPainter(QScriptEngine *eng, QPainter *p)
{
    return QScript::wrapPointer(eng, p);
}

/////////////////////////////////////////////////////////////

static QScriptValue ctor(QScriptContext *ctx, QScriptEngine *eng)
{
    if (ctx->argumentCount() > 0) {
        QPaintDevice *device = qscriptvalue_cast<QPaintDevice*>(ctx->argument(0));
        return newPainter(eng, new QPainter(device));
    } else {
        return newPainter(eng, new QPainter());
    }
}

/////////////////////////////////////////////////////////////

static QScriptValue background(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Painter, background);
    return eng->toScriptValue(self->background());
}

/////////////////////////////////////////////////////////////

static QScriptValue backgroundMode(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Painter, backgroundMode);
    return QScriptValue(eng, static_cast<int>(self->backgroundMode()));
}

/////////////////////////////////////////////////////////////

static QScriptValue begin(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Painter, begin);
    QWidget *device = qscriptvalue_cast<QWidget*>(ctx->argument(0));
    if (!device) {
        return ctx->throwError(QScriptContext::TypeError,
                               "QPainter.prototype.begin: argument is not a QWidget");
    }
    return QScriptValue(eng, self->begin(device));
}

/////////////////////////////////////////////////////////////

static QScriptValue boundingRect(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Painter, boundingRect);
    QRect result;
    if (ctx->argumentCount() == 3) {
        result = self->boundingRect(qscriptvalue_cast<QRect>(ctx->argument(0)),
                                    ctx->argument(1).toInt32(),
                                    ctx->argument(2).toString());
    } else if (ctx->argumentCount() == 6) {
        result = self->boundingRect(ctx->argument(0).toInt32(),
                                    ctx->argument(1).toInt32(),
                                    ctx->argument(2).toInt32(),
                                    ctx->argument(3).toInt32(),
                                    ctx->argument(4).toInt32(),
                                    ctx->argument(5).toString());
    }
    return eng->toScriptValue(result);
}

/////////////////////////////////////////////////////////////

static QScriptValue brush(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Painter, brush);
    return eng->toScriptValue(self->brush());
}

/////////////////////////////////////////////////////////////

static QScriptValue brushOrigin(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Painter, brushOrigin);
    return eng->toScriptValue(self->brushOrigin());
}

/////////////////////////////////////////////////////////////

static QScriptValue clipPath(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Painter, clipPath);
    return eng->toScriptValue(self->clipPath());
}

/////////////////////////////////////////////////////////////

static QScriptValue clipRegion(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Painter, clipRegion);
    return eng->toScriptValue(self->clipRegion());
}

/////////////////////////////////////////////////////////////

static QScriptValue combinedMatrix(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Painter, combinedMatrix);
    return eng->toScriptValue(self->combinedMatrix());
}

/////////////////////////////////////////////////////////////

static QScriptValue combinedTransform(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Painter, combinedTransform);
    return eng->toScriptValue(self->combinedTransform());
}

/////////////////////////////////////////////////////////////

static QScriptValue compositionMode(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Painter, compositionMode);
    return QScriptValue(eng, static_cast<int>(self->compositionMode()));
}

/////////////////////////////////////////////////////////////

static QScriptValue device(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Painter, device);
    return eng->toScriptValue(self->device());
}

/////////////////////////////////////////////////////////////

static QScriptValue deviceMatrix(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Painter, deviceMatrix);
    return eng->toScriptValue(self->deviceMatrix());
}

/////////////////////////////////////////////////////////////

static QScriptValue deviceTransform(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Painter, deviceTransform);
    return eng->toScriptValue(self->deviceTransform());
}

/////////////////////////////////////////////////////////////

static QScriptValue drawArc(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Painter, drawArc);
    if (ctx->argumentCount() == 6) {
        // drawArc(x, y, height, width, startAngle, spanAngle)
        self->drawArc(ctx->argument(0).toInt32(),
                      ctx->argument(1).toInt32(),
                      ctx->argument(2).toInt32(),
                      ctx->argument(3).toInt32(),
                      ctx->argument(4).toInt32(),
                      ctx->argument(5).toInt32());
    } else if (ctx->argumentCount() == 3) {
        // drawArc(rectangle, startAngle, spanAngle)
        self->drawArc(qscriptvalue_cast<QRectF>(ctx->argument(0)),
                      ctx->argument(1).toInt32(),
                      ctx->argument(2).toInt32());
    }
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue drawChord(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Painter, drawChord);
    if (ctx->argumentCount() == 6) {
        // x, y, height, width, startAngle, spanAngle
        self->drawChord(ctx->argument(0).toInt32(),
                        ctx->argument(1).toInt32(),
                        ctx->argument(2).toInt32(),
                        ctx->argument(3).toInt32(),
                        ctx->argument(4).toInt32(),
                        ctx->argument(5).toInt32());
    } else if (ctx->argumentCount() == 3) {
        // rectangle, startAngle, spanAngle
        self->drawChord(qscriptvalue_cast<QRectF>(ctx->argument(0)),
                        ctx->argument(1).toInt32(),
                        ctx->argument(2).toInt32());
    }
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue drawConvexPolygon(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Painter, drawConvexPolygon);
    self->drawConvexPolygon(qscriptvalue_cast<QPolygonF>(ctx->argument(0)));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue drawEllipse(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Painter, drawEllipse);
    if (ctx->argumentCount() == 4) {
        // drawEllipse(x, y, width, height)
        self->drawEllipse(ctx->argument(0).toInt32(),
                          ctx->argument(1).toInt32(),
                          ctx->argument(2).toInt32(),
                          ctx->argument(3).toInt32());
    } else if (ctx->argumentCount() == 1) {
        // drawEllipse(rect)
        self->drawEllipse(qscriptvalue_cast<QRectF>(ctx->argument(0)));
    }
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue drawImage(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Painter, drawImage);
    if (ctx->argumentCount() == 2) {
        // target, image
        QScriptValue arg0 = ctx->argument(0);
        QImage image = qscriptvalue_cast<QImage>(ctx->argument(1));
        if (arg0.property("width").isValid()) {
            self->drawImage(qscriptvalue_cast<QRectF>(arg0), image);
        } else {
            self->drawImage(qscriptvalue_cast<QPointF>(arg0), image);
        }
    }
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue drawLine(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Painter, drawLine);
    if (ctx->argumentCount() == 4) {
        // x1, y1, x2, y2
        self->drawLine(ctx->argument(0).toInt32(),
                       ctx->argument(1).toInt32(),
                       ctx->argument(2).toInt32(),
                       ctx->argument(3).toInt32());
    } else if (ctx->argumentCount() == 2) {
        // p1, p2
        self->drawLine(qscriptvalue_cast<QPointF>(ctx->argument(0)),
                       qscriptvalue_cast<QPointF>(ctx->argument(1)));
    } else if (ctx->argumentCount() == 1) {
        // line
        self->drawLine(qscriptvalue_cast<QLineF>(ctx->argument(0)));
    }
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue drawLines(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Painter, drawLines);
    return ctx->throwError("QPainter.prototype.drawLines is not implemented");
//    self->drawLines(qscriptvalue_cast<QVector<QLineF> >(ctx->argument(0)));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue drawPath(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Painter, drawPath);
    self->drawPath(qscriptvalue_cast<QPainterPath>(ctx->argument(0)));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue drawPicture(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Painter, drawPicture);
    if (ctx->argumentCount() == 2) {
        self->drawPicture(qscriptvalue_cast<QPointF>(ctx->argument(0)),
                          qscriptvalue_cast<QPicture>(ctx->argument(1)));
    } else if (ctx->argumentCount() == 3) {
        self->drawPicture(ctx->argument(0).toInt32(),
                          ctx->argument(1).toInt32(),
                          qscriptvalue_cast<QPicture>(ctx->argument(2)));
    }
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue drawPie(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Painter, drawPie);
    if (ctx->argumentCount() == 6) {
        // x, y, height, width, startAngle, spanAngle
        self->drawPie(ctx->argument(0).toInt32(),
                      ctx->argument(1).toInt32(),
                      ctx->argument(2).toInt32(),
                      ctx->argument(3).toInt32(),
                      ctx->argument(4).toInt32(),
                      ctx->argument(5).toInt32());
    } else if (ctx->argumentCount() == 3) {
        // rectangle, startAngle, spanAngle
        self->drawPie(qscriptvalue_cast<QRectF>(ctx->argument(0)),
                      ctx->argument(1).toInt32(),
                      ctx->argument(2).toInt32());
    }
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue drawPixmap(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Painter, drawPixmap);
    if (ctx->argumentCount() == 5) {
        // x, y, width, height, pixmap
        self->drawPixmap(ctx->argument(0).toInt32(),
                         ctx->argument(1).toInt32(),
                         ctx->argument(2).toInt32(),
                         ctx->argument(3).toInt32(),
                         qscriptvalue_cast<QPixmap>(ctx->argument(4)));
    }
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue drawPoint(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Painter, drawPoint);
    if (ctx->argumentCount() == 2) {
        // drawPoint(x, y)
        self->drawPoint(ctx->argument(0).toInt32(),
                        ctx->argument(1).toInt32());
    } else if (ctx->argumentCount() == 1) {
        // drawPoint(point)
        self->drawPoint(qscriptvalue_cast<QPointF>(ctx->argument(0)));
    }
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue drawPoints(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Painter, drawPoints);
    self->drawPoints(qscriptvalue_cast<QPolygonF>(ctx->argument(0)));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue drawPolygon(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Painter, drawPolygon);
    // ### fillRule (2nd argument)
    self->drawPolygon(qscriptvalue_cast<QPolygonF>(ctx->argument(0)));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue drawPolyline(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Painter, drawPolyline);
    self->drawPolyline(qscriptvalue_cast<QPolygonF>(ctx->argument(0)));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue drawRect(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Painter, drawRect);
    if (ctx->argumentCount() == 4) {
        // x, y, width, height
        self->drawRect(ctx->argument(0).toInt32(),
                       ctx->argument(1).toInt32(),
                       ctx->argument(2).toInt32(),
                       ctx->argument(3).toInt32());
    } else if (ctx->argumentCount() == 1) {
        // rect
        self->drawRect(qscriptvalue_cast<QRectF>(ctx->argument(0)));
    }
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue drawRects(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Painter, drawRects);
    self->drawRects(qscriptvalue_cast<QVector<QRectF> >(ctx->argument(0)));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue drawRoundRect(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Painter, drawRoundRect);
    // ### xRnd, yRnd
    if (ctx->argumentCount() >= 4) {
        self->drawRoundRect(ctx->argument(0).toInt32(),
                            ctx->argument(1).toInt32(),
                            ctx->argument(2).toInt32(),
                            ctx->argument(3).toInt32());
    } else {
        self->drawRoundRect(qscriptvalue_cast<QRectF>(ctx->argument(0)));
    }
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue drawText(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Painter, drawText);
    if (ctx->argumentCount() == 3) {
        // x, y, text
        self->drawText(ctx->argument(0).toInt32(),
                       ctx->argument(1).toInt32(),
                       ctx->argument(2).toString());
    } else if (ctx->argumentCount() == 2) {
        QScriptValue arg0 = ctx->argument(0);
        if (arg0.property("width").isValid()) {
            self->drawText(qscriptvalue_cast<QRectF>(arg0),
                           ctx->argument(1).toString());
        } else {
            self->drawText(qscriptvalue_cast<QPointF>(arg0),
                           ctx->argument(1).toString());
        }
    }
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue drawTiledPixmap(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Painter, drawTiledPixmap);
    if (ctx->argumentCount() >= 5) {
        // x, y, width, height, pixmap, sx, sy
        self->drawTiledPixmap(ctx->argument(0).toInt32(),
                              ctx->argument(1).toInt32(),
                              ctx->argument(2).toInt32(),
                              ctx->argument(3).toInt32(),
                              qscriptvalue_cast<QPixmap>(ctx->argument(4)),
                              ctx->argument(5).toInt32(),
                              ctx->argument(6).toInt32());
    } else {
        // rect, pixmap, position
        self->drawTiledPixmap(qscriptvalue_cast<QRectF>(ctx->argument(0)),
                              qscriptvalue_cast<QPixmap>(ctx->argument(1)),
                              qscriptvalue_cast<QPointF>(ctx->argument(2)));
    }
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue end(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Painter, end);
    return QScriptValue(eng, self->end());
}

/////////////////////////////////////////////////////////////

static QScriptValue eraseRect(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Painter, eraseRect);
    if (ctx->argumentCount() == 4) {
        // x, y, width, height
        self->eraseRect(ctx->argument(0).toInt32(),
                        ctx->argument(1).toInt32(),
                        ctx->argument(2).toInt32(),
                        ctx->argument(3).toInt32());
    } else if (ctx->argumentCount() == 1) {
        // rect
        self->eraseRect(qscriptvalue_cast<QRectF>(ctx->argument(0)));
    }
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue fillPath(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Painter, fillPath);
    QPainterPath *path = qscriptvalue_cast<QPainterPath*>(ctx->argument(0));
    if (!path) {
        return ctx->throwError(QScriptContext::TypeError,
                               "QPainter.prototype.fillPath: argument is not a PainterPath");
    }
    self->fillPath(*path, qscriptvalue_cast<QBrush>(ctx->argument(1)));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue fillRect(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Painter, fillRect);
    if (ctx->argumentCount() == 4) {
        // x, y, width, height, brush
        self->fillRect(ctx->argument(0).toInt32(),
                       ctx->argument(1).toInt32(),
                       ctx->argument(2).toInt32(),
                       ctx->argument(3).toInt32(),
                       qscriptvalue_cast<QBrush>(ctx->argument(4)));
    } else if (ctx->argumentCount() == 1) {
        // rect, brush
        self->fillRect(qscriptvalue_cast<QRectF>(ctx->argument(0)),
                       qscriptvalue_cast<QBrush>(ctx->argument(1)));
    }
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue font(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Painter, font);
    return qScriptValueFromValue(eng, self->font());
}

/////////////////////////////////////////////////////////////

static QScriptValue fontInfo(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(Painter, fontInfo);
    return ctx->throwError("QPainter.prototype.fontInfo is not implemented");
}

/////////////////////////////////////////////////////////////

static QScriptValue fontMetrics(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(Painter, fontMetrics);
    return ctx->throwError("QPainter.prototype.fontMetrics is not implemented");
}

/////////////////////////////////////////////////////////////

static QScriptValue hasClipping(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Painter, hasClipping);
    return QScriptValue(eng, self->hasClipping());
}

/////////////////////////////////////////////////////////////

static QScriptValue initFrom(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Painter, initFrom);
    QWidget *widget = qscriptvalue_cast<QWidget*>(ctx->argument(0));
    if (!widget) {
        return ctx->throwError(QScriptContext::TypeError,
                               "QPainter.prototype.initFrom: argument is not a Widget");
    }
    self->initFrom(widget);
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue isActive(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Painter, isActive);
    return QScriptValue(eng, self->isActive());
}

/////////////////////////////////////////////////////////////

static QScriptValue layoutDirection(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Painter, layoutDirection);
    return QScriptValue(eng, static_cast<int>(self->layoutDirection()));
}

/////////////////////////////////////////////////////////////

static QScriptValue opacity(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Painter, opacity);
    return QScriptValue(eng, self->opacity());
}

/////////////////////////////////////////////////////////////

static QScriptValue paintEngine(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Painter, paintEngine);
    return eng->toScriptValue(self->paintEngine());
}

/////////////////////////////////////////////////////////////

static QScriptValue pen(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Painter, pen);
    return eng->toScriptValue(self->pen());
}

/////////////////////////////////////////////////////////////

static QScriptValue renderHints(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Painter, renderHints);
    return QScriptValue(eng, static_cast<int>(self->renderHints()));
}

/////////////////////////////////////////////////////////////

static QScriptValue resetMatrix(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Painter, resetMatrix);
    self->resetMatrix();
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue resetTransform(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Painter, resetTransform);
    self->resetTransform();
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue restore(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Painter, restore);
    self->restore();
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue rotate(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Painter, rotate);
    self->rotate(ctx->argument(0).toNumber());
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue save(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Painter, save);
    self->save();
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue scale(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Painter, scale);
    self->scale(ctx->argument(0).toNumber(),
                ctx->argument(1).toNumber());
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setBackground(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Painter, setBackground);
    self->setBackground(qscriptvalue_cast<QBrush>(ctx->argument(0)));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setBackgroundMode(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Painter, setBackgroundMode);
    self->setBackgroundMode(static_cast<Qt::BGMode>(ctx->argument(0).toInt32()));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setBrush(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Painter, setBrush);
    self->setBrush(qscriptvalue_cast<QBrush>(ctx->argument(0)));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setBrushOrigin(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Painter, setBrushOrigin);
    self->setBrushOrigin(qscriptvalue_cast<QPointF>(ctx->argument(0)));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setClipPath(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Painter, setClipPath);
    // ### ClipOperation
    self->setClipPath(qscriptvalue_cast<QPainterPath>(ctx->argument(0)));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setClipRect(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Painter, setClipRect);
    // ### ClipOperation
    if (ctx->argumentCount() >= 4) {
        // x, y, width, height [, operation]
        self->setClipRect(ctx->argument(0).toInt32(),
                          ctx->argument(1).toInt32(),
                          ctx->argument(2).toInt32(),
                          ctx->argument(3).toInt32());
    } else if (ctx->argumentCount() >= 1) {
        // rect [, operation]
        self->setClipRect(qscriptvalue_cast<QRectF>(ctx->argument(0)));
    }
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setClipRegion(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Painter, setClipRegion);
    // ### ClipOperation
    self->setClipRegion(qscriptvalue_cast<QRegion>(ctx->argument(0)));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setClipping(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Painter, setClipping);
    self->setClipping(ctx->argument(0).toBoolean());
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setCompositionMode(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Painter, setCompositionMode);
    self->setCompositionMode(static_cast<QPainter::CompositionMode>(ctx->argument(0).toInt32()));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setFont(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Painter, setFont);
    self->setFont(qscriptvalue_cast<QFont>(ctx->argument(0)));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setLayoutDirection(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Painter, setLayoutDirection);
    self->setLayoutDirection(static_cast<Qt::LayoutDirection>(ctx->argument(0).toInt32()));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setOpacity(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Painter, setOpacity);
    self->setOpacity(ctx->argument(0).toNumber());
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setPen(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Painter, setPen);
    self->setPen(qscriptvalue_cast<QPen>(ctx->argument(0)));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setRenderHint(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Painter, setRenderHint);
    self->setRenderHint(static_cast<QPainter::RenderHint>(ctx->argument(0).toInt32()),
                        ctx->argument(1).toBoolean());
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setRenderHints(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Painter, setRenderHints);
    self->setRenderHints(static_cast<QPainter::RenderHints>(ctx->argument(0).toInt32()),
                         ctx->argument(1).toBoolean());
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setTransform(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Painter, setTransform);
    self->setTransform(qscriptvalue_cast<QTransform>(ctx->argument(0)));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setViewTransformEnabled(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Painter, setViewTransformEnabled);
    self->setViewTransformEnabled(ctx->argument(0).toBoolean());
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setViewport(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Painter, setViewport);
    if (ctx->argumentCount() == 4) {
        // x, y, width, height
        self->setViewport(ctx->argument(0).toInt32(),
                          ctx->argument(1).toInt32(),
                          ctx->argument(2).toInt32(),
                          ctx->argument(3).toInt32());
    } else if (ctx->argumentCount() == 1) {
        // rect
        self->setViewport(qscriptvalue_cast<QRect>(ctx->argument(0)));
    }
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setWindow(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Painter, setWindow);
    if (ctx->argumentCount() == 4) {
        // x, y, width, height
        self->setWindow(ctx->argument(0).toInt32(),
                        ctx->argument(1).toInt32(),
                        ctx->argument(2).toInt32(),
                        ctx->argument(3).toInt32());
    } else if (ctx->argumentCount() == 1) {
        // rect
        self->setWindow(qscriptvalue_cast<QRect>(ctx->argument(0)));
    }
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setWorldMatrix(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Painter, setWorldMatrix);
    self->setWorldMatrix(qscriptvalue_cast<QMatrix>(ctx->argument(0)),
                         ctx->argument(1).toBoolean());
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setWorldMatrixEnabled(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Painter, setWorldMatrixEnabled);
    self->setWorldMatrixEnabled(ctx->argument(0).toBoolean());
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setWorldTransform(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Painter, setWorldTransform);
    self->setWorldTransform(qscriptvalue_cast<QTransform>(ctx->argument(0)),
                            ctx->argument(1).toBoolean());
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue shear(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Painter, shear);
    self->shear(ctx->argument(0).toNumber(),
                     ctx->argument(1).toNumber());
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue strokePath(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Painter, strokePath);
    QPainterPath *path = qscriptvalue_cast<QPainterPath*>(ctx->argument(0));
    if (!path) {
        return ctx->throwError(QScriptContext::TypeError,
                               "QPainter.prototype.strokePath: argument is not a PainterPath");
    }
    self->strokePath(*path, qscriptvalue_cast<QPen>(ctx->argument(1)));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue testRenderHint(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Painter, testRenderHint);
    return QScriptValue(eng, self->testRenderHint(static_cast<QPainter::RenderHint>(ctx->argument(0).toInt32())));
}

/////////////////////////////////////////////////////////////

static QScriptValue transform(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Painter, transform);
    return eng->toScriptValue(self->transform());
}

/////////////////////////////////////////////////////////////

static QScriptValue translate(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Painter, translate);
    if (ctx->argumentCount() == 2) {
        // dx, dy
        self->translate(ctx->argument(0).toNumber(),
                             ctx->argument(1).toNumber());
    } else if (ctx->argumentCount() == 1) {
        // offset
        self->translate(qscriptvalue_cast<QPointF>(ctx->argument(0)));
    }
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue viewTransformEnabled(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Painter, viewTransformEnabled);
    return QScriptValue(eng, self->viewTransformEnabled());
}

/////////////////////////////////////////////////////////////

static QScriptValue viewport(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Painter, viewport);
    return eng->toScriptValue(self->viewport());
}

/////////////////////////////////////////////////////////////

static QScriptValue window(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Painter, window);
    return eng->toScriptValue(self->window());
}

/////////////////////////////////////////////////////////////

static QScriptValue worldMatrix(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Painter, worldMatrix);
    return eng->toScriptValue(self->worldMatrix());
}

/////////////////////////////////////////////////////////////

static QScriptValue worldMatrixEnabled(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Painter, worldMatrixEnabled);
    return QScriptValue(eng, self->worldMatrixEnabled());
}

/////////////////////////////////////////////////////////////

static QScriptValue worldTransform(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Painter, worldTransform);
    return eng->toScriptValue(self->worldTransform());
}

/////////////////////////////////////////////////////////////

static QScriptValue toString(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(Painter, toString);
    return QScriptValue(eng, "QPainter");
}

/////////////////////////////////////////////////////////////

QScriptValue constructPainterClass(QScriptEngine *eng)
{
    QScriptValue proto = newPainter(eng, new QPainter());
    ADD_PROTO_FUNCTION(proto, background);
    ADD_PROTO_FUNCTION(proto, backgroundMode);
    ADD_PROTO_FUNCTION(proto, begin);
    ADD_PROTO_FUNCTION(proto, boundingRect);
    ADD_PROTO_FUNCTION(proto, brush);
    ADD_PROTO_FUNCTION(proto, brushOrigin);
    ADD_PROTO_FUNCTION(proto, clipPath);
    ADD_PROTO_FUNCTION(proto, clipRegion);
    ADD_PROTO_FUNCTION(proto, combinedMatrix);
    ADD_PROTO_FUNCTION(proto, combinedTransform);
    ADD_PROTO_FUNCTION(proto, compositionMode);
    ADD_PROTO_FUNCTION(proto, device);
    ADD_PROTO_FUNCTION(proto, deviceMatrix);
    ADD_PROTO_FUNCTION(proto, deviceTransform);
    ADD_PROTO_FUNCTION(proto, drawChord);
    ADD_PROTO_FUNCTION(proto, drawConvexPolygon);
    ADD_PROTO_FUNCTION(proto, drawArc);
    ADD_PROTO_FUNCTION(proto, drawEllipse);
    ADD_PROTO_FUNCTION(proto, drawImage);
    ADD_PROTO_FUNCTION(proto, drawLine);
    ADD_PROTO_FUNCTION(proto, drawLines);
    ADD_PROTO_FUNCTION(proto, drawPath);
    ADD_PROTO_FUNCTION(proto, drawPicture);
    ADD_PROTO_FUNCTION(proto, drawPie);
    ADD_PROTO_FUNCTION(proto, drawPixmap);
    ADD_PROTO_FUNCTION(proto, drawPoint);
    ADD_PROTO_FUNCTION(proto, drawPoints);
    ADD_PROTO_FUNCTION(proto, drawPolygon);
    ADD_PROTO_FUNCTION(proto, drawPolyline);
    ADD_PROTO_FUNCTION(proto, drawRect);
    ADD_PROTO_FUNCTION(proto, drawRects);
    ADD_PROTO_FUNCTION(proto, drawRoundRect);
    ADD_PROTO_FUNCTION(proto, drawText);
    ADD_PROTO_FUNCTION(proto, drawTiledPixmap);
    ADD_PROTO_FUNCTION(proto, end);
    ADD_PROTO_FUNCTION(proto, eraseRect);
    ADD_PROTO_FUNCTION(proto, fillPath);
    ADD_PROTO_FUNCTION(proto, fillRect);
    ADD_PROTO_FUNCTION(proto, font);
    ADD_PROTO_FUNCTION(proto, fontInfo);
    ADD_PROTO_FUNCTION(proto, fontMetrics);
    ADD_PROTO_FUNCTION(proto, hasClipping);
    ADD_PROTO_FUNCTION(proto, initFrom);
    ADD_PROTO_FUNCTION(proto, isActive);
    ADD_PROTO_FUNCTION(proto, layoutDirection);
    ADD_PROTO_FUNCTION(proto, opacity);
    ADD_PROTO_FUNCTION(proto, paintEngine);
    ADD_PROTO_FUNCTION(proto, pen);
    ADD_PROTO_FUNCTION(proto, renderHints);
    ADD_PROTO_FUNCTION(proto, resetMatrix);
    ADD_PROTO_FUNCTION(proto, resetTransform);
    ADD_PROTO_FUNCTION(proto, restore);
    ADD_PROTO_FUNCTION(proto, rotate);
    ADD_PROTO_FUNCTION(proto, save);
    ADD_PROTO_FUNCTION(proto, scale);
    ADD_PROTO_FUNCTION(proto, setBackground);
    ADD_PROTO_FUNCTION(proto, setBackgroundMode);
    ADD_PROTO_FUNCTION(proto, setBrush);
    ADD_PROTO_FUNCTION(proto, setBrushOrigin);
    ADD_PROTO_FUNCTION(proto, setClipPath);
    ADD_PROTO_FUNCTION(proto, setClipRect);
    ADD_PROTO_FUNCTION(proto, setClipRegion);
    ADD_PROTO_FUNCTION(proto, setClipping);
    ADD_PROTO_FUNCTION(proto, setCompositionMode);
    ADD_PROTO_FUNCTION(proto, setFont);
    ADD_PROTO_FUNCTION(proto, setLayoutDirection);
    ADD_PROTO_FUNCTION(proto, setOpacity);
    ADD_PROTO_FUNCTION(proto, setPen);
    ADD_PROTO_FUNCTION(proto, setRenderHint);
    ADD_PROTO_FUNCTION(proto, setRenderHints);
    ADD_PROTO_FUNCTION(proto, setTransform);
    ADD_PROTO_FUNCTION(proto, setViewTransformEnabled);
    ADD_PROTO_FUNCTION(proto, setViewport);
    ADD_PROTO_FUNCTION(proto, setWindow);
    ADD_PROTO_FUNCTION(proto, setWorldMatrix);
    ADD_PROTO_FUNCTION(proto, setWorldMatrixEnabled);
    ADD_PROTO_FUNCTION(proto, setWorldTransform);
    ADD_PROTO_FUNCTION(proto, shear);
    ADD_PROTO_FUNCTION(proto, strokePath);
    ADD_PROTO_FUNCTION(proto, testRenderHint);
    ADD_PROTO_FUNCTION(proto, toString);
    ADD_PROTO_FUNCTION(proto, transform);
    ADD_PROTO_FUNCTION(proto, translate);
    ADD_PROTO_FUNCTION(proto, viewTransformEnabled);
    ADD_PROTO_FUNCTION(proto, viewport);
    ADD_PROTO_FUNCTION(proto, window);
    ADD_PROTO_FUNCTION(proto, worldMatrix);
    ADD_PROTO_FUNCTION(proto, worldMatrixEnabled);
    ADD_PROTO_FUNCTION(proto, worldTransform);

    QScript::registerPointerMetaType<QPainter>(eng, proto);

    qScriptRegisterSequenceMetaType<QVector<QRectF> >(eng);

    return eng->newFunction(ctor, proto);
}
