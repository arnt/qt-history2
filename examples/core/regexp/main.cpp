#include <QtGui>

#include "regexp.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    Regexp form;
    form.show();
    app.connect(&app, SIGNAL(lastWindowClosed()), &app, SLOT(quit()));
    return app.exec();
}
