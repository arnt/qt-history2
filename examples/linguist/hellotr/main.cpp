#include <QtGui>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QTranslator translator;
    translator.load("hellotr_la", ".");
    app.installTranslator(&translator);

    QPushButton hello(QPushButton::tr("Hello world!"));

    hello.show();
    return app.exec();
}
