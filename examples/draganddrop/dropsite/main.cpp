#include <QApplication>
#include "dropsitewindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    DropSiteWindow *window = new DropSiteWindow;
    window->show();

    return app.exec();
}
