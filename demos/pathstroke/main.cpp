#include "pathstroke.h"

#include <qapplication.h>

int main(int argc, char **argv)
{
    Q_INIT_RESOURCE(pathstroke);

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
