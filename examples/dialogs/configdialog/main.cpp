#include <QApplication>

#include "configdialog.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    ConfigDialog dialog;
    return dialog.exec();
}
