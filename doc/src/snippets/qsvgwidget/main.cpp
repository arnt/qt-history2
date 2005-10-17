#include <QtGui>
#include <QtSvg>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QSvgWidget window(":/files/sunflower.svg");
    window.show();
    return app.exec();
}
