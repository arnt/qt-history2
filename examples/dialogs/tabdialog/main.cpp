#include <QApplication>

#include "tabdialog.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
//    app.setDesktopSettingsAware(false);
    QString fileName;

    if (argc >= 2)
        fileName = argv[1];
    else
        fileName = ".";

    TabDialog tabdialog(fileName);
    tabdialog.show();
    app.setMainWidget(&tabdialog);
    return app.exec();
}
