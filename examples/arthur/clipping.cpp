#include "clipping.h"

#include <qapplication.h>
#include <qbitmap.h>
#include <qpainter.h>
#include <qevent.h>

Clipping::Clipping(QWidget *parent)
    : DemoWidget(parent)
{
    pressPoint = QPoint(-1, -1);

    const int rectCount = 10;

    for (int i=0; i<rectCount; ++i) {
        int width  = 100;
        int height = 100;

        int x = i*7;
        int y = i*13;

        rects.append(QRect(x, y, width, height));
        rectDirection.append(QPoint(xfunc(i*113)*5 + 5, yfunc(i*113)*5 + 5));
    }
}

void Clipping::paintEvent(QPaintEvent *)
{
    int w = width(), h = height();

    QPainter pt(this);

    fillBackground(&pt);

    QRegion region;
    for (int i=0; i<rects.size(); ++i) {
        QRect r = rects.at(i);
        QPoint d = rectDirection.at(i);
        r.moveBy(d);

        if (r.left() < 0) {
            r.setRect(0, r.y(), r.width(), r.height());
            d.setX(-d.x());
        } else if (r.right() > w) {
            r.setRect(w-r.width(), r.y(), r.width(), r.height());
            d.setX(-d.x());
        }

        if (r.top() < 0) {
            r.setRect(r.x(), 0, r.width(), r.height());
            d.setY(-d.y());
        } else if (r.bottom() > h) {
            r.setRect(r.x(), h-r.height(), r.width(), r.height());
            d.setY(-d.y());
        }

        if (i%4 == 0)
            region |= QRegion(r, QRegion::Ellipse);
        else
            region |= r;

        rects[i] = r;
        rectDirection[i] = d;
    }

    if (pressPoint != QPoint(-1, -1)) {
        QRect mouseRect = QRect(pressPoint, currentPoint);
        region ^= mouseRect.normalize();
    }

    QRegion clip(0, 0, w, h);
    clip ^= region;
    pt.setClipRegion(clip);

//     pt.setBrush(QBrush(QPoint(0, 0), QColor(220, 220, 255, attributes->alpha ? 191 : 255),
//                        QPoint(0, h), QColor(63, 63, 150, attributes->alpha ? 191 : 255)));
    QColor bg = palette().color(QPalette::Background);
    pt.setPen(Qt::NoPen);
    pt.setBrush(QColor(bg.red(), bg.green(), bg.blue(), attributes->alpha ? 191 : 255));

    pt.drawRect(rect());
}

void Clipping::resizeEvent(QResizeEvent *)
{
    bgFill = QPixmap();
}

void Clipping::mousePressEvent(QMouseEvent *e)
{
    pressPoint = e->pos();
    currentPoint = e->pos();
}

void Clipping::mouseMoveEvent(QMouseEvent *e)
{
    currentPoint = e->pos();
}
