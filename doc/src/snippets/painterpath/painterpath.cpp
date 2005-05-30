#include <QtGui/QtGui>



int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    QImage image(100, 100, QImage::Format_RGB32);
    image.fill(0xffffffff);

    QPainter painter(&image);
    painter.fillRect(0, 0, 100, 100, Qt::white);

    painter.save();
    painter.translate(0.5, 0.5);
    painter.setPen(QPen(QColor(103, 103, 150), 3, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin));
    painter.setBrush(QColor(223, 223, 255));
    painter.setRenderHint(QPainter::Antialiasing);

    // PATH
    QPainterPath path;
    path.addRect(20, 20, 60, 60);

    path.moveTo(0, 0);
    path.cubicTo(99, 0,  50, 50,  99, 99);
    path.cubicTo(0, 99,  50, 50,  0, 0);

    painter.drawPath(path);

    painter.restore();
    painter.setPen(Qt::black);
    painter.drawRect(0, 0, 99, 99);
    painter.end();

    QLabel lab;
    lab.setPixmap(QPixmap::fromImage(image));
    lab.show();
    return app.exec();
}
