#include "pathstroke.h"

#include <qapplication.h>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    app.setStyle(new ArthurStyle());

    PathStrokeWidget pathStrokeWidget;
    pathStrokeWidget.show();

    return app.exec();
}
