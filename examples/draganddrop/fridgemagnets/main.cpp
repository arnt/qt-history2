#include <QApplication>
#include "dragwidget.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    DragWidget *window = new DragWidget;
    window->show();
    return app.exec();
}
