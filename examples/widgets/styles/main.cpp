#include <QApplication>

#include "widgetgallery.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    WidgetGallery gallery;
    return gallery.exec();
}
