#include <qapplication.h>
#include <Carbon/Carbon.h>
#include "qmaccontrol_mac.h"
#include <qpushbutton.h>

int
main(int argc, char **argv)
{
    //standard things
    QApplication a(argc, argv);
    QWidget *window = new QWidget(NULL, "TopLevel");
    a.setMainWidget(window);
    window->show();

    //interesting part, create a macpushbutton, and bind it with a qwidget
    Rect r; 
    SetRect(&r, 0, 0, 50, 35);
    ControlRef button;
    CreatePushButtonControl((WindowPtr)window->handle(),  &r, 0, &button);
    Boolean t = true;
    SetControlData(button, 0, kControlPushButtonDefaultTag, sizeof(t), &t);
    QMacControl qmc(window, button, "ButtonControl");
    qmc.setGeometry(50, 50, 200, 25);
    qmc.setCaption("Hello World");

    QPushButton pb(window, "ButtonControl2");
    pb.setGeometry(200, 40, 200, 25);
    pb.setCaption("Hello World2");
    pb.show();

    //finally event loop
    return a.exec();
}
