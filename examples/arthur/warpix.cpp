#include "warpix.h"

#include <qbitmap.h>
#include <qevent.h>
#include <qpainter.h>

Warpix::Warpix(QWidget *parent)
    : DemoWidget(parent)
{
    setPixmap(QPixmap("chux.png"));

    animationLoopStep = -1;

    beat = new QSound("beat.wav");
    beat->setLoops(-1);

    clickPos = QPoint(-1, -1);
}

void Warpix::paintEvent(QPaintEvent *)
{
    int w = width();
    int h = height();
    QPainter p(this);

    // Fill background based on user specified attributes.
    fillBackground(&p);

    double x = 0;
    double y = 0;
    if (clickPos.x() == -1 && clickPos.y() == -1) {
        x = xfunc(animationStep);
        y = yfunc(animationStep);

        x = x * w/3 + w/2;
        y = y * h/3 + h/2;
    } else {
        x = clickPos.x();
        y = clickPos.y();
    }

    p.drawPixmap(QRect(0, 0, x+1, y+1), p1);
    p.drawPixmap(QRect(x, 0, w-x, y+1), p2);
    p.drawPixmap(QRect(0, y, x+1, h-y), p3);
    p.drawPixmap(QRect(x, y, w-x, h-y), p4);
}

static void copy_pixmap(const QPixmap *src, QPixmap *dest, int x, int y, int w, int h)
{
    dest->resize(w, h);
    bitBlt(dest, 0, 0, src, x, y, w, h);
    if (src->mask()) {
	QBitmap mask(w, h);
	bitBlt(&mask, 0, 0, src->mask(), x, y, w, h);
	dest->setMask(mask);
    }
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
    clickPos = QPoint(qLimit(100, width()-200, e->x()),
                      qLimit(100, height()-200, e->y()));

}
