#include <QApplication>

#include "languagechooser.h"
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    LanguageChooser chooser;
    chooser.show();
    return app.exec();
}
