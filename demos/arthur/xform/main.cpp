#include "xform.h"

#include <qapplication.h>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    XFormWidget xformWidget(0);
    QStyle *arthurStyle = new ArthurStyle();
    xformWidget.setStyle(arthurStyle);
    foreach (QWidget *w, qFindChildren<QWidget *>(&xformWidget))
        w->setStyle(arthurStyle);
    xformWidget.show();

    return app.exec();
}
