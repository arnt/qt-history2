#include <QApplication>
#include "textfinder.h"

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(textfinder);
    QApplication app(argc, argv);

    TextFinder *textFinder = new TextFinder;
    textFinder->show();

    return app.exec();
}