#include <QApplication>

#include "launcherpanel.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    LauncherPanel panel;
    panel.show();
    return app.exec();
}
