#include "tiger.h"

#include <qevent.h>
#include <qpainter.h>
#include <qpainterpath.h>

#include <math.h>

struct Shape {

    Shape() : hasPen(false), hasFill(false)
    {
    }

    QPainterPath path;
    QColor penColor;
    QColor fillColor;

    bool hasPen, hasFill;

    void setPenColor(const QColor &c) { penColor = c; hasPen = true; }
    void setFillColor(const QColor &c) { fillColor = c; hasFill = true; }
};

static Shape *currentShape;
static QList<Shape*> shapes;

void createShapes() {
#include "tiger_commands.h"
}

Tiger::Tiger(QWidget *parent) : DemoWidget(parent)
{
    timeoutRate = -1;
    createShapes();
    zoom = 1;

    setMouseTracking(true);

    mx = my = -1;
    flip = 1;
}

void Tiger::wheelEvent(QWheelEvent *e)
{
    zoom += e->delta() / (120.0 * 4);
    if (zoom < 0.1)
        zoom = 0.1;
    update();
}

void Tiger::mousePressEvent(QMouseEvent *)
{
    flip = flip == 1 ? -1 : 1;
    update();
}

void Tiger::mouseMoveEvent(QMouseEvent *e)
{
    mx = e->x();
    my = e->y();
    update();
}

void Tiger::paintEvent(QPaintEvent *)
{
    QMatrix m;
    m.translate(200 + width() / 2 - 300, 190 + height() / 2 - 300);
    m.scale(zoom, zoom);

    QMatrix minv = m.invert();
    QPoint mpt = QPoint(mx, my) * minv;

    QPainter p(this);
    fillBackground(&p);

    if (attributes->antialias)
        p.setRenderHint(QPainter::Antialiasing);

    p.setMatrix(m);

    const int dist = 100;

    for (int i=0; i<shapes.size(); ++i) {
        const Shape *s = shapes.at(i);
        if (!s)
            continue;

        QPainterPath path;
        path.addPath(s->path);

        for (int i=0; i<path.elementCount(); ++i) {
            const QPainterPath::Element &e = path.elementAt(i);

            float dx = e.x - mpt.x();
            float dy = e.y - mpt.y();
            float len = dist - sqrt(dx * dx + dy * dy);

            if (len > 0) {
                const_cast<QPainterPath::Element *>(&e)->x = e.x + flip * dx * len / float(dist);
                const_cast<QPainterPath::Element *>(&e)->y = e.y + flip * dy * len / float(dist);
            }
        }

        p.setPen(s->hasPen ? QPen(s->penColor) : QPen(Qt::NoPen));
        p.setBrush(s->hasFill ? QBrush(s->fillColor) : QBrush(Qt::NoBrush));
        p.drawPath(path);
    }


}


