#include <QtGui>

#include "dialog.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    Dialog *dialog = new Dialog(0);
    app.setMainWidget(dialog);
    dialog->setWindowTitle("QIconSet");
    dialog->resize(400, 240);
    dialog->show();

    return app.exec();
}

