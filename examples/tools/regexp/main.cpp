#include <QApplication>

#include "regexpdialog.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    RegExpDialog dialog;
    dialog.show();
    return dialog.exec();
}
