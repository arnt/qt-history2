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

#include <QApplication>
#include <QPainter>
#include <QMouseEvent>
#include <QDateTime>
#include <QPainterPath>
#include <QMatrix>
#include <stdlib.h>
#include <qdebug.h>
#include "items.h"

class Item
{
public:
    enum Shape {Circle, Rectangle, Path};

    Item(QPoint topLeft, Shape _shape, const QColor &c)
        : shape(_shape), selected(false), color(c) {
        translate(topLeft);
        if (shape == Path) {
            path = new QPainterPath;
            path->addRect(20,20, 60,60);
            path->moveTo(0,0);
            path->cubicTo(99,0, 50,50, 99,99);
            path->moveTo(99,99);
            path->cubicTo(0,99, 50,50, 0,0);
#ifndef Q_WS_QWS
            QFont fnt("times", 75);
#else
            QFont fnt("vera_sans", 75);
#endif
            QRect r = QFontMetrics(fnt).boundingRect("Trolltech");
            path->addText(-r.center() + QPoint(50, 140), fnt, "Trolltech");
        }
    }

    ~Item() {
        if (shape == Path)
            delete path;
    }

    void draw(QPainter *p, bool useSpecialColor = false) {
        if (selected)
            p->setPen(Qt::red);
        else
            p->setPen(Qt::black);
        if (!useSpecialColor) {
            p->setBrush(color);
        } else {
            QColor c = color;
            c.setAlpha(c.alpha()/2);
            c = c.dark();
            p->setBrush(c);
        }
        switch (shape) {
        case Circle:
            p->drawEllipse(trans.x(), trans.y(), 99, 99);
            break;
        case Rectangle:
            p->drawRect(trans.x(), trans.y(), 99, 99);
            break;
        case Path:
            p->drawPath(*path * QMatrix(1, 0, 0, 1, trans.x(), trans.y()));
            break;
        }
    }

    QRect boundingRect() const {
        QRect r;
        switch (shape) {
        case Circle:
        case Rectangle:
            r = QRect(trans.x(), trans.y(), 100, 100);
            break;
        case Path:
            r = path->boundingRect().toRect();
            r.adjust(-1, -1, 1, 1);
            r.translate(trans);
            break;
        }
        return r;
    }

    bool contains(const QPoint &p) const {
        QRect br = boundingRect();
        if (shape == Circle) {
            QPoint p2 = p - br.center();
            return p2.x() * p2.x() + p2.y() * p2.y() < br.width()/2 * br.height()/2;
        }
        return br.contains(p);
    }

    void setSelected(bool sel) {
        selected = sel;
    }

    void setOffset(QPoint p) {
        offset = p - trans;
    }

    void translate(QPoint p) {
        trans = p - offset;
    }

private:
    QPoint offset;
    QPoint trans;
    Shape shape;
    bool selected;
    QColor color;
    QPainterPath *path;
};

Items::Items(QWidget *parent)
    : DemoWidget(parent)
{
    selectedItem = 0;
      srand(QTime::currentTime().msec());
    for (int i = 0; i < 500; ++i) {
        items.append(new Item(QPoint(50+rand()%380, 50+rand()%380), Item::Rectangle,
                              QColor(120+rand()%136,120+rand()%136,120+rand()%136)));
        items.append(new Item(QPoint(50+rand()%380, 50+rand()%380), Item::Circle,
                              QColor(120+rand()%136,120+rand()%136,120+rand()%136)));
    }

#if 0
     Item *item = new Item(QPoint(), Item::Path, QColor(120, 255, 120, 200));
     item->translate(QPoint(591/2, 600/2) - item->boundingRect().center());
     items.append(item);
#endif
}

Items::~Items()
{
    for (int i = 0; i < items.size(); ++i)
        delete items[i];
}

void Items::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.drawPixmap(0, 0, buffer);

    if (selectedItem)
        selectedItem->draw(&p, true);
}

void Items::mousePressEvent(QMouseEvent *event)
{
    for (int i = items.size()-1; i >= 0; --i) {
        if (items.at(i)->contains(event->pos())) {
            if (selectedItem)
                selectedItem->setSelected(false);
            selectedItem = items[i];
            selectedItem->setSelected(true);
            selectedItem->setOffset(event->pos());
            items.move(i, items.size()-1); // raise
            break;
        }
    }
    if (!selectedItem)
        return;
    drawItems(selectedItem->boundingRect());

    update();
}

void Items::mouseMoveEvent(QMouseEvent *event)
{
    if (selectedItem) {
        selectedItem->translate(event->pos());
        update();
    }
}

void Items::mouseReleaseEvent(QMouseEvent *)
{
    if (selectedItem) {
        selectedItem->setSelected(false);
        QRect br = selectedItem->boundingRect();
        selectedItem = 0;
        drawItems(br);
    }
    update();
}

void Items::resizeEvent(QResizeEvent *event)
{
    DemoWidget::resizeEvent(event);
    buffer = QPixmap(size());
    drawItems(QRect());
}

void Items::drawItems(const QRect &rect)
{
    QPainter px(&buffer);

    if (!rect.isEmpty())
        px.setClipRect(rect);
    drawBackground(&px);
    for (int i = 0; i < items.size(); ++i) {
        if (rect.isEmpty() || items.at(i)->boundingRect().intersects(rect))
            if (items[i] != selectedItem)
                items[i]->draw(&px);
    }
}

void Items::resetState()
{
    drawItems(QRect());
    update();
}
