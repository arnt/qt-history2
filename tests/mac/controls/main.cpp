#include <qapplication.h>
#include <Carbon/Carbon.h>
#include "qmaccontrol.h"

int
main(int argc, char **argv)
{
    QApplication a(argc, argv);
    QWidget *window = new QWidget(NULL);
    a.setMainWidget(window);
    
    Rect r; 
    SetRect(&r, 0, 0, 50, 35);
    ControlRef button;
    CreatePushButtonControl((WindowPtr)window->handle(),  &r, 0, &button);
    QMacControl qmc(window, button, "ButtonControl");

    window->show();
    return a.exec();
}
