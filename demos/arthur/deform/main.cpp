#include "pathdeform.h"

#include <qapplication.h>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    PathDeformWidget deformWidget(0);
    QStyle *arthurStyle = new ArthurStyle();
    deformWidget.setStyle(arthurStyle);
    foreach (QWidget *w, qFindChildren<QWidget *>(&deformWidget))
        w->setStyle(arthurStyle);
    deformWidget.show();

    return app.exec();
}
