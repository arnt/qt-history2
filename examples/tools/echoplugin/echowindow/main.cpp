#include <QtGui>

#include "echowindow.h"
#include "echointerface.h"

int main(int argv, char *args[])
{
    QApplication app(argv, args);

    EchoWindow window;
    window.show();

    return app.exec();
}
