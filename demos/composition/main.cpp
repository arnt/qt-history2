#include "composition.h"

#include <qapplication.h>

int main(int argc, char **argv)
{
 //   Q_INIT_RESOURCE(deform);

    QApplication app(argc, argv);

    CompositionWidget compWidget(0);
    QStyle *arthurStyle = new ArthurStyle();
    compWidget.setStyle(arthurStyle);

    QList<QWidget *> widgets = qFindChildren<QWidget *>(&compWidget);
    foreach (QWidget *w, widgets)
        w->setStyle(arthurStyle);
    compWidget.show();

    return app.exec();
}
