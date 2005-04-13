#include <QApplication>

#include "imageviewer.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    ImageViewer imageViewer;
    imageViewer.show();
    return app.exec();
}
