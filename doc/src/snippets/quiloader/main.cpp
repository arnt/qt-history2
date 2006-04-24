#include <QtGui>
#include <QtUiTools>

#include "mywidget.h"

QWidget *loadCustomWidget(QWidget *parent)
{
    QUiLoader loader;
    QWidget *myWidget;

    QStringList availableWidgets = loader.availableWidgets();

    if (availableWidgets.contains("AnalogClock"))
        myWidget = loader.createWidget("AnalogClock", parent);

    return myWidget;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MyWidget widget;
    widget.show();

    QWidget *customWidget = loadCustomWidget(0);
    customWidget->show();
    return app.exec();
}
