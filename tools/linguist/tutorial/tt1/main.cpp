/****************************************************************
**
** Translation tutorial 1
**
****************************************************************/

#include <QApplication>
#include <QPushButton>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QTranslator translator;
    translator.load("tt1_la", ".");
    app.installTranslator(&translator);

    QPushButton hello(QPushButton::tr("Hello world!"));

    app.setMainWidget(&hello);
    hello.show();
    return app.exec();
}
