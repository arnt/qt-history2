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

#include "qsvggraphics_p.h"

#include "qpainter.h"
#include "qtextdocument.h"
#include "qabstracttextdocumentlayout.h"
#include "qtextcursor.h"
#include "qdebug.h"

#include <math.h>
#include <limits.h>

void QSvgAnimation::draw(QPainter *)
{
    qWarning()<<"<animation> no implemented";
}


QSvgCircle::QSvgCircle(QSvgNode *parent, const QRectF &rect)
    : QSvgNode(parent), m_bounds(rect)
{
}


QRectF QSvgCircle::bounds() const
{
    return m_bounds;
}

void QSvgCircle::draw(QPainter *p)
{
    applyStyle(p);
    p->drawEllipse(m_bounds);
    revertStyle(p);
}

QSvgArc::QSvgArc(QSvgNode *parent, const QPainterPath &path)
    : QSvgNode(parent), cubic(path)
{
}

void QSvgArc::draw(QPainter *p)
{
    applyStyle(p);
    p->drawPath(cubic);
    revertStyle(p);
}

QSvgEllipse::QSvgEllipse(QSvgNode *parent, const QRectF &rect)
    : QSvgNode(parent), m_bounds(rect)
{
}

QRectF QSvgEllipse::bounds() const
{
    return m_bounds;
}

void QSvgEllipse::draw(QPainter *p)
{
    applyStyle(p);
    p->drawEllipse(m_bounds);
    revertStyle(p);
}

QSvgImage::QSvgImage(QSvgNode *parent, const QImage &image,
                     const QRect &bounds)
    : QSvgNode(parent), m_image(image),
      m_bounds(bounds)
{
    if (m_bounds.width() == 0)
        m_bounds.setWidth(m_image.width());
    if (m_bounds.height() == 0)
        m_bounds.setHeight(m_image.height());
}

void QSvgImage::draw(QPainter *p)
{
    applyStyle(p);
    p->drawImage(m_bounds, m_image);
    revertStyle(p);
}


QSvgLine::QSvgLine(QSvgNode *parent, const QLineF &line)
    : QSvgNode(parent), m_bounds(line)
{
}


void QSvgLine::draw(QPainter *p)
{
    applyStyle(p);
    p->drawLine(m_bounds);
    revertStyle(p);
}

QSvgPath::QSvgPath(QSvgNode *parent, const QPainterPath &qpath)
    : QSvgNode(parent), m_path(qpath)
{
}

void QSvgPath::draw(QPainter *p)
{
    applyStyle(p);
    p->drawPath(m_path);
    revertStyle(p);
}

QSvgPolygon::QSvgPolygon(QSvgNode *parent, const QPolygonF &poly)
    : QSvgNode(parent), m_poly(poly)
{

}

QRectF QSvgPolygon::bounds() const
{
    return m_poly.boundingRect();
}

void QSvgPolygon::draw(QPainter *p)
{
    applyStyle(p);
    p->drawPolygon(m_poly);
    revertStyle(p);
}


QSvgPolyline::QSvgPolyline(QSvgNode *parent, const QPolygonF &poly)
    : QSvgNode(parent), m_poly(poly)
{

}

void QSvgPolyline::draw(QPainter *p)
{
    applyStyle(p);
    if (p->brush().style() != Qt::NoBrush) {
        QPen save = p->pen();
        p->setPen(QPen(Qt::NoPen));
        p->drawPolygon(m_poly);
        p->setPen(save);
    }
    p->drawPolyline(m_poly);
    revertStyle(p);
}

QSvgRect::QSvgRect(QSvgNode *node, const QRectF &rect, int rx, int ry)
    : QSvgNode(node),
      m_rect(rect), m_rx(rx), m_ry(ry)
{
}

QRectF QSvgRect::bounds() const
{
    return m_rect;
}

void QSvgRect::draw(QPainter *p)
{
    applyStyle(p);
    if (m_rx || m_ry)
        p->drawRoundRect(m_rect, m_rx, m_ry);
    else
        p->drawRect(m_rect);
    revertStyle(p);
}

QSvgText::QSvgText(QSvgNode *parent, const QPointF &coord)
    : QSvgNode(parent), m_coord(coord),
      m_textAlignment(Qt::AlignLeft)
{
}

QSvgText::~QSvgText()
{
}

//QRectF QSvgText::bounds() const {}

void QSvgText::draw(QPainter *p)
{
    applyStyle(p);
    QTextLayout tl(m_text);
    QTextOption op = tl.textOption();
    op.setAlignment(m_textAlignment);
    //tl.setTextOption(op);
    tl.setAdditionalFormats(m_formatRanges);
    tl.beginLayout();
    qreal y = 0;
    bool initial = true;
    qreal px = m_coord.x();
    qreal py = m_coord.y();

    forever {
        QTextLine line = tl.createLine();
        if (!line.isValid())
            break;
    }
    for (int i = 0; i < tl.lineCount(); ++i) {
        QTextLine line = tl.lineAt(i);

        line.setPosition(QPointF(0, y-line.ascent()));
        y += line.height();

        if (initial) {
            qreal w = line.naturalTextWidth();
            px = m_coord.x();
            py = m_coord.y();
            if (m_textAlignment == Qt::AlignHCenter) {
                px = m_coord.x() - w / 2;
            }
            else if (m_textAlignment == Qt::AlignRight)
                px = m_coord.x() - w;
        }
    }
    tl.endLayout();
    tl.draw(p, QPointF(px, py));

    revertStyle(p);
}

void QSvgText::insertText(const QString &text)
{
    QTextLayout::FormatRange range;
    range.start = m_text.length();
    range.length = text.length();
    range.format = m_formats.top();

    m_formatRanges.append(range);
    m_text += text;
}

void QSvgText::insertFormat(const QTextCharFormat &format)
{
    QTextCharFormat mergedFormat = format;
    if (!m_formats.isEmpty()) {
        mergedFormat = m_formats.top();
        mergedFormat.merge(format);
    }

    m_formats.push(mergedFormat);
}

void QSvgText::popFormat()
{
    if (m_formats.count() > 1)
        m_formats.pop();
}

const QTextCharFormat &QSvgText::topFormat() const
{
    return m_formats.top();
}

void QSvgText::setTextAlignment(const Qt::Alignment &alignment)
{
    m_textAlignment = alignment;
}

void QSvgTextArea::draw(QPainter *p)
{
    applyStyle(p);

    revertStyle(p);
}

QSvgUse::QSvgUse(QSvgNode *parent, QSvgNode *node)
    : QSvgNode(parent), m_link(node)
{

}

void QSvgUse::draw(QPainter *p)
{
    applyStyle(p);
    m_link->draw(p);
    revertStyle(p);
}

void QSvgVideo::draw(QPainter *p)
{
    applyStyle(p);

    revertStyle(p);
}

QSvgNode::Type QSvgAnimation::type() const
{
    return ANIMATION;
}

QSvgNode::Type QSvgArc::type() const
{
    return ARC;
}

QSvgNode::Type QSvgCircle::type() const
{
    return CIRCLE;
}

QSvgNode::Type QSvgEllipse::type() const
{
    return ELLIPSE;
}

QSvgNode::Type QSvgImage::type() const
{
    return IMAGE;
}

QSvgNode::Type QSvgLine::type() const
{
    return LINE;
}

QSvgNode::Type QSvgPath::type() const
{
    return PATH;
}

QSvgNode::Type QSvgPolygon::type() const
{
    return POLYGON;
}

QSvgNode::Type QSvgPolyline::type() const
{
    return POLYLINE;
}

QSvgNode::Type QSvgRect::type() const
{
    return RECT;
}

QSvgNode::Type QSvgText::type() const
{
    return TEXT;
}

QSvgNode::Type QSvgTextArea::type() const
{
    return TEXTAREA;
}

QSvgNode::Type QSvgUse::type() const
{
    return USE;
}

QSvgNode::Type QSvgVideo::type() const
{
    return VIDEO;
}
