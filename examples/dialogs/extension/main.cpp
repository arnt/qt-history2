#include <QApplication>

#include "finddialog.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    FindDialog dialog;
    app.setMainWidget(&dialog);
    dialog.show();
    return app.exec();
}
