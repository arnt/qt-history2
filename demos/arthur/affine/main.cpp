#include "xform.h"

#include <qapplication.h>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    XFormWidget xformWidget(0);
    QStyle *arthurStyle = new ArthurStyle();
    xformWidget.setStyle(arthurStyle);

    QList<QWidget *> widgets = qFindChildren<QWidget *>(&xformWidget);
    foreach (QWidget *w, widgets)
        w->setStyle(arthurStyle);

    xformWidget.show();

    return app.exec();
}
