#include "xform.h"

#include <qapplication.h>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    app.setStyle(new ArthurStyle());

    XFormWidget xformWidget(0);
    xformWidget.show();

    return app.exec();
}
