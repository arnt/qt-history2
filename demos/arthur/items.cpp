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
            path->curveTo(99,0, 50,50, 99,99);
            path->moveTo(99,99);
            path->curveTo(0,99, 50,50, 0,0);
            QFont fnt("times", 75);
            QRect r = QFontMetrics(fnt).boundingRect("Trolltech");
            path->addText(-r.center() + QPoint(50, 140), fnt, "Trolltech");
        }
    }

    ~Item() {
        if (shape == Path)
            delete path;
    }

    void draw(QPainter *p, bool useOwnColor = true) {
        if (selected)
            p->setPen(Qt::red);
        else
            p->setPen(Qt::black);
        if (useOwnColor)
            p->setBrush(color);
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
            r.addCoords(-1, -1, 1, 1);
            r.translate(trans);
            break;
        }
        return r;
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
    srand(QTime::currentTime().msec());
    for (int i = 0; i < 500; ++i) {
        items.append(new Item(QPoint(50+rand()%380, 50+rand()%380), Item::Rectangle,
                              QColor(120+rand()%136,120+rand()%136,120+rand()%136)));
        items.append(new Item(QPoint(50+rand()%380, 50+rand()%380), Item::Circle,
                              QColor(120+rand()%136,120+rand()%136,120+rand()%136)));
    }

    Item *item = new Item(QPoint(), Item::Path, QColor(120, 255, 120, 200));
    item->translate(QPoint(591/2, 600/2) - item->boundingRect().center());
    items.append(item);
    dragging = false;
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

    if (dragging) {
        p.setBrush(QColor(120, 120, 255, 100));
        items.last()->draw(&p, false);
    }
}

void Items::mousePressEvent(QMouseEvent *event)
{
    dragging = true;
    for (int i = items.size()-1; i >= 0; --i) {
        if (items.at(i)->boundingRect().contains(event->pos())) {
            items.last()->setSelected(false);
            items[i]->setSelected(true);
            items[i]->setOffset(event->pos());
            itemBr = items.at(i)->boundingRect();
            items.move(i, items.size()-1);
            break;
        }
    }
    if (!itemBrOrig.isEmpty())
        drawItems(itemBrOrig);
    itemBrOrig = itemBr;

    QPainter px(&buffer);
    items.last()->draw(&px);
    px.end();

    update();
}

void Items::mouseMoveEvent(QMouseEvent *event)
{
    items.last()->translate(event->pos());
    itemBr = items.last()->boundingRect();
    update();
}

void Items::mouseReleaseEvent(QMouseEvent *)
{
    dragging = false;
    drawItems(itemBr);
    if (!itemBrOrig.isEmpty())
        drawItems(itemBrOrig);
    itemBrOrig = itemBr;
    update();
}

void Items::resizeEvent(QResizeEvent *)
{
    buffer.resize(size());
    drawItems(QRect());
}

void Items::drawItems(const QRect &rect)
{
    QPainter px(&buffer);

    if (!rect.isEmpty())
        px.setClipRect(rect);
    fillBackground(&px);
    for (int i = 0; i < items.size(); ++i) {
        if (rect.isEmpty() || items.at(i)->boundingRect().intersects(rect))
            items[i]->draw(&px);
    }
}

void Items::resetState()
{
    drawItems(QRect());
    update();
}
