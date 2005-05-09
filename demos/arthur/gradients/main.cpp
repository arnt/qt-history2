#include "gradients.h"

#include <qapplication.h>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    app.setStyle(new ArthurStyle());

    GradientWidget gradientWidget(0);
    gradientWidget.show();

    return app.exec();
}
