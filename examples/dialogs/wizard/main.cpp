#include <QApplication>

#include "licensewizard.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    LicenseWizard wizard;
    app.setMainWidget(&wizard);
    wizard.show();
    return app.exec();
}
