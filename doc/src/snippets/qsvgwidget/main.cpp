#include <QtGui>
#include <QtSvg>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QSvgWidget window(":/files/spheres.svg");
    window.show();
    QSvgRenderer *renderer = window.renderer();
    QImage image(150, 150, QImage::Format_RGB32);
    QPainter painter;
    painter.begin(&image);
    renderer->render(&painter);
    painter.end();
    image.save("spheres.png", "PNG", 9);
    return app.exec();
}
