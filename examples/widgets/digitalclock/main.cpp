#include <QApplication>

#include "digitalclock.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    DigitalClock clock;
    app.setMainWidget(&clock);
    clock.show();
    return app.exec();
}
