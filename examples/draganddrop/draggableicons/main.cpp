#include <QApplication>
#include <QHBoxWidget>
#include "dragwidget.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QHBoxWidget *mainWidget = new QHBoxWidget;
    new DragWidget(mainWidget);
    new DragWidget(mainWidget);
    mainWidget->setWindowTitle("Simple Icons");
    
    mainWidget->show();

    return app.exec();
}
