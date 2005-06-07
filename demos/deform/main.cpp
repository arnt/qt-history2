#include "pathdeform.h"

#include <qapplication.h>

int main(int argc, char **argv)
{
    Q_INIT_RESOURCE(deform);

    QApplication app(argc, argv);

    PathDeformWidget deformWidget(0);
    QStyle *arthurStyle = new ArthurStyle();
    deformWidget.setStyle(arthurStyle);

    QList<QWidget *> widgets = qFindChildren<QWidget *>(&deformWidget);
    foreach (QWidget *w, widgets)
        w->setStyle(arthurStyle);
    deformWidget.show();

    return app.exec();
}
