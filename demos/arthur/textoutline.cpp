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

#include "textoutline.h"

#include <qpainter.h>
#include <qpainterpath.h>
#include <qapplication.h>
#include <qline.h>
#include <qevent.h>

TextOutline::TextOutline(QWidget *parent)
    : DemoWidget(parent)
{
    QFont f("Times New Roman,Serif,vera_sans");
    f.setStyleHint(QFont::Serif);
    f.setStyleStrategy(QFont::ForceOutline);
    f.setPointSize(100);

    QString text("Trolltech");
    basePaths.resize(text.size());
    xpaths.resize(text.size());

    int x = 0;
    for (int i = 0; i < text.size(); ++i) {
        QChar c = text.at(i);
        basePaths[i].addText(x, 100, f, QString(c));
        x += QFontMetrics(f).width(c);
        basePathBounds |= basePaths.at(i).boundingRect();
    }

    // Increase the bounds a bit to avoid close to zero calculation problems...
    basePathBounds.adjust(-5, -5, 5, 5);
    pul = pur = pbl = pbr = QPoint(-1, -1);
    dragLocation = TopLeft;
}

void TextOutline::drawTarget(QPainter *p, const QPoint &pt)
{
    p->save();
    p->setBrush(QColor(220, 255, 220, attributes->alpha ? 127 : 255));
    p->setPen(Qt::NoPen);
    p->translate(pt.x() - 10, pt.y() - 10);
    p->drawEllipse(0, 0, 20, 20);
    p->setBrush(Qt::NoBrush);
    p->setPen(Qt::black);
    p->setClipRegion(QRegion(QRect(0, 0, 20, 20)) - QRect(2, 2, 16, 16));
    p->drawEllipse(0, 0, 20, 20);
    p->setClipping(false);
    p->drawLine(-2, 10, 2, 10);
    p->drawLine(10, -2, 10, 2);
    p->drawLine(18, 10, 22, 10);
    p->drawLine(10, 18, 10, 22);
    p->drawEllipse(8, 8, 5, 5);
    p->restore();
}

void TextOutline::paintEvent(QPaintEvent *)
{
    QPainter p(this);

    drawBackground(&p);

    for (int i = 0; i < xpaths.size(); ++i) {
        QPainterPath &xpath = xpaths[i];
        p.fillPath(xpath, QColor(159, 124, 240, attributes->alpha ? 127 : 255));
        if (attributes->antialias)
            p.setRenderHint(QPainter::Antialiasing);
        p.strokePath(xpath, QPen(QColor(0, 0, 0, attributes->alpha ? 191 : 255), 2));
    }

    drawTarget(&p, pul);
    drawTarget(&p, pbl);
    drawTarget(&p, pur);
    drawTarget(&p, pbr);
}

void TextOutline::startAnimation()
{
    resetState();
}

void TextOutline::stopAnimation()
{

}


struct SortHelper
{
    SortHelper(TextOutline::DragLocation loc, qreal dist) : drag(loc), distance(dist) {}
    SortHelper() { };

    TextOutline::DragLocation drag;
    qreal distance;

    bool operator<(const SortHelper &other) const {
        return distance < other.distance;
    }
};

void TextOutline::mousePressEvent(QMouseEvent *e)
{
    QList<SortHelper> l;
    l << SortHelper(TopLeft, QLineF(e->pos(), pul).length());
    l << SortHelper(TopRight, QLineF(e->pos(), pur).length());
    l << SortHelper(BottomLeft, QLineF(e->pos(), pbl).length());
    l << SortHelper(BottomRight, QLineF(e->pos(), pbr).length());

    qSort(l);
    // First element is now the closest to the mouse press.
    dragLocation = l.at(0).drag;

    // Force update first one...
    mouseMoveEvent(e);
}

void TextOutline::mouseMoveEvent(QMouseEvent *e)
{
    QPoint p = e->pos();

    switch (dragLocation) {
    case TopLeft:
        pul = p;
        break;
    case TopRight:
        pur = p;
        break;
    case BottomLeft:
        pbl = p;
        break;
    case BottomRight:
        pbr = p;
        break;
    }

    updatePath();
    update();
}

void TextOutline::showEvent(QShowEvent *)
{
    if (pul == QPoint(-1, -1)) {
        int w = width();
        int h = height();
        int w2 = w/2, w4 = w/4, h2 = h/2, h4 = h/4;

        pul = QPoint(w4, h4);
        pur = QPoint(w2+w4, h4);
        pbl = QPoint(w4, h2+h4);
        pbr = QPoint(w2+w4, h2+h4);
        updatePath();
    }
}

QPointF TextOutline::mapPoint(qreal x, qreal y, bool *mappingOk)
{
    qreal dx = (x-basePathBounds.x()) / basePathBounds.width();

    QLineF topLine(pul, pur);
    QLineF bottomLine(pbl, pbr);
    QLineF vertLine(topLine.pointAt(dx), bottomLine.pointAt(dx));

    qreal dy = (y-basePathBounds.y()) / basePathBounds.height();
    QLineF leftLine(pul, pbl);
    QLineF rightLine(pur, pbr);
    QLineF horLine(leftLine.pointAt(dy), rightLine.pointAt(dy));

    QPointF isect;
    QLineF::IntersectType t = horLine.intersect(vertLine, &isect);
    *mappingOk = t != QLineF::NoIntersection;
    return isect;
}

void TextOutline::updatePath()
{
    for (int p = 0; p < basePaths.size(); ++p) {
        QPainterPath &basePath = basePaths[p];
        bool mappingOk = true;
        QPainterPath newPath;
        newPath.setFillRule(Qt::WindingFill);
        for (int i=0; i<basePath.elementCount() && mappingOk; ++i) {
            const QPainterPath::Element &elm = basePath.elementAt(i);
            switch (elm.type) {
            case QPainterPath::MoveToElement:
                newPath.moveTo(mapPoint(elm.x, elm.y, &mappingOk));
                break;
            case QPainterPath::LineToElement:
                newPath.lineTo(mapPoint(elm.x, elm.y, &mappingOk));
                break;
            case QPainterPath::CurveToElement:
                newPath.cubicTo(mapPoint(elm.x, elm.y, &mappingOk),
                                mapPoint(basePath.elementAt(i+1).x, basePath.elementAt(i+1).y, &mappingOk),
                                mapPoint(basePath.elementAt(i+2).x, basePath.elementAt(i+2).y, &mappingOk));
                // Skip the two CurveToDataElement's
                i += 2;
                break;
            default:
                qFatal("aaahahhhhh!!!!");
                break;
            }
        }
        if (mappingOk)
            xpaths[p] = newPath;
    }
}
