#include "pathstroke.h"

#include <qapplication.h>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    PathStrokeWidget pathStrokeWidget;
    QStyle *arthurStyle = new ArthurStyle();
    pathStrokeWidget.setStyle(arthurStyle);
    foreach (QWidget *w, qFindChildren<QWidget *>(&pathStrokeWidget))
        w->setStyle(arthurStyle);
    pathStrokeWidget.show();

    return app.exec();
}
