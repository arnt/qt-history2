#include <QApplication>
#include <QPushButton>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QTranslator translator;
    translator.load("hellotr_la");
    app.installTranslator(&translator);

    QPushButton hello(QPushButton::tr("Hello world!"));
    hello.resize(100, 30);

    hello.show();
    return app.exec();
}
