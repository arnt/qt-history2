/****************************************************************
**
** Qt tutorial 1
**
****************************************************************/

#include <QApplication>
#include <QPushButton>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QPushButton hello("Hello world!");
    hello.resize(100, 30);

    app.setMainWidget(&hello);
    hello.show();
    return app.exec();
}
