#include <QApplication>

#include "classwizard.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    ClassWizard wizard;
    return wizard.exec();
}
