#include "ui_imagedialog.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QDialog window;
    Ui::Form ui;
    ui.setupUi(&window);

    app.setMainWidget(&window);
    window.show();
    return app.exec();
}
