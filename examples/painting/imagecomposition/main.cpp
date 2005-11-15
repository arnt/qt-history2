#include <QApplication>

#include "imagecomposer.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    ImageComposer composer;
    composer.show();
    return app.exec();
}
