#include <QApplication>

#include "licensewizard.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    LicenseWizard wizard;
    return wizard.exec();
}
