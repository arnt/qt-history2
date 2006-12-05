#include <QtGui>
#include <stdio.h>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    QLabel label("This process is just waiting to die");
    label.show();

    int c;
    fgetc(stdin); // block until fed

    qDebug("Process is running");

    return 0;
}
