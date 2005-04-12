#include <QApplication>
#include <QVBoxLayout>
#include "dragwidget.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QWidget *mainWidget = new QWidget;
    QVBoxLayout *vbox = new QVBoxLayout(mainWidget);
    vbox->addWidget(new DragWidget);
    vbox->addWidget(new DragWidget);
    mainWidget->setWindowTitle("Simple Icons");

    mainWidget->show();

    return app.exec();
}
