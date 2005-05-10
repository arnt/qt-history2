#include "gradients.h"

#include <qapplication.h>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    GradientWidget gradientWidget(0);
    QStyle *arthurStyle = new ArthurStyle();
    gradientWidget.setStyle(arthurStyle);
    foreach (QWidget *w, qFindChildren<QWidget *>(&gradientWidget))
        w->setStyle(arthurStyle);
    gradientWidget.show();

    return app.exec();
}
