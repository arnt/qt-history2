/****************************************************************
**
** Translation tutorial 3
**
****************************************************************/

#include "mainwindow.h"

#include <QApplication>
#include <QString>
#include <QLocale>
#include <QTranslator>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    QString locale = QLocale::system().name();
    locale.chop(3); //remove country

    QTranslator translator(0);
    translator.load(QString("tt3_") + locale, ".");
    app.installTranslator(&translator);

    MainWindow *mw = new MainWindow;
    mw->show();
    return app.exec();
}
