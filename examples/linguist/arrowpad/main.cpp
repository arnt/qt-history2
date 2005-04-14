#include <QtGui>

#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QString locale = QLocale::system().name();

    QTranslator translator;
    translator.load(QString("arrowpad_") + locale, ".");
    app.installTranslator(&translator);

    MainWindow *mainWin = new MainWindow;
    mainWin->show();
    return app.exec();
}
