#include <QtGui>

#include "regexpdialog.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    RegExpDialog dialog;
    app.setMainWidget(&dialog);
    dialog.show();
    return app.exec();
}
