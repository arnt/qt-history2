#include "xform.h"

#include <qapplication.h>

int main(int argc, char **argv)
{
    Q_INIT_RESOURCE(affine);

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
