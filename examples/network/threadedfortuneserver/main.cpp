#include <QApplication>

#include <stdlib.h>
#include <time.h>

#include "dialog.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    Dialog dialog;
    dialog.show();
    srand(time(0));
    return dialog.exec();
}
