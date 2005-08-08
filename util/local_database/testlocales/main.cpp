#include <QtGui/QApplication>

#include "localewidget.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    LocaleWidget wgt;
    wgt.show();
    return app.exec();
}
