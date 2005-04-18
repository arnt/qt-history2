#include <QApplication>
#include <QHBoxLayout>
#include "dragwidget.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QWidget *mainWidget = new QWidget;
    QHBoxLayout *horizontalLayout = new QHBoxLayout;
    horizontalLayout->addWidget(new DragWidget);
    horizontalLayout->addWidget(new DragWidget);

    mainWidget->setLayout(horizontalLayout);
    mainWidget->setWindowTitle("Draggable Icons");
    mainWidget->show();

    return app.exec();
}
