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

#include "warpix.h"

#include <qbitmap.h>
#include <qevent.h>
#include <qpainter.h>

Warpix::Warpix(QWidget *parent)
    : DemoWidget(parent)
{
    setPixmap(QPixmap(":/res/chux.png"));

#ifdef BEAT_IS_RESOURCE
    beat = new QSound(":/res/beat.wav");
#else
    beat = new QSound("beat.wav");
#endif
    beat->setLoops(-1);

    clickPos = QPoint(-1, -1);
}

void Warpix::paintEvent(QPaintEvent *)
{
    int w = width();
    int h = height();
    QPainter p(this);

    // Fill background based on user specified attributes.
    drawBackground(&p);

    double x = 0;
    double y = 0;
    if (clickPos.x() == -1 && clickPos.y() == -1) {
        x = xfunc(animationStep()/8);
        y = yfunc(animationStep()/8);

        x = x * w/3 + w/2;
        y = y * h/3 + h/2;
    } else {
        x = clickPos.x();
        y = clickPos.y();
    }

    int ix = int(x);
    int iy = int(y);
    p.drawPixmap(QRect(0, 0, ix+1, iy+1), p1);
    p.drawPixmap(QRect(ix, 0, w-ix, iy+1), p2);
    p.drawPixmap(QRect(0, iy, ix+1, h-iy), p3);
    p.drawPixmap(QRect(ix, iy, w-ix, h-iy), p4);
}

static void copy_pixmap(const QPixmap *src, QPixmap *dest, int x, int y, int w, int h)
{
    *dest = QPixmap(w, h);
    dest->fill(QColor(0, 0, 0, 0));
    QPainter pt(dest);
    pt.drawPixmap(0, 0, *src, x, y, w, h);
}

void Warpix::setPixmap(const QPixmap &pm)
{
    int w = pm.width() / 2;
    int h = pm.height() / 2;

    copy_pixmap(&pm, &p1, 0, 0, w, h);
    copy_pixmap(&pm, &p2, w, 0, w, h);
    copy_pixmap(&pm, &p3, 0, h, w, h);
    copy_pixmap(&pm, &p4, w, h, w, h);
}

void Warpix::mousePressEvent(QMouseEvent *e)
{
    beat->play();
    clickPos = QPoint(e->x(), e->y());
}

void Warpix::mouseReleaseEvent(QMouseEvent *)
{
    beat->stop();
    clickPos = QPoint(-1, -1);
}

#define qLimit(min, max, val) qMin(qMax((min), (val)), (max))

void Warpix::mouseMoveEvent(QMouseEvent *e)
{
    clickPos = QPoint(qLimit(0, width(), e->x()),
                      qLimit(0, height(), e->y()));
}
