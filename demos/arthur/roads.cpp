#include "roads.h"

#include <qline.h>
#include <qpainter.h>

Roads::Roads(QWidget *parent)
{
    timeoutRate = 20;

    yellowLine.moveTo(100, 40);
    yellowLine.curveTo(140, 40, 170, 60, 200, 60);
    yellowLine.curveTo(260, 40, 280, 40, 330, 50);
    yellowLine.curveTo(380, 60, 360, 100, 340, 120);
    yellowLine.curveTo(320, 140, 300, 200, 300, 300);
    yellowLine.curveTo(300, 400, 160, 400, 180, 300);
    yellowLine.curveTo(200, 200, 100, 170, 90, 170);
    yellowLine.curveTo(0, 170, 0, 40, 100, 40);

//     yellowLine = yellowLine.toReversed();

//     yellowLine.addEllipse(0, 0, 400, 400);

    QPainterPathStroker stroker;
    stroker.setWidth(10);
    carVectors = stroker.createStroke(yellowLine).toSubpathPolygons();
    pixmap.load("car8x16.png");

//     printf("Number of car vectors: %d\n", carVectors.size());
//     for (int i=0; i<carVectors.size(); ++i) {
//         printf(" -> size: %d\n", carVectors.at(i).size());
//     }
}

void Roads::paintEvent(QPaintEvent *e)
{
    QPainter p(this);
    fillBackground(&p);

    if (attributes->antialias)
        p.setRenderHints(QPainter::LineAntialiasing);

    int offset = 100;
    QRect r(offset, offset, width() - 2*offset, height() - 2*offset);
    QRect roadRect(0, 0, 400, 400);

    p.setPen(Qt::black);
    p.setBrush(QColor(255, 255, 255, attributes->alpha ? 127 : 255));
    p.drawRect(r);

    p.translate(r.topLeft());
    p.scale(r.width() / 400.0, r.height() / 400.0);

    p.strokePath(yellowLine, QPen(Qt::black, 20, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    p.strokePath(yellowLine, QPen(Qt::yellow, 0, Qt::DotLine));

    for (int c=0; c<5; ++c) {
        int i = c % carVectors.size();
        int t = (animationStep + c*17) % carVectors.at(i).size();
        QLineF vec = t == carVectors.at(i).size()-1
                     ? QLineF(carVectors.at(i).at(0), carVectors.at(i).at(1))
                     : QLineF(carVectors.at(i).at(t), carVectors.at(i).at(t+1));
        p.save();
        p.translate(vec.start().toPoint());
        vec = vec.normalVector();
        float angle = vec.angle(QLineF(0, 0, 1, 0));

        // Shift angle to 360
        if (vec.vy() < 0)
            angle = 360 - angle;


        p.setPen(Qt::NoPen);
        p.setBrush(Qt::red);
        p.rotate(angle-180);

        //         p.drawRect(-8, -4, 16, 8);
        p.drawPixmap(-4, -8, pixmap);

        p.translate(-vec.start().toPoint());
        p.restore();
    }
}
