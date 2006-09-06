/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "chip.h"

#include <QtGui>

Chip::Chip(const QColor &color, int x, int y)
{
    this->x = x;
    this->y = y;
    this->color = color;
    setZValue((x + y) % 2);

    setFlags(ItemIsSelectable | ItemIsMovable);
    setAcceptsHoverEvents(true);
}

QRectF Chip::boundingRect() const
{
    return QRectF(0, 0, 110, 70);
}

void Chip::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget);

    QColor fillColor = color;
    if (option->state & QStyle::State_MouseOver)
        fillColor = fillColor.light(125);
    
    if (option->levelOfDetail < 0.2) {
        if (option->levelOfDetail < 0.125) {
            painter->fillRect(QRectF(0, 0, 110, 70), fillColor);
            return;
        }

        painter->setPen(QPen(Qt::black, 0));
        painter->setBrush(fillColor);
        painter->drawRect(13, 13, 97, 57);
        return;
    }

    QPen oldPen = painter->pen();
    QPen pen = oldPen;
    int width = 0;
    if (option->state & QStyle::State_Selected)
        width += 2;

    pen.setWidth(width);
    painter->setPen(pen);
    painter->setBrush(QBrush(fillColor.dark(option->state & QStyle::State_Sunken ? 120 : 100)));

    painter->drawRect(QRect(14, 14, 79, 39));
    if (option->levelOfDetail >= 1) {
        painter->setPen(QPen(Qt::gray, 1));
        painter->drawLine(15, 54, 94, 54);
        painter->drawLine(94, 53, 94, 15);
    }
    painter->setPen(oldPen);
    
    if (option->levelOfDetail >= 2) {
        // Draw text
        QFont font("Times", 10);
        font.setStyleStrategy(QFont::ForceOutline);
        painter->setFont(font);
        painter->setRenderHint(QPainter::TextAntialiasing, false);
        painter->save();
        painter->scale(0.1, 0.1);
        painter->drawText(170, 180, QString("Model: VSC-2000 (Very Small Chip) at %1x%2").arg(x).arg(y));
        painter->drawText(170, 200, QString("Serial number: DLWR-WEER-123L-ZZ33-SDSJ"));
        painter->drawText(170, 220, QString("Manufacturer: Chip Manufacturer"));
        painter->restore();
    }
    if (option->levelOfDetail >= 0.5) {
        // Draw lines
        for (int i = 0; i <= 10; i += (option->levelOfDetail > 0.5 ? 1 : 2)) {
            painter->drawLine(18 + 7 * i, 13, 18 + 7 * i, 5);
            painter->drawLine(18 + 7 * i, 54, 18 + 7 * i, 62);
        }
        for (int i = 0; i <= 6; i += (option->levelOfDetail > 0.5 ? 1 : 2)) {
            painter->drawLine(5, 18 + i * 5, 13, 18 + i * 5);
            painter->drawLine(94, 18 + i * 5, 102, 18 + i * 5);
        }
    }
    if (option->levelOfDetail >= 0.4) {
        painter->setPen(oldPen);
        painter->drawLine(25, 35, 35, 35);
        painter->drawLine(35, 30, 35, 40);
        painter->drawLine(35, 30, 45, 35);
        painter->drawLine(35, 40, 45, 35);
        painter->drawLine(45, 30, 45, 40);
        painter->drawLine(45, 35, 55, 35);
    }

    painter->setPen(QPen(Qt::red, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter->setBrush(Qt::NoBrush);
    QPainterPath path;
    if (stuff.size() > 1) {
        path.moveTo(stuff.first());
        for (int i = 1; i < stuff.size(); ++i)
            path.lineTo(stuff.at(i));
        painter->drawPath(path);
    }
}

bool Chip::contains(const QPointF &point) const
{
    // Inside filled rect
    if (point.x() >= 14.0 && point.x() <= 93.0 && point.y() >= 14.0 && point.y() <= 53.0)
        return true;
    return false;
}

void Chip::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mousePressEvent(event);
    update();
}

void Chip::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->modifiers() & Qt::ShiftModifier) {
        stuff << event->pos();
        update();
        return;
    }
    QGraphicsItem::mouseMoveEvent(event);
}

void Chip::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mouseReleaseEvent(event);
    update();
}
