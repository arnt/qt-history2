#include <QtGui>

#include "dialog.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    Dialog dialog;
    app.setMainWidget(&dialog);
    dialog.show();
    return app.exec();
}
