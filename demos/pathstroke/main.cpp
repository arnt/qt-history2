#include "pathstroke.h"

#include <qapplication.h>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    PathStrokeWidget pathStrokeWidget;
    QStyle *arthurStyle = new ArthurStyle();
    pathStrokeWidget.setStyle(arthurStyle);
    QList<QWidget *> widgets = qFindChildren<QWidget *>(&pathStrokeWidget);
    foreach (QWidget *w, widgets)
        w->setStyle(arthurStyle);
    pathStrokeWidget.show();

    return app.exec();
}
