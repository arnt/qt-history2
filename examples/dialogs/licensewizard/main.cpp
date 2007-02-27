#include <QApplication>

#include "licensewizard.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    LicenseWizard wizard;
    wizard.show();
    return app.exec();
}
