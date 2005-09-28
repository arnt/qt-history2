/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qsvgstyle_p.h"

#include "qpainter.h"
#include "qpair.h"
#include "qcolor.h"
#include "qdebug.h"


QSvgStyleProperty::~QSvgStyleProperty()
{
}

QSvgQualityStyle::QSvgQualityStyle(int color)
    : m_colorRendering(color)
{

}
void QSvgQualityStyle::apply(QPainter *, const QRectF &, QSvgNode *)
{

}
void QSvgQualityStyle::revert(QPainter *)
{

}

QSvgFillStyle::QSvgFillStyle(const QBrush &brush)
    : m_fill(brush)
{
}

void QSvgFillStyle::apply(QPainter *p, const QRectF &, QSvgNode *)
{
    m_oldFill = p->brush();
    p->setBrush(m_fill);
}

void QSvgFillStyle::revert(QPainter *p)
{
    p->setBrush(m_oldFill);
}

QSvgViewportFillStyle::QSvgViewportFillStyle(const QBrush &brush)
    : m_viewportFill(brush)
{
}

void QSvgViewportFillStyle::apply(QPainter *p, const QRectF &, QSvgNode *)
{
    m_oldFill = p->brush();
    p->setBrush(m_viewportFill);
}

void QSvgViewportFillStyle::revert(QPainter *p)
{
    p->setBrush(m_oldFill);
}

QSvgFontStyle::QSvgFontStyle(const QFont &font)
    : m_font(font)
{
}

void QSvgFontStyle::apply(QPainter *p, const QRectF &, QSvgNode *)
{
    m_oldFont = p->font();
    p->setFont(m_font);
}

void QSvgFontStyle::revert(QPainter *p)
{
    p->setFont(m_oldFont);
}

QSvgStrokeStyle::QSvgStrokeStyle(const QPen &pen)
    : m_stroke(pen)
{
}

void QSvgStrokeStyle::apply(QPainter *p, const QRectF &, QSvgNode *)
{
    m_oldStroke = p->pen();
    p->setPen(m_stroke);
}

void QSvgStrokeStyle::revert(QPainter *p)
{
    p->setPen(m_oldStroke);
}

QSvgSolidColorStyle::QSvgSolidColorStyle(const QColor &color)
    : m_solidColor(color)
{
}

void QSvgSolidColorStyle::apply(QPainter *p, const QRectF &, QSvgNode *)
{
    m_oldFill = p->brush();
    m_oldStroke = p->pen();
    QBrush b = m_oldFill;
    b.setColor(m_solidColor);
    p->setBrush(b);
    QPen pen = m_oldStroke;
    pen.setColor(m_solidColor);
    p->setPen(pen);
}

void QSvgSolidColorStyle::revert(QPainter *p)
{
    p->setBrush(m_oldFill);
    p->setPen(m_oldStroke);
}

QSvgGradientStyle::QSvgGradientStyle(QGradient *grad, bool resolveBounds)
    : m_gradient(grad), m_resolveBounds(resolveBounds)
{
}

void QSvgGradientStyle::apply(QPainter *p, const QRectF &rect, QSvgNode *)
{
    m_oldFill = p->brush();

    //resolving stop colors
    if (!m_resolvePoints.isEmpty()) {
        QColor color = p->brush().color();
        if (!color.isValid())
            color = p->pen().color();
        QList<qreal>::const_iterator itr = m_resolvePoints.begin();
        for (; itr != m_resolvePoints.end(); ++itr) {
            qDebug()<<"resolving "<<(*itr)<<" to "<<color;
            m_gradient->setColorAt(*itr, color);
        }
    }

    //we need to resolve boundries on linear gradients
    if (m_resolveBounds &&
        m_gradient->type() == QGradient::LinearGradient) {
        qDebug()<< (((QLinearGradient*)m_gradient)->start())
                << (((QLinearGradient*)m_gradient)->finalStop())
                << rect;
        QLinearGradient gradient(rect.x(), rect.y(),
                                 rect.width(), rect.height());
        gradient.setStops(m_gradient->stops());
        qDebug()<<m_gradient->stops().count()
                << (m_gradient->stops()[0].second) << (m_gradient->stops()[1].second);
        QBrush b(gradient);
        p->setBrush(b);
    } else {
        QBrush b(*m_gradient);
        p->setBrush(b);
    }
}

void QSvgGradientStyle::revert(QPainter *p)
{
    p->setBrush(m_oldFill);
}

void QSvgGradientStyle::addResolve(qreal offset)
{
    m_resolvePoints.append(offset);
}

QSvgTransformStyle::QSvgTransformStyle(const QMatrix &trans)
    : m_transform(trans)
{
}

void QSvgTransformStyle::apply(QPainter *p, const QRectF &, QSvgNode *)
{
    m_oldWorldMatrix = p->matrix();
    p->setMatrix(m_transform, true);
}

void QSvgTransformStyle::revert(QPainter *p)
{
    p->setMatrix(m_oldWorldMatrix, false);//don't combine
}

QSvgStyleProperty::Type QSvgQualityStyle::type() const
{
    return QUALITY;
}

QSvgStyleProperty::Type QSvgFillStyle::type() const
{
    return FILL;
}

QSvgStyleProperty::Type QSvgViewportFillStyle::type() const
{
    return VIEWPORT_FILL;
}

QSvgStyleProperty::Type QSvgFontStyle::type() const
{
    return FONT;
}

QSvgStyleProperty::Type QSvgStrokeStyle::type() const
{
    return STROKE;
}

QSvgStyleProperty::Type QSvgSolidColorStyle::type() const
{
    return SOLID_COLOR;
}

QSvgStyleProperty::Type QSvgGradientStyle::type() const
{
    return GRADIENT;
}

QSvgStyleProperty::Type QSvgTransformStyle::type() const
{
    return TRANSFORM;
}

void QSvgStyle::apply(QPainter *p, const QRectF &rect, QSvgNode *node)
{
    if (quality) {
        quality->apply(p, rect, node);
    }

    if (fill) {
        fill->apply(p, rect, node);
    }

    if (viewportFill) {
        viewportFill->apply(p, rect, node);
    }

    if (font) {
        font->apply(p, rect, node);
    }

    if (stroke) {
        stroke->apply(p, rect, node);
    }

    if (solidColor) {
        solidColor->apply(p, rect, node);
    }

    if (gradient) {
        gradient->apply(p, rect, node);
    }

    if (transform) {
        transform->apply(p, rect, node);
    }
}

void QSvgStyle::revert(QPainter *p)
{
    if (quality) {
        quality->revert(p);
    }

    if (fill) {
        fill->revert(p);
    }

    if (viewportFill) {
        viewportFill->revert(p);
    }

    if (font) {
        font->revert(p);
    }

    if (stroke) {
        stroke->revert(p);
    }

    if (solidColor) {
        solidColor->revert(p);
    }

    if (gradient) {
        gradient->revert(p);
    }

    if (transform) {
        transform->revert(p);
    }
}
