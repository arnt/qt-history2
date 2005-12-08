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
#include "qsvgfont_p.h"
#include "qsvgnode_p.h"
#include "qsvgtinydocument_p.h"

#include "qpainter.h"
#include "qpair.h"
#include "qcolor.h"
#include "qdebug.h"

#include <math.h>

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

QSvgFillStyle::QSvgFillStyle(const QBrush &brush, bool fromColor)
    : m_fill(brush), m_fromColor(fromColor)
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

QSvgFontStyle::QSvgFontStyle(QSvgFont *font, QSvgTinyDocument *doc)
    : m_font(font), m_pointSize(24), m_doc(doc)
{
}

QSvgFontStyle::QSvgFontStyle(const QFont &font)
    : m_font(0), m_pointSize(24), m_qfont(font)
{
}


void QSvgFontStyle::setPointSize(qreal size)
{
    m_pointSize = size;
}

qreal QSvgFontStyle::pointSize() const
{
    return m_pointSize;
}

void QSvgFontStyle::apply(QPainter *p, const QRectF &, QSvgNode *)
{
    if (!m_font) {
        m_oldFont = p->font();
        p->setFont(m_qfont);
    }
}

void QSvgFontStyle::revert(QPainter *p)
{
    if (!m_font) {
        p->setFont(m_oldFont);
    }
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
            //qDebug()<<"resolving "<<(*itr)<<" to "<<color;
            m_gradient->setColorAt(*itr, color);
        }
    }

    //we need to resolve boundries
    //the code is funky i'll have to verify it
    //(testcases right now are bugs/resolve_radial.svg
    // and bugs/resolve_linear.svg)
    if (m_resolveBounds) {
        if (m_gradient->type() == QGradient::LinearGradient) {
            QLinearGradient *grad = (QLinearGradient*)(m_gradient);
            qreal xs, ys, xf, yf;
            xs = rect.x();
            ys = rect.y();
            xf = (rect.x()+rect.width())  * grad->finalStop().x();
            yf = (rect.y()+rect.height()) * grad->finalStop().y();
            QLinearGradient gradient(xs, ys,
                                     xf, yf);
            gradient.setStops(m_gradient->stops());
            QBrush b(gradient);
            p->setBrush(b);
        } else {
            QRadialGradient *grad = (QRadialGradient*)m_gradient;
            qreal cx, cy, r, fx, fy;
            cx = rect.width() * grad->center().x();
            cy = rect.height() * grad->center().y();
            //### the radius is wrong. it has to be transformed
            // so that the horizontal on is rect.width() * grad->radius();
            // and vertical rect.height() * grad->radius(). it's a simple
            // transformation but we don't support exclusive fill
            // transformations at the moment
            r  = rect.width() * grad->radius();
            fx = rect.width() * grad->focalPoint().x();
            fy = rect.width() * grad->focalPoint().y();
            QRadialGradient gradient(cx, cy,
                                     r, fx, fy);
            gradient.setStops(m_gradient->stops());
            QBrush b(gradient);
            p->setBrush(b);
        }
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

    if (animateColor) {
        animateColor->apply(p, rect, node);
    }

    //animated transforms have to be applied
    //_after_ the original object transformations
    if (!animateTransforms.isEmpty()) {
        QList<QSvgAnimateTransform*>::const_iterator itr;
        for (itr = animateTransforms.begin(); itr != animateTransforms.end();
             ++itr) {
            (*itr)->apply(p, rect, node);
        }
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

    //animated transforms need to be reverted _before_
    //the native transforms
    if (!animateTransforms.isEmpty()) {
        QList<QSvgAnimateTransform*>::const_iterator itr;
        itr = animateTransforms.begin();
        //only need to rever the first one because that
        //one has the original world matrix for the primitve
        if (itr != animateTransforms.end()) {
            (*itr)->revert(p);
        }
    }

    if (transform) {
        transform->revert(p);
    }

    if (animateColor) {
        animateColor->revert(p);
    }
}

QSvgAnimateTransform::QSvgAnimateTransform(int startMs, int endMs, int byMs )
    : QSvgStyleProperty(),
      m_from(startMs), m_to(endMs), m_by(byMs),
      m_type(Empty), m_count(0), m_finished(false)
{
    m_totalRunningTime = m_to - m_from;
}

void QSvgAnimateTransform::setArgs(TransformType type, const QList<qreal> &args)
{
    m_type = type;
    m_args = args;
    Q_ASSERT(!(args.count()%3));
    m_count = args.count() / 3;
}

void QSvgAnimateTransform::apply(QPainter *p, const QRectF &, QSvgNode *node)
{
    m_oldWorldMatrix = p->matrix();
    resolveMatrix(node);
    if (!m_finished || m_freeze)
        p->setMatrix(m_transform, true);
}

void QSvgAnimateTransform::revert(QPainter *p)
{
    if (!m_finished || m_freeze) {
        p->setMatrix(m_oldWorldMatrix, false);//don't combine
    }
}

void QSvgAnimateTransform::resolveMatrix(QSvgNode *node)
{
    static const qreal deg2rad = qreal(0.017453292519943295769);
    qreal elapsed = node->document()->currentElapsed();
    qreal percent = (elapsed - m_from) / m_to;
    if (elapsed < m_from || m_finished)
        return;

    if (percent > 1) {
        percent -= ((int)percent);
    }

    qreal currentPosition = percent * (m_count-1); //array offset
    int startElem = static_cast<int>(floor(currentPosition));
    int endElem   = static_cast<int>(ceil(currentPosition));

    switch(m_type)
    {
    case Translate: {
        startElem *= 3;
        endElem   *= 3;
        qreal from1, from2, from3;
        qreal to1, to2, to3;
        from1 = m_args[startElem++];
        from2 = m_args[startElem++];
        from3 = m_args[startElem++];
        to1   = m_args[endElem++];
        to2   = m_args[endElem++];
        to3   = m_args[endElem++];

        qreal transXDiff = (to1-from1) * percent;
        qreal transX = from1 + transXDiff;
        qreal transYDiff = (to2-from2) * percent;
        qreal transY = from2 + transYDiff;
        m_transform = QMatrix();
        m_transform.translate(transX, transY);
        break;
    }
    case Scale: {
        startElem *= 3;
        endElem   *= 3;
        qreal from1, from2, from3;
        qreal to1, to2, to3;
        from1 = m_args[startElem++];
        from2 = m_args[startElem++];
        from3 = m_args[startElem++];
        to1   = m_args[endElem++];
        to2   = m_args[endElem++];
        to3   = m_args[endElem++];

        qreal transXDiff = (to1-from1) * percent;
        qreal transX = from1 + transXDiff;
        qreal transYDiff = (to2-from2) * percent;
        qreal transY = from2 + transYDiff;
        if (transY == 0)
            transY = transX;
        m_transform = QMatrix();
        m_transform.scale(transX, transY);
        break;
    }
    case Rotate: {
        startElem *= 3;
        endElem   *= 3;
        qreal from1, from2, from3;
        qreal to1, to2, to3;
        from1 = m_args[startElem++];
        from2 = m_args[startElem++];
        from3 = m_args[startElem++];
        to1   = m_args[endElem++];
        to2   = m_args[endElem++];
        to3   = m_args[endElem++];

        qreal rotationDiff = (to1-from1) * percent;
        //qreal rotation = from1 + rotationDiff;

        qreal transXDiff = (to2-from2) * percent;
        qreal transX = from2 + transXDiff;
        qreal transYDiff = (to3-from3) * percent;
        qreal transY = from3 + transYDiff;
        m_transform = QMatrix();
        m_transform.translate(transX, transY);
        m_transform.rotate(rotationDiff);
        m_transform.translate(-transX, -transY);
        break;
    }
    case SkewX: {
        startElem *= 3;
        endElem   *= 3;
        qreal from1, from2, from3;
        qreal to1, to2, to3;
        from1 = m_args[startElem++];
        from2 = m_args[startElem++];
        from3 = m_args[startElem++];
        to1   = m_args[endElem++];
        to2   = m_args[endElem++];
        to3   = m_args[endElem++];

        qreal transXDiff = (to1-from1) * percent;
        qreal transX = from1 + transXDiff;
        m_transform = QMatrix();
        m_transform.shear(tan(transX*deg2rad), 0);
        break;
    }
    case SkewY: {
        startElem *= 3;
        endElem   *= 3;
        qreal from1, from2, from3;
        qreal to1, to2, to3;
        from1 = m_args[startElem++];
        from2 = m_args[startElem++];
        from3 = m_args[startElem++];
        to1   = m_args[endElem++];
        to2   = m_args[endElem++];
        to3   = m_args[endElem++];


        qreal transYDiff = (to1-from1) * percent;
        qreal transY = from1 + transYDiff;
        m_transform = QMatrix();
        m_transform.shear(0, tan(transY*deg2rad));
        break;
    }
    default:
        break;
    }

    if (m_repeatCount < 0)
        return;

    if (elapsed > m_to) {
        if (m_repeatCount > 1) {
            --m_repeatCount;
        } else if (m_repeatCount > 0 && m_repeatCount < 1) {
            if (m_repeatCount <= percent) {
                m_finished = true;
            }
        }
    } else if (m_repeatCount > 0 && m_repeatCount < 1) {
        //this happens if m_repeatCount < 1 from the start
        if (m_repeatCount <= percent) {
            m_finished = true;
        }
    }
}

QSvgStyleProperty::Type QSvgAnimateTransform::type() const
{
    return ANIMATE_TRANSFORM;
}

void QSvgAnimateTransform::setFreeze(bool freeze)
{
    m_freeze = freeze;
}

void QSvgAnimateTransform::setRepeatCount(qreal repeatCount)
{
    m_repeatCount = repeatCount;
}

QSvgAnimateColor::QSvgAnimateColor(int startMs, int endMs, int byMs)
    : QSvgStyleProperty(),
      m_from(startMs), m_to(endMs), m_by(byMs),
      m_finished(false)
{
    m_totalRunningTime = m_to - m_from;
}

void QSvgAnimateColor::setArgs(bool fill,
                               const QList<QColor> &colors)
{
    m_fill = fill;
    m_colors = colors;
}

void QSvgAnimateColor::setFreeze(bool freeze)
{
    m_freeze = freeze;
}

void QSvgAnimateColor::setRepeatCount(qreal repeatCount)
{
    m_repeatCount = repeatCount;
}

void QSvgAnimateColor::apply(QPainter *p, const QRectF &, QSvgNode *node)
{
    qreal elapsed = node->document()->currentElapsed();
    qreal percent = (elapsed - m_from) / m_to;

    if (elapsed < m_from || m_finished)
        return;
    if (percent > 1) {
        percent -= ((int)percent);
    }

    qreal currentPosition = percent * (m_colors.count()-1); //array offset

    percent *= (m_colors.count() - 1);
    if (percent > 1) {
        percent -= ((int)percent);
    }

    int startElem = static_cast<int>(floor(currentPosition));
    int endElem   = static_cast<int>(ceil(currentPosition));
    QColor start = m_colors[startElem];
    QColor end = m_colors[endElem];
    qreal aDiff = (end.alpha() - start.alpha()) * percent;
    qreal rDiff = (end.red()   - start.red()) * percent;
    qreal gDiff = (end.green() - start.green()) * percent;
    qreal bDiff = (end.blue()  - start.blue()) * percent;


    int alpha  = int(start.alpha() + aDiff);
    int red    = int(start.red() + rDiff);
    int green  = int(start.green() + gDiff);
    int blue   = int(start.blue() + bDiff);

    QColor color(red, green, blue, alpha);

    if (m_fill) {
        QBrush b = p->brush();
        m_oldBrush = b;
        b.setColor(color);
        p->setBrush(b);
    } else {
        QPen pen = p->pen();
        m_oldPen = pen;
        pen.setColor(color);
        p->setPen(pen);
    }

    if (m_repeatCount < 0)
        return;

    if (elapsed > m_to) {
        if (m_repeatCount > 1) {
            --m_repeatCount;
        } else if (m_repeatCount > 0 && m_repeatCount < 1) {
            if (m_repeatCount <= percent) {
                m_finished = true;
            }
        }
    } else if (m_repeatCount > 0 && m_repeatCount < 1) {
        //this happens if m_repeatCount < 1 from the start
        if (m_repeatCount <= percent) {
            m_finished = true;
        }
    }
}

void QSvgAnimateColor::revert(QPainter *p)
{
    if (m_fill) {
        p->setBrush(m_oldBrush);
    } else {
        p->setPen(m_oldPen);
    }
}

QSvgStyleProperty::Type QSvgAnimateColor::type() const
{
    return ANIMATE_COLOR;
}
