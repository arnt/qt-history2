#include <QtGui>

#include "http.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    Http http;
    app.setMainWidget(&http);
    http.show();
    return app.exec();
}
