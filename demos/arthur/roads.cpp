#include "roads.h"

#include <qline.h>
#include <qpainter.h>

Roads::Roads(QWidget *parent)
{
    yellowLine.moveTo(0, 0);

    yellowLine.curveTo(100, 0, 100, 100, 0, 100);


    QPainterPathStroker stroker;
    stroker.setWidth(10);
    carVector = stroker.createStroke(yellowLine).toFillPolygon();
}

void Roads::paintEvent(QPaintEvent *e)
{
    QPainter p(this);
    fillBackground(&p);

    if (attributes->antialias)
        p.setRenderHints(QPainter::LineAntialiasing);

    int offset = 100;
    QRect r(offset, offset, width() - 2*offset, height() - 2*offset);

    p.translate(r.topLeft());
//     p.scale(r.width() / 100.0, r.height() / 100.0);

    p.strokePath(yellowLine, QPen(Qt::black, 20, Qt::SolidLine, Qt::RoundCap,
                                  Qt::RoundJoin));

    p.setPen(QPen(Qt::yellow, 0, Qt::DotLine));
    p.drawPath(yellowLine);

    int t = animationStep % carVector.size();
    QLineF vec = t == carVector.size()-1
                 ? QLineF(carVector.at(0), carVector.at(1))
                 : QLineF(carVector.at(t), carVector.at(t+1));

    p.translate(vec.start().toPoint());
    float angle = vec.angle(QLineF(0, 0, 1, 0));
//     p.rotate(angle);
//     p.translate(-10, -5);
    p.setPen(Qt::NoPen);
    p.setBrush(Qt::red);
    p.drawEllipse(-3, -3, 6, 6);

}
