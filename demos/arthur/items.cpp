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

#include "items.h"

class Item
{
public:
    enum Shape {Circle, Rectangle, Text};

    Item(QPoint topLeft, Shape _shape) : shape(_shape), sel(false) {
        translate(topLeft);
    }

    void draw(QPainter *p) {
        p->save();
        if (sel)
            p->setPen(Qt::white);
        else
            p->setPen(Qt::black);
        switch (shape) {
        case Circle:
            p->setBrush(QColor(120, 120, 255, 127));
            p->drawEllipse(trans.x(), trans.y(), 100, 100);
            break;
        case Rectangle:
            p->setBrush(QColor(120, 255, 120, 127));
            p->drawRect(trans.x(), trans.y(), 100, 100);
            break;
        case Text:
            p->drawText(trans.x(), trans.y(), "Trolltech");
            break;
        }
        p->restore();
    }

    QRect boundingRect() const {
        QRect r;
        switch (shape) {
        case Circle:
        case Rectangle:
            r = QRect(trans.x(), trans.y(), 100, 100);
            break;
        case Text:
            QFontMetrics fm(qApp->font());
            r = fm.boundingRect("Trolltech");
            r.setX(trans.x());
            r.setY(trans.y());
            break;
        }
        return r;
    }

    void setSelected(bool selected) {
        sel = selected;
    }

    void setOffset(QPoint p) {
        offset = p - trans;
    }

    bool selected() const {
        return sel;
    }

    void translate(QPoint p) {
        trans = p - offset;
    }

private:
    QPoint offset;
    QPoint trans;
    Shape shape;
    bool sel;
};

Items::Items(QWidget *parent)
    : DemoWidget(parent)
{
    srand(QTime::currentTime().msec());
    for (int i = 0; i < 200; ++i) {
        items.append(new Item(QPoint(rand()%512, rand()%512), Item::Rectangle));
        items.append(new Item(QPoint(rand()%512, rand()%512), Item::Circle));
    }
}

Items::~Items()
{
    for (int i = 0; i < items.size(); ++i)
        delete items[i];
}

void Items::paintEvent(QPaintEvent *pe)
{
    QPainter p(this);
    p.drawPixmap(0, 0, buffer);

    p.setBrush(QColor(255,255,255,80));
    if (!anchor.isNull())
        p.drawRect(itemBr);
}

void Items::mousePressEvent(QMouseEvent *event)
{
    anchor = current = event->pos();

    bool selected = false;
    for (int i = items.size()-1; i >= 0; --i) {
        if (!selected && items.at(i)->boundingRect().contains(anchor)) {
            items[i]->setSelected(true);
            items[i]->setOffset(anchor);
            itemBr = items.at(i)->boundingRect();
            selected = true;
        } else {
            items[i]->setSelected(false);
        }
    }
    if (!itemBrOrig.isEmpty())
        drawItems(itemBrOrig);
    itemBrOrig = itemBr;
    drawItems(itemBr);
    update();
}

void Items::mouseMoveEvent(QMouseEvent *event)
{
    current = event->pos();
    for (int i = 0; i < items.size(); ++i) {
        if (items.at(i)->selected()) {
            items[i]->translate(current);
            itemBr = items.at(i)->boundingRect();
            break;
        }
    }
    update();
}

void Items::mouseReleaseEvent(QMouseEvent *event)
{
    anchor = current = QPoint();
    drawItems(itemBr);
    if (!itemBrOrig.isEmpty())
        drawItems(itemBrOrig);
    itemBrOrig = itemBr;
    update();
}

void Items::resizeEvent(QResizeEvent *)
{
    buffer.resize(size());
    drawItems(QRect(QPoint(0,0),size()));
}

void Items::drawItems(const QRect &rect)
{
    QRect result = rect;
    QPainter px(&buffer);
    int drawn = 0;
    for (int i = 0; i < items.size(); ++i) {
        if (rect.isEmpty() || items.at(i)->boundingRect().intersects(rect)) {
            result |= items.at(i)->boundingRect();
            ++drawn;
        }
    }
    px.setClipRect(result);
    fillBackground(&px);
    for (int i = 0; i < items.size(); ++i) {
        if (items[i]->boundingRect().intersects(result))
            items[i]->draw(&px);
    }
//    qDebug() << drawn;
}
