#include <QtGui>

#include "stylewindow.h"

int main(int argv, char *args[])
{
    QApplication app(argv, args);
    QApplication::setStyle(QStyleFactory::create("simplestyle"));

    StyleWindow window;
    window.resize(200, 50);
    window.show();

    return app.exec();
}
