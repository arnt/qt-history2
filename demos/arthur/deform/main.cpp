#include "pathdeform.h"

#include <qapplication.h>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    app.setStyle(new ArthurStyle());

    PathDeformWidget deformWidget(0);
    deformWidget.show();

    return app.exec();
}
