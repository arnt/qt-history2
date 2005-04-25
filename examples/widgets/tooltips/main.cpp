#include <QApplication>

#include "sortingbox.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    SortingBox sortingBox;
    sortingBox.show();
    return app.exec();
}
