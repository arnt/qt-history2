/****************************************************************
**
** Translation tutorial 2
**
****************************************************************/

#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QString>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QString locale = QLocale::system().name();

    QTranslator translator(0);
    translator.load(QString("tt2_") + locale, ".");
    app.installTranslator(&translator);

    MainWindow *mw = new MainWindow;
    app.setMainWidget(mw);
    mw->show();
    return app.exec();
}
