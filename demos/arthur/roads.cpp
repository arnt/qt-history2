#include "roads.h"

#include <qline.h>
#include <qpainter.h>

Roads::Roads(QWidget *parent)
    : DemoWidget(parent)
{
    timeoutRate = 50;

    yellowLine.moveTo(100, 40);
    yellowLine.curveTo(140, 40, 170, 60, 200, 60);
    yellowLine.curveTo(260, 60, 280, 40, 330, 50);
    yellowLine.curveTo(380, 60, 360, 100, 340, 120);
    yellowLine.curveTo(320, 140, 300, 200, 300, 300);
    yellowLine.curveTo(300, 400, 160, 400, 180, 300);
    yellowLine.curveTo(200, 200, 100, 170, 90, 170);
    yellowLine.curveTo(0, 170, 0, 40, 100, 40);

    QPainterPathStroker stroker;
    stroker.setWidth(20);
    carVectors = stroker.createStroke(yellowLine).toSubpathPolygons();
    pixmap.load(":/res/car16x32.png");

//     printf("Number of car vectors: %d\n", carVectors.size());
//     for (int i=0; i<carVectors.size(); ++i) {
//         printf(" -> size: %d\n", carVectors.at(i).size());
//     }
}

void Roads::paintEvent(QPaintEvent *)
{
    int offset = 50;
    QRect r(offset, offset, width() - 2*offset, height() - 2*offset);

    if (backBuffer.size() != size()) {
        backBuffer.resize(size());
        QPainter bp(&backBuffer);

        fillBackground(&bp);

        if (attributes->antialias)
            bp.setRenderHints(QPainter::LineAntialiasing);

        QRect roadRect(0, 0, 400, 400);

        bp.setPen(Qt::black);
        bp.setBrush(QColor(0, 127, 0, attributes->alpha ? 127 : 255));
        bp.drawRect(r);

        bp.translate(r.topLeft());
        bp.scale(r.width() / 400.0, r.height() / 400.0);

        bp.strokePath(yellowLine, QPen(Qt::black, 40, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        bp.strokePath(yellowLine, QPen(Qt::yellow, 0, Qt::DashDotLine));
    }


    QPainter p(this);
    p.drawPixmap(0, 0, backBuffer);

    if (attributes->antialias)
        p.setRenderHints(QPainter::LineAntialiasing);


    p.translate(r.topLeft());
    p.scale(r.width() / 400.0, r.height() / 400.0);



    const int carCount = 4;
    for (int c=0; c<carCount; ++c) {
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
        p.drawPixmap(-pixmap.width() / 2, -pixmap.height() / 2, pixmap);

        p.translate(-vec.start().toPoint());
        p.restore();
    }
}
