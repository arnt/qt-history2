#include <QApplication>

#include "configdialog.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    ConfigDialog *dialog = new ConfigDialog;
    dialog->show();
    app.setMainWidget(dialog);
    return app.exec();
}
