#include <QtGui/QtGui>

int main()
{
    QPainter painter;
    // PATH
    QPainterPath path;
    path.addRect(20, 20, 80, 80);

    path.moveTo(0, 0);
    path.cubicTo(99, 0,  50, 50,  99, 99);
    path.cubicTo(0, 99,  50, 50,  0, 0);

    painter.drawPath(path);
    return 0;
}
