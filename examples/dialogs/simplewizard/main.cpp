#include <QApplication>

#include "classwizard.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    ClassWizard wizard;
    app.setMainWidget(&wizard);
    wizard.show();
    return app.exec();
}
