#include <QtGui/QtGui>

int main()
{
    QWidget anyPaintDevice;
    // BRUSH SNIPPET
    QPainter painter;
    QBrush brush(Qt::yellow);           // yellow solid pattern
    painter.begin(&anyPaintDevice);   // paint something
    painter.setBrush(brush);          // set the yellow brush
    painter.setPen(Qt::NoPen);        // do not draw outline
    painter.drawRect(40,30, 200,100); // draw filled rectangle
    painter.setBrush(Qt::NoBrush);    // do not fill
    painter.setPen(Qt::black);            // set black pen, 0 pixel width
    painter.drawRect(10,10, 30,20);   // draw rectangle outline
    painter.end();                    // painting done

    // LINEAR
    QLinearGradient linearGrad(QPointF(100, 100), QPointF(200, 200));
    linearGrad.setColorAt(0, Qt::black);
    linearGrad.setColorAt(1, Qt::white);

    // RADIAL
    QRadialGradient radialGrad(QPointF(100, 100), 100);
    radialGrad.setColorAt(0, Qt::red);
    radialGrad.setColorAt(0.5, Qt::blue);
    radialGrad.setColorAt(1, Qt::green);
}
